/* -*- linux-c -*-
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * 
 *
 * ****************************************************************/

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/mtio.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/major.h>
#include <linux/completion.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>
#include <asm/ioctls.h>

#include <asm/vio.h>
#include <asm/iSeries/vio.h>
#include <asm/iSeries/HvLpEvent.h>
#include <asm/iSeries/HvCallEvent.h>
#include <asm/iSeries/HvLpConfig.h>

#define ACQD_VERSION		"1.2"
#define ACQD_MAXREQ		1

#define ACQD_KERN_WARN	KERN_WARNING "acqd: "
#define ACQD_KERN_INFO	KERN_INFO "acqd: "

static int acqd_numdev;

/*
 * The minor number follows the conventions of the SCSI tape drives.  The
 * rewind and mode are encoded in the minor #.  We use this struct to break
 * them out
 */
struct viot_devinfo_struct {
	int devno;
	int mode;
	int rewind;
};

#define VIOTAPOP_RESET          0
#define VIOTAPOP_FSF	        1
#define VIOTAPOP_BSF	        2
#define VIOTAPOP_FSR	        3
#define VIOTAPOP_BSR	        4
#define VIOTAPOP_WEOF	        5
#define VIOTAPOP_REW	        6
#define VIOTAPOP_NOP	        7
#define VIOTAPOP_EOM	        8
#define VIOTAPOP_ERASE          9
#define VIOTAPOP_SETBLK        10
#define VIOTAPOP_SETDENSITY    11
#define VIOTAPOP_SETPOS	       12
#define VIOTAPOP_GETPOS	       13
#define VIOTAPOP_SETPART       14
#define VIOTAPOP_UNLOAD        15

struct acqdlpevent {
	struct HvLpEvent event;
	u32 reserved;
	u16 version;
	u16 sub_type_result;
	u16 tape;
	u16 flags;
	u32 token;
	u64 len;
	union {
		struct {
			u32 tape_op;
			u32 count;
		} op;
		struct {
			u32 type;
			u32 resid;
			u32 dsreg;
			u32 gstat;
			u32 erreg;
			u32 file_no;
			u32 block_no;
		} get_status;
		struct {
			u32 block_no;
		} get_pos;
	} u;
};

enum acqdsubtype {
	acqdopen = 0x0001,
	acqdclose = 0x0002,
	acqdread = 0x0003,
	acqdwrite = 0x0004,
	acqdgetinfo = 0x0005,
	acqdop = 0x0006,
	acqdgetpos = 0x0007,
	acqdsetpos = 0x0008,
	acqdgetstatus = 0x0009
};

enum acqdrc {
	acqd_InvalidRange = 0x0601,
	acqd_InvalidToken = 0x0602,
	acqd_DMAError = 0x0603,
	acqd_UseError = 0x0604,
	acqd_ReleaseError = 0x0605,
	acqd_InvalidTape = 0x0606,
	acqd_InvalidOp = 0x0607,
	acqd_TapeErr = 0x0608,

	acqd_AllocTimedOut = 0x0640,
	acqd_BOTEnc = 0x0641,
	acqd_BlankTape = 0x0642,
	acqd_BufferEmpty = 0x0643,
	acqd_CleanCartFound = 0x0644,
	acqd_CmdNotAllowed = 0x0645,
	acqd_CmdNotSupported = 0x0646,
	acqd_DataCheck = 0x0647,
	acqd_DecompressErr = 0x0648,
	acqd_DeviceTimeout = 0x0649,
	acqd_DeviceUnavail = 0x064a,
	acqd_DeviceBusy = 0x064b,
	acqd_EndOfMedia = 0x064c,
	acqd_EndOfTape = 0x064d,
	acqd_EquipCheck = 0x064e,
	acqd_InsufficientRs = 0x064f,
	acqd_InvalidLogBlk = 0x0650,
	acqd_LengthError = 0x0651,
	acqd_LibDoorOpen = 0x0652,
	acqd_LoadFailure = 0x0653,
	acqd_NotCapable = 0x0654,
	acqd_NotOperational = 0x0655,
	acqd_NotReady = 0x0656,
	acqd_OpCancelled = 0x0657,
	acqd_PhyLinkErr = 0x0658,
	acqd_RdyNotBOT = 0x0659,
	acqd_TapeMark = 0x065a,
	acqd_WriteProt = 0x065b
};

static const struct vio_error_entry acqd_err_table[] = {
	{ acqd_InvalidRange, EIO, "Internal error" },
	{ acqd_InvalidToken, EIO, "Internal error" },
	{ acqd_DMAError, EIO, "DMA error" },
	{ acqd_UseError, EIO, "Internal error" },
	{ acqd_ReleaseError, EIO, "Internal error" },
	{ acqd_InvalidTape, EIO, "Invalid tape device" },
	{ acqd_InvalidOp, EIO, "Invalid operation" },
	{ acqd_TapeErr, EIO, "Tape error" },
	{ acqd_AllocTimedOut, EBUSY, "Allocate timed out" },
	{ acqd_BOTEnc, EIO, "Beginning of tape encountered" },
	{ acqd_BlankTape, EIO, "Blank tape" },
	{ acqd_BufferEmpty, EIO, "Buffer empty" },
	{ acqd_CleanCartFound, ENOMEDIUM, "Cleaning cartridge found" },
	{ acqd_CmdNotAllowed, EIO, "Command not allowed" },
	{ acqd_CmdNotSupported, EIO, "Command not supported" },
	{ acqd_DataCheck, EIO, "Data check" },
	{ acqd_DecompressErr, EIO, "Decompression error" },
	{ acqd_DeviceTimeout, EBUSY, "Device timeout" },
	{ acqd_DeviceUnavail, EIO, "Device unavailable" },
	{ acqd_DeviceBusy, EBUSY, "Device busy" },
	{ acqd_EndOfMedia, ENOSPC, "End of media" },
	{ acqd_EndOfTape, ENOSPC, "End of tape" },
	{ acqd_EquipCheck, EIO, "Equipment check" },
	{ acqd_InsufficientRs, EOVERFLOW, "Insufficient tape resources" },
	{ acqd_InvalidLogBlk, EIO, "Invalid logical block location" },
	{ acqd_LengthError, EOVERFLOW, "Length error" },
	{ acqd_LibDoorOpen, EBUSY, "Door open" },
	{ acqd_LoadFailure, ENOMEDIUM, "Load failure" },
	{ acqd_NotCapable, EIO, "Not capable" },
	{ acqd_NotOperational, EIO, "Not operational" },
	{ acqd_NotReady, EIO, "Not ready" },
	{ acqd_OpCancelled, EIO, "Operation cancelled" },
	{ acqd_PhyLinkErr, EIO, "Physical link error" },
	{ acqd_RdyNotBOT, EIO, "Ready but not beginning of tape" },
	{ acqd_TapeMark, EIO, "Tape mark" },
	{ acqd_WriteProt, EROFS, "Write protection error" },
	{ 0, 0, NULL },
};

/* Maximum number of tapes we support */
#define ACQD_MAX_TAPE	HVMAXARCHITECTEDVIRTUALTAPES
#define MAX_PARTITIONS		4

/* defines for current tape state */
#define VIOT_IDLE		0
#define VIOT_READING		1
#define VIOT_WRITING		2

/* Our info on the tapes */
struct tape_descr {
	char rsrcname[10];
	char type[4];
	char model[3];
};

static struct tape_descr *acqd_unitinfo;
static dma_addr_t acqd_unitinfo_token;

static struct mtget viomtget[ACQD_MAX_TAPE];

static struct class_simple *tape_class;

static struct device *tape_device[ACQD_MAX_TAPE];

/*
 * maintain the current state of each tape (and partition)
 * so that we know when to write EOF marks.
 */
static struct {
	unsigned char	cur_part;
	int		dev_handle;
	unsigned char	part_stat_rwi[MAX_PARTITIONS];
} state[ACQD_MAX_TAPE];

/* We single-thread */
static struct semaphore reqSem;

/*
 * When we send a request, we use this struct to get the response back
 * from the interrupt handler
 */
struct op_struct {
	void			*buffer;
	dma_addr_t		dmaaddr;
	size_t			count;
	int			rc;
	int			non_blocking;
	struct completion	com;
	struct device		*dev;
	struct op_struct	*next;
};

static spinlock_t	op_struct_list_lock;
static struct op_struct	*op_struct_list;

/* forward declaration to resolve interdependence */
static int chg_state(int index, unsigned char new_state, struct file *file);

/* procfs support */
static int proc_acqd_show(struct seq_file *m, void *v)
{
	int i;

	seq_printf(m, "acqd driver version " ACQD_VERSION "\n");
	for (i = 0; i < acqd_numdev; i++) {
		seq_printf(m, "acqd device %d is iSeries resource %10.10s"
				"type %4.4s, model %3.3s\n",
				i, acqd_unitinfo[i].rsrcname,
				acqd_unitinfo[i].type,
				acqd_unitinfo[i].model);
	}
	return 0;
}

static int proc_acqd_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_acqd_show, NULL);
}

static struct file_operations proc_acqd_operations = {
	.open		= proc_acqd_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/* Decode the device minor number into its parts */
void get_dev_info(struct inode *ino, struct viot_devinfo_struct *devi)
{
	devi->devno = iminor(ino) & 0x1F;
	devi->mode = (iminor(ino) & 0x60) >> 5;
	/* if bit is set in the minor, do _not_ rewind automatically */
	devi->rewind = (iminor(ino) & 0x80) == 0;
}

/* This is called only from the exit and init paths, so no need for locking */
static void clear_op_struct_pool(void)
{
	while (op_struct_list) {
		struct op_struct *toFree = op_struct_list;
		op_struct_list = op_struct_list->next;
		kfree(toFree);
	}
}

/* Likewise, this is only called from the init path */
static int add_op_structs(int structs)
{
	int i;

	for (i = 0; i < structs; ++i) {
		struct op_struct *new_struct =
			kmalloc(sizeof(*new_struct), GFP_KERNEL);
		if (!new_struct) {
			clear_op_struct_pool();
			return -ENOMEM;
		}
		new_struct->next = op_struct_list;
		op_struct_list = new_struct;
	}
	return 0;
}

/* Allocate an op structure from our pool */
static struct op_struct *get_op_struct(void)
{
	struct op_struct *retval;
	unsigned long flags;

	spin_lock_irqsave(&op_struct_list_lock, flags);
	retval = op_struct_list;
	if (retval)
		op_struct_list = retval->next;
	spin_unlock_irqrestore(&op_struct_list_lock, flags);
	if (retval) {
		memset(retval, 0, sizeof(*retval));
		init_completion(&retval->com);
	}

	return retval;
}

/* Return an op structure to our pool */
static void free_op_struct(struct op_struct *op_struct)
{
	unsigned long flags;

	spin_lock_irqsave(&op_struct_list_lock, flags);
	op_struct->next = op_struct_list;
	op_struct_list = op_struct;
	spin_unlock_irqrestore(&op_struct_list_lock, flags);
}

/* Map our tape return codes to errno values */
int tape_rc_to_errno(int tape_rc, char *operation, int tapeno)
{
	const struct vio_error_entry *err;

	if (tape_rc == 0)
		return 0;

	err = vio_lookup_rc(acqd_err_table, tape_rc);
	printk(ACQD_KERN_WARN "error(%s) 0x%04x on Device %d (%-10s): %s\n",
			operation, tape_rc, tapeno,
			acqd_unitinfo[tapeno].rsrcname, err->msg);
	return -err->errno;
}

/* Get info on all tapes from OS/400 */
static int get_acqd_info(void)
{
	HvLpEvent_Rc hvrc;
	int i;
	size_t len = sizeof(*acqd_unitinfo) * ACQD_MAX_TAPE;
	struct op_struct *op = get_op_struct();

	if (op == NULL)
		return -ENOMEM;

	acqd_unitinfo = dma_alloc_coherent(iSeries_vio_dev, len,
		&acqd_unitinfo_token, GFP_ATOMIC);
	if (acqd_unitinfo == NULL) {
		free_op_struct(op);
		return -ENOMEM;
	}

	memset(acqd_unitinfo, 0, len);

	hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
			HvLpEvent_Type_VirtualIo,
			viomajorsubtype_tape | acqdgetinfo,
			HvLpEvent_AckInd_DoAck, HvLpEvent_AckType_ImmediateAck,
			viopath_sourceinst(viopath_hostLp),
			viopath_targetinst(viopath_hostLp),
			(u64) (unsigned long) op, VIOVERSION << 16,
			acqd_unitinfo_token, len, 0, 0);
	if (hvrc != HvLpEvent_Rc_Good) {
		printk(ACQD_KERN_WARN "hv error on op %d\n",
				(int)hvrc);
		free_op_struct(op);
		return -EIO;
	}

	wait_for_completion(&op->com);

	free_op_struct(op);

	for (i = 0;
	     ((i < ACQD_MAX_TAPE) && (acqd_unitinfo[i].rsrcname[0]));
	     i++)
		acqd_numdev++;
	return 0;
}


/* Write */
static ssize_t viotap_write(struct file *file, const char *buf,
		size_t count, loff_t * ppos)
{
	HvLpEvent_Rc hvrc;
	unsigned short flags = file->f_flags;
	int noblock = ((flags & O_NONBLOCK) != 0);
	ssize_t ret;
	struct viot_devinfo_struct devi;
	struct op_struct *op = get_op_struct();

	if (op == NULL)
		return -ENOMEM;

	get_dev_info(file->f_dentry->d_inode, &devi);

	/*
	 * We need to make sure we can send a request.  We use
	 * a semaphore to keep track of # requests in use.  If
	 * we are non-blocking, make sure we don't block on the
	 * semaphore
	 */
	if (noblock) {
		if (down_trylock(&reqSem)) {
			ret = -EWOULDBLOCK;
			goto free_op;
		}
	} else
		down(&reqSem);

	/* Allocate a DMA buffer */
	op->dev = tape_device[devi.devno];
	op->buffer = dma_alloc_coherent(op->dev, count, &op->dmaaddr,
			GFP_ATOMIC);

	if (op->buffer == NULL) {
		printk(ACQD_KERN_WARN
				"error allocating dma buffer for len %ld\n",
				count);
		ret = -EFAULT;
		goto up_sem;
	}

	/* Copy the data into the buffer */
	if (copy_from_user(op->buffer, buf, count)) {
		printk(ACQD_KERN_WARN "tape: error on copy from user\n");
		ret = -EFAULT;
		goto free_dma;
	}

	op->non_blocking = noblock;
	init_completion(&op->com);
	op->count = count;

	hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
			HvLpEvent_Type_VirtualIo,
			viomajorsubtype_tape | acqdwrite,
			HvLpEvent_AckInd_DoAck, HvLpEvent_AckType_ImmediateAck,
			viopath_sourceinst(viopath_hostLp),
			viopath_targetinst(viopath_hostLp),
			(u64)(unsigned long)op, VIOVERSION << 16,
			((u64)devi.devno << 48) | op->dmaaddr, count, 0, 0);
	if (hvrc != HvLpEvent_Rc_Good) {
		printk(ACQD_KERN_WARN "hv error on op %d\n",
				(int)hvrc);
		ret = -EIO;
		goto free_dma;
	}

	if (noblock)
		return count;

	wait_for_completion(&op->com);

	if (op->rc)
		ret = tape_rc_to_errno(op->rc, "write", devi.devno);
	else {
		chg_state(devi.devno, VIOT_WRITING, file);
		ret = op->count;
	}

free_dma:
	dma_free_coherent(op->dev, count, op->buffer, op->dmaaddr);
up_sem:
	up(&reqSem);
free_op:
	free_op_struct(op);
	return ret;
}

/* read */
static ssize_t viotap_read(struct file *file, char *buf, size_t count,
		loff_t *ptr)
{
	HvLpEvent_Rc hvrc;
	unsigned short flags = file->f_flags;
	struct op_struct *op = get_op_struct();
	int noblock = ((flags & O_NONBLOCK) != 0);
	ssize_t ret;
	struct viot_devinfo_struct devi;

	if (op == NULL)
		return -ENOMEM;

	get_dev_info(file->f_dentry->d_inode, &devi);

	/*
	 * We need to make sure we can send a request.  We use
	 * a semaphore to keep track of # requests in use.  If
	 * we are non-blocking, make sure we don't block on the
	 * semaphore
	 */
	if (noblock) {
		if (down_trylock(&reqSem)) {
			ret = -EWOULDBLOCK;
			goto free_op;
		}
	} else
		down(&reqSem);

	chg_state(devi.devno, VIOT_READING, file);

	/* Allocate a DMA buffer */
	op->dev = tape_device[devi.devno];
	op->buffer = dma_alloc_coherent(op->dev, count, &op->dmaaddr,
			GFP_ATOMIC);
	if (op->buffer == NULL) {
		ret = -EFAULT;
		goto up_sem;
	}

	op->count = count;
	init_completion(&op->com);

	hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
			HvLpEvent_Type_VirtualIo,
			viomajorsubtype_tape | acqdread,
			HvLpEvent_AckInd_DoAck, HvLpEvent_AckType_ImmediateAck,
			viopath_sourceinst(viopath_hostLp),
			viopath_targetinst(viopath_hostLp),
			(u64)(unsigned long)op, VIOVERSION << 16,
			((u64)devi.devno << 48) | op->dmaaddr, count, 0, 0);
	if (hvrc != HvLpEvent_Rc_Good) {
		printk(ACQD_KERN_WARN "tape hv error on op %d\n",
				(int)hvrc);
		ret = -EIO;
		goto free_dma;
	}

	wait_for_completion(&op->com);

	if (op->rc)
		ret = tape_rc_to_errno(op->rc, "read", devi.devno);
	else {
		ret = op->count;
		if (ret && copy_to_user(buf, op->buffer, ret)) {
			printk(ACQD_KERN_WARN "error on copy_to_user\n");
			ret = -EFAULT;
		}
	}

free_dma:
	dma_free_coherent(op->dev, count, op->buffer, op->dmaaddr);
up_sem:
	up(&reqSem);
free_op:
	free_op_struct(op);
	return ret;
}

/* ioctl */
static int viotap_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	HvLpEvent_Rc hvrc;
	int ret;
	struct viot_devinfo_struct devi;
	struct mtop mtc;
	u32 myOp;
	struct op_struct *op = get_op_struct();

	if (op == NULL)
		return -ENOMEM;

	get_dev_info(file->f_dentry->d_inode, &devi);

	down(&reqSem);

	ret = -EINVAL;

	switch (cmd) {
	case MTIOCTOP:
		ret = -EFAULT;
		/*
		 * inode is null if and only if we (the kernel)
		 * made the request
		 */
		if (inode == NULL)
			memcpy(&mtc, (void *) arg, sizeof(struct mtop));
		else if (copy_from_user((char *)&mtc, (char *)arg,
					sizeof(struct mtop)))
			goto free_op;

		ret = -EIO;
		switch (mtc.mt_op) {
		case MTRESET:
			myOp = VIOTAPOP_RESET;
			break;
		case MTFSF:
			myOp = VIOTAPOP_FSF;
			break;
		case MTBSF:
			myOp = VIOTAPOP_BSF;
			break;
		case MTFSR:
			myOp = VIOTAPOP_FSR;
			break;
		case MTBSR:
			myOp = VIOTAPOP_BSR;
			break;
		case MTWEOF:
			myOp = VIOTAPOP_WEOF;
			break;
		case MTREW:
			myOp = VIOTAPOP_REW;
			break;
		case MTNOP:
			myOp = VIOTAPOP_NOP;
			break;
		case MTEOM:
			myOp = VIOTAPOP_EOM;
			break;
		case MTERASE:
			myOp = VIOTAPOP_ERASE;
			break;
		case MTSETBLK:
			myOp = VIOTAPOP_SETBLK;
			break;
		case MTSETDENSITY:
			myOp = VIOTAPOP_SETDENSITY;
			break;
		case MTTELL:
			myOp = VIOTAPOP_GETPOS;
			break;
		case MTSEEK:
			myOp = VIOTAPOP_SETPOS;
			break;
		case MTSETPART:
			myOp = VIOTAPOP_SETPART;
			break;
		case MTOFFL:
			myOp = VIOTAPOP_UNLOAD;
			break;
		default:
			printk(ACQD_KERN_WARN "MTIOCTOP called "
					"with invalid op 0x%x\n", mtc.mt_op);
			goto free_op;
		}

		/*
		 * if we moved the head, we are no longer
		 * reading or writing
		 */
		switch (mtc.mt_op) {
		case MTFSF:
		case MTBSF:
		case MTFSR:
		case MTBSR:
		case MTTELL:
		case MTSEEK:
		case MTREW:
			chg_state(devi.devno, VIOT_IDLE, file);
		}

		init_completion(&op->com);
		hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
				HvLpEvent_Type_VirtualIo,
				viomajorsubtype_tape | acqdop,
				HvLpEvent_AckInd_DoAck,
				HvLpEvent_AckType_ImmediateAck,
				viopath_sourceinst(viopath_hostLp),
				viopath_targetinst(viopath_hostLp),
				(u64)(unsigned long)op,
				VIOVERSION << 16,
				((u64)devi.devno << 48), 0,
				(((u64)myOp) << 32) | mtc.mt_count, 0);
		if (hvrc != HvLpEvent_Rc_Good) {
			printk(ACQD_KERN_WARN "hv error on op %d\n",
					(int)hvrc);
			goto free_op;
		}
		wait_for_completion(&op->com);
		ret = tape_rc_to_errno(op->rc, "tape operation", devi.devno);
		goto free_op;

	case MTIOCGET:
		ret = -EIO;
		init_completion(&op->com);
		hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
				HvLpEvent_Type_VirtualIo,
				viomajorsubtype_tape | acqdgetstatus,
				HvLpEvent_AckInd_DoAck,
				HvLpEvent_AckType_ImmediateAck,
				viopath_sourceinst(viopath_hostLp),
				viopath_targetinst(viopath_hostLp),
				(u64)(unsigned long)op, VIOVERSION << 16,
				((u64)devi.devno << 48), 0, 0, 0);
		if (hvrc != HvLpEvent_Rc_Good) {
			printk(ACQD_KERN_WARN "hv error on op %d\n",
					(int)hvrc);
			goto free_op;
		}
		wait_for_completion(&op->com);

		/* Operation is complete - grab the error code */
		ret = tape_rc_to_errno(op->rc, "get status", devi.devno);
		free_op_struct(op);
		up(&reqSem);

		if ((ret == 0) && copy_to_user((void *)arg,
					&viomtget[devi.devno],
					sizeof(viomtget[0])))
			ret = -EFAULT;
		return ret;
	case MTIOCPOS:
		printk(ACQD_KERN_WARN "Got an (unsupported) MTIOCPOS\n");
		break;
	default:
		printk(ACQD_KERN_WARN "got an unsupported ioctl 0x%0x\n",
				cmd);
		break;
	}

free_op:
	free_op_struct(op);
	up(&reqSem);
	return ret;
}

static int viotap_open(struct inode *inode, struct file *file)
{
	HvLpEvent_Rc hvrc;
	struct viot_devinfo_struct devi;
	int ret;
	struct op_struct *op = get_op_struct();

	if (op == NULL)
		return -ENOMEM;

	get_dev_info(file->f_dentry->d_inode, &devi);

	/* Note: We currently only support one mode! */
	if ((devi.devno >= acqd_numdev) || (devi.mode)) {
		ret = -ENODEV;
		goto free_op;
	}

	init_completion(&op->com);

	hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
			HvLpEvent_Type_VirtualIo,
			viomajorsubtype_tape | acqdopen,
			HvLpEvent_AckInd_DoAck, HvLpEvent_AckType_ImmediateAck,
			viopath_sourceinst(viopath_hostLp),
			viopath_targetinst(viopath_hostLp),
			(u64)(unsigned long)op, VIOVERSION << 16,
			((u64)devi.devno << 48), 0, 0, 0);
	if (hvrc != 0) {
		printk(ACQD_KERN_WARN "bad rc on signalLpEvent %d\n",
				(int) hvrc);
		ret = -EIO;
		goto free_op;
	}

	wait_for_completion(&op->com);
	ret = tape_rc_to_errno(op->rc, "open", devi.devno);

free_op:
	free_op_struct(op);
	return ret;
}


static int viotap_release(struct inode *inode, struct file *file)
{
	HvLpEvent_Rc hvrc;
	struct viot_devinfo_struct devi;
	int ret = 0;
	struct op_struct *op = get_op_struct();

	if (op == NULL)
		return -ENOMEM;
	init_completion(&op->com);

	get_dev_info(file->f_dentry->d_inode, &devi);

	if (devi.devno >= acqd_numdev) {
		ret = -ENODEV;
		goto free_op;
	}

	chg_state(devi.devno, VIOT_IDLE, file);

	if (devi.rewind) {
		hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
				HvLpEvent_Type_VirtualIo,
				viomajorsubtype_tape | acqdop,
				HvLpEvent_AckInd_DoAck,
				HvLpEvent_AckType_ImmediateAck,
				viopath_sourceinst(viopath_hostLp),
				viopath_targetinst(viopath_hostLp),
				(u64)(unsigned long)op, VIOVERSION << 16,
				((u64)devi.devno << 48), 0,
				((u64)VIOTAPOP_REW) << 32, 0);
		wait_for_completion(&op->com);

		tape_rc_to_errno(op->rc, "rewind", devi.devno);
	}

	hvrc = HvCallEvent_signalLpEventFast(viopath_hostLp,
			HvLpEvent_Type_VirtualIo,
			viomajorsubtype_tape | acqdclose,
			HvLpEvent_AckInd_DoAck, HvLpEvent_AckType_ImmediateAck,
			viopath_sourceinst(viopath_hostLp),
			viopath_targetinst(viopath_hostLp),
			(u64)(unsigned long)op, VIOVERSION << 16,
			((u64)devi.devno << 48), 0, 0, 0);
	if (hvrc != 0) {
		printk(ACQD_KERN_WARN "bad rc on signalLpEvent %d\n",
				(int) hvrc);
		ret = -EIO;
		goto free_op;
	}

	wait_for_completion(&op->com);

	if (op->rc)
		printk(ACQD_KERN_WARN "close failed\n");

free_op:
	free_op_struct(op);
	return ret;
}

struct file_operations viotap_fops = {
	owner: THIS_MODULE,
	read: viotap_read,
	write: viotap_write,
	ioctl: viotap_ioctl,
	open: viotap_open,
	release: viotap_release,
};

/* Handle interrupt events for tape */
static void vioHandleTapeEvent(struct HvLpEvent *event)
{
	int tapeminor;
	struct op_struct *op;
	struct acqdlpevent *tevent = (struct acqdlpevent *)event;

	if (event == NULL) {
		/* Notification that a partition went away! */
		if (!viopath_isactive(viopath_hostLp)) {
			/* TODO! Clean up */
		}
		return;
	}

	tapeminor = event->xSubtype & VIOMINOR_SUBTYPE_MASK;
	op = (struct op_struct *)event->xCorrelationToken;
	switch (tapeminor) {
	case acqdgetinfo:
	case acqdopen:
	case acqdclose:
		op->rc = tevent->sub_type_result;
		complete(&op->com);
		break;
	case acqdread:
		op->rc = tevent->sub_type_result;
		op->count = tevent->len;
		complete(&op->com);
		break;
	case acqdwrite:
		if (op->non_blocking) {
			dma_free_coherent(op->dev, op->count,
					op->buffer, op->dmaaddr);
			free_op_struct(op);
			up(&reqSem);
		} else {
			op->rc = tevent->sub_type_result;
			op->count = tevent->len;
			complete(&op->com);
		}
		break;
	case acqdop:
	case acqdgetpos:
	case acqdsetpos:
	case acqdgetstatus:
		if (op) {
			op->count = tevent->u.op.count;
			op->rc = tevent->sub_type_result;
			if (!op->non_blocking)
				complete(&op->com);
		}
		break;
	default:
		printk(ACQD_KERN_WARN "wierd ack\n");
	}
}

static int acqd_probe(struct vio_dev *vdev, const struct vio_device_id *id)
{
	char tapename[32];
	int i = vdev->unit_address;
	int j;

	if (i >= acqd_numdev)
		return -ENODEV;

	tape_device[i] = &vdev->dev;

	state[i].cur_part = 0;
	for (j = 0; j < MAX_PARTITIONS; ++j)
		state[i].part_stat_rwi[j] = VIOT_IDLE;
	class_simple_device_add(tape_class, MKDEV(ACQD_MAJOR, i), NULL,
			"iseries!vt%d", i);
	class_simple_device_add(tape_class, MKDEV(ACQD_MAJOR, i | 0x80),
			NULL, "iseries!nvt%d", i);
	devfs_mk_cdev(MKDEV(ACQD_MAJOR, i), S_IFCHR | S_IRUSR | S_IWUSR,
			"iseries/vt%d", i);
	devfs_mk_cdev(MKDEV(ACQD_MAJOR, i | 0x80),
			S_IFCHR | S_IRUSR | S_IWUSR, "iseries/nvt%d", i);
	sprintf(tapename, "iseries/vt%d", i);
	state[i].dev_handle = devfs_register_tape(tapename);
	printk(ACQD_KERN_INFO "tape %s is iSeries "
			"resource %10.10s type %4.4s, model %3.3s\n",
			tapename, acqd_unitinfo[i].rsrcname,
			acqd_unitinfo[i].type, acqd_unitinfo[i].model);
	return 0;
}

static int acqd_remove(struct vio_dev *vdev)
{
	int i = vdev->unit_address;

	devfs_remove("iseries/nvt%d", i);
	devfs_remove("iseries/vt%d", i);
	devfs_unregister_tape(state[i].dev_handle);
	class_simple_device_remove(MKDEV(ACQD_MAJOR, i | 0x80));
	class_simple_device_remove(MKDEV(ACQD_MAJOR, i));
	return 0;
}

/**
 * acqd_device_table: Used by vio.c to match devices that we
 * support.
 */
static struct vio_device_id acqd_device_table[] __devinitdata = {
	{ "acqd", "" },
	{ 0, }
};

MODULE_DEVICE_TABLE(vio, acqd_device_table);
static struct vio_driver acqd_driver = {
	.name = "acqd",
	.id_table = acqd_device_table,
	.probe = acqd_probe,
	.remove = acqd_remove
};


int __init viotap_init(void)
{
	int ret;
	struct proc_dir_entry *e;

	op_struct_list = NULL;
	if ((ret = add_op_structs(ACQD_MAXREQ)) < 0) {
		printk(ACQD_KERN_WARN "couldn't allocate op structs\n");
		return ret;
	}
	spin_lock_init(&op_struct_list_lock);

	sema_init(&reqSem, ACQD_MAXREQ);

	if (viopath_hostLp == HvLpIndexInvalid) {
		vio_set_hostlp();
		if (viopath_hostLp == HvLpIndexInvalid) {
			ret = -ENODEV;
			goto clear_op;
		}
	}

	ret = viopath_open(viopath_hostLp, viomajorsubtype_tape,
			ACQD_MAXREQ + 2);
	if (ret) {
		printk(ACQD_KERN_WARN
				"error on viopath_open to hostlp %d\n", ret);
		ret = -EIO;
		goto clear_op;
	}

	printk(ACQD_KERN_INFO "vers " ACQD_VERSION
			", hosting partition %d\n", viopath_hostLp);

	vio_setHandler(viomajorsubtype_tape, vioHandleTapeEvent);

	ret = register_chrdev(ACQD_MAJOR, "acqd", &viotap_fops);
	if (ret < 0) {
		printk(ACQD_KERN_WARN "Error registering acqd device\n");
		goto clear_handler;
	}

	tape_class = class_simple_create(THIS_MODULE, "tape");
	if (IS_ERR(tape_class)) {
		printk(ACQD_KERN_WARN "Unable to allocat class\n");
		ret = PTR_ERR(tape_class);
		goto unreg_chrdev;
	}

	if ((ret = get_acqd_info()) < 0) {
		printk(ACQD_KERN_WARN "Unable to obtain virtual device information");
		goto unreg_class;
	}

	ret = vio_register_driver(&acqd_driver);
	if (ret)
		goto unreg_class;

	e = create_proc_entry("iSeries/acqd", S_IFREG|S_IRUGO, NULL);
	if (e) {
		e->owner = THIS_MODULE;
		e->proc_fops = &proc_acqd_operations;
	}

	return 0;

unreg_class:
	class_simple_destroy(tape_class);
unreg_chrdev:
	unregister_chrdev(ACQD_MAJOR, "acqd");
clear_handler:
	vio_clearHandler(viomajorsubtype_tape);
	viopath_close(viopath_hostLp, viomajorsubtype_tape, ACQD_MAXREQ + 2);
clear_op:
	clear_op_struct_pool();
	return ret;
}

/* Give a new state to the tape object */
static int chg_state(int index, unsigned char new_state, struct file *file)
{
	unsigned char *cur_state =
	    &state[index].part_stat_rwi[state[index].cur_part];
	int rc = 0;

	/* if the same state, don't bother */
	if (*cur_state == new_state)
		return 0;

	/* write an EOF if changing from writing to some other state */
	if (*cur_state == VIOT_WRITING) {
		struct mtop write_eof = { MTWEOF, 1 };

		rc = viotap_ioctl(NULL, file, MTIOCTOP,
				  (unsigned long)&write_eof);
	}
	*cur_state = new_state;
	return rc;
}

/* Cleanup */
static void __exit viotap_exit(void)
{
	int ret;

	remove_proc_entry("iSeries/acqd", NULL);
	vio_unregister_driver(&acqd_driver);
	class_simple_destroy(tape_class);
	ret = unregister_chrdev(ACQD_MAJOR, "acqd");
	if (ret < 0)
		printk(ACQD_KERN_WARN "Error unregistering device: %d\n",
				ret);
	if (acqd_unitinfo)
		dma_free_coherent(iSeries_vio_dev,
				sizeof(acqd_unitinfo[0]) * ACQD_MAX_TAPE,
				acqd_unitinfo, acqd_unitinfo_token);
	viopath_close(viopath_hostLp, viomajorsubtype_tape, ACQD_MAXREQ + 2);
	vio_clearHandler(viomajorsubtype_tape);
	clear_op_struct_pool();
}

MODULE_LICENSE("GPL");
module_init(viotap_init);
module_exit(viotap_exit);
