
/**
 *Taken for TV Time
 *
 *
 * 
 */


#include <stdio.h>
#include "acqd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libxml/tree.h>
#include "acq/acq.h"
#include "acq_config.h"




#ifdef WIN32
#define xml2PtrMalloc malloc
#define xml2PtrRealloc realloc
#define xml2PtrFree free
#else
#define xml2PtrFree xmlFree
#define xml2PtrMalloc xmlMalloc
#define xml2PtrRealloc xmlRealloc
#endif




struct config_info_s {
	int pos;
	int active;
	char name[32];
	commtype_t *commtype;
	char vendorname[100];
	uint64 address;
	uint32 vendorid;
	uint32 deviceid;
	uint32 storagetype;
	uint32 algotype;
	char *acqdata_fileformat;
	char *acqdata_file;
	device_info_t *devices;
	config_info_t *next;
	config_info_t *prev;
};


struct device_info_s {

	char name[32];
	uint32 min_devices;
	uint32 max_devices;
	uint32 digital_devices;
	uint32 analog_devices;
	uint32 logical_devices;
	uint32 time_devices;
	uint32 freq_devices;
	sensordev_t *sensdev;
	device_info_t *next;
	device_info_t *prev;
};


struct sensordev_s {
	uint64 freq;
	uint32 devicenum;
	uint32 min_sample;
	uint32 max_sample;
	uint32 min_caliber;
	uint32 max_caliber;
	char *equation;
	char *time;
	char *units;
	uint32 enablealgo;
	uint8 active;
	sensordev_t *next;
	sensordev_t *prev;
};

struct config_mgr_s {
	config_info_t *first;
	config_info_t *current;
	int num_configs;
	int max_position;
	int verbose;
	int mode;
	int last_device;
	int new_install;
	char configrc[255];
	char *hwname;
	char *devname;
	char *commname;

};



/********************* Communication Related ********************************/

struct serial_s {
	uint32 port;
	uint32 baud;
	uint8 databits;
	uint8 paritybits;
	uint8 stopbits;
	uint8 flowcontrol;
	uint8 pinstate;
};
struct parport_s {
	uint32 port;
};

struct usbdev_s {
	uint32 usbdevid;
	uint32 classid;
	uint32 speed;
};
struct pcidev_s {
	uint32 speed;
	uint32 devid;
	uint32 vendorid;
	uint32 addr;
};
struct pcmcia_s {
	uint32 addr;
	uint32 protocol;

};
struct irda_s {
	uint32 speed;
};

struct bluetooth_s {
	uint32 speed;
};
struct gsmdev_s {
	uint32 speed;
};
struct wifi_s {
	char essid[64];
};
struct wimax_s {
	char id[64];
};

struct zigbee_s {
	char id[64];
};

struct network_s {
	union {
		char ipaddr4[16];
		char ipaddr6[48];
	} ipaddr;
	uint16 port;
	int addrtype;
};



struct commtype_s {
	char name[32];
	char devicefile[200];
	int mode;
	int type1;
	int type2;
	union {
		serial_t serial;
		parport_t pp;
		usbdev_t udev;
		pcidev_t pcidev;
		pcmcia_t pcmcia;
		irda_t irda;
		bluetooth_t blt;
		gsmdev_t gsm;
		wifi_t wifi;
		wimax_t wimax;
		zigbee_t zigbee;
		network_t net;
	} interface;
	commtype_t *next;
	commtype_t *prev;
};


int comparehwinfo(config_info_t * devinfo, config_info_t * newdevinfo)
{
	return 0;
}
int config_set_current_recfileprefix(config_mgr_t * mgr, char *fpref)
{
	return 0;
}
int config_set_current_recformat(config_mgr_t * mgr, char *fmt)
{
	return 0;
}
int config_set_current_rectype(config_mgr_t * mgr, char *rectype)
{
	return 0;
}
int config_set_current_recmode(config_mgr_t * mgr, char *recmode)
{
	return 0;
}
int config_set_current_vendorname(config_mgr_t * mgr, char *vname)
{
	return 0;
}
int config_set_current_deviceid(config_mgr_t * mgr, char *deviceid)
{
	return 0;
}
int config_set_current_vendorid(config_mgr_t * mgr, char *vendorid)
{
	return 0;
}
int config_set_current_hwaddr(config_mgr_t * mgr, char *hwaddr)
{
	return 0;
}

int config_currenthw_add(config_mgr_t * mgr, int pos, char *max_s,
						 char *max_c, char *unit_s)
{
	return 0;
}
int config_set_currenthw_active(config_mgr_t * mgr, int active)
{
	return 0;
}
int config_set_currenthw_minscale(config_mgr_t * mgr, char *min_c)
{
	return 0;
}
int config_set_currenthw_minsample(config_mgr_t * mgr, char *min_s)
{
	return 0;
}
int config_set_currenthw_formula(config_mgr_t * mgr, char *formula)
{
	return 0;
}

config_set_currenthw_time(config_mgr_t * mgr, char *time)
{
	return 0;
}



device_info_t *get_devicelist(config_mgr_t * mgr, const char *devicename)
{
	if (!mgr)
		return NULL;
	if (!mgr->current)
		return NULL;
	if (!mgr->current->devices)
		return NULL;
	if (!strcasecmp(mgr->current->devices->name, devicename))
		return mgr->current->devices;
	else
		return NULL;
}

int file_is_openable_for_read(const char *filename)
{
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		return 0;
	} else {
		close(fd);
		return 1;
	}
}




static int isFreePos(config_mgr_t * mgr, int pos)
{
	config_info_t *rp = mgr->first;

	if (rp) {
		do {
			if (pos == rp->pos)
				return 0;
			rp = rp->next;
		} while (rp != mgr->first);
	}

	return 1;
}

static int getNextPos(config_mgr_t * mgr)
{
	if (!mgr->first)
		return 1;
	return mgr->first->prev->pos + 1;
}

static config_info_t *config_info_new(int pos, const char *name)
{
	config_info_t *infoobj = malloc(sizeof(config_info_t));

	DPRINT("Created new info obj\n");

	if (infoobj) {
		DPRINT("created info object\n");

		infoobj->pos = pos;
		infoobj->active = 1;
		infoobj->devices = NULL;
		infoobj->commtype = NULL;
		memset(infoobj->name, 0, sizeof(infoobj->name));
		memset(infoobj->vendorname, 0, sizeof(infoobj->vendorname));
		infoobj->vendorid = -1;
		infoobj->deviceid = -1;
		infoobj->acqdata_fileformat = NULL;
		infoobj->acqdata_file = NULL;
		infoobj->storagetype = -1;
		infoobj->algotype = 0;
		infoobj->address = -1L;
		if (name) {
			snprintf(infoobj->name, sizeof(infoobj->name), "%s", name);
		} else {
			snprintf(infoobj->name, sizeof(infoobj->name), "%d", pos);
		}

		infoobj->next = 0;
		infoobj->prev = 0;
		return infoobj;
	}
	return 0;
}

static int insert(config_mgr_t * mgr, config_info_t * i, int allowdup)
{
	mgr->num_configs++;

	if (i->pos > mgr->max_position) {
		mgr->max_position = i->pos;
	}

	if (!mgr->first) {
		mgr->first = i;
		mgr->first->next = mgr->first;
		mgr->first->prev = mgr->first;
		mgr->current = i;
	} else {
		config_info_t *rp = mgr->first;

		do {
			if (rp->pos == i->pos) {
				if (mgr->verbose) {
					LOGMSG("config: Position %d already in use.\n", i->pos);
				}
				return 0;
			}

			if (!allowdup) {
				if (!comparehwinfo(rp, i)) {
					if (mgr->verbose) {
						LOGMSG("config: Device list  already in use.\n");
					}
					return 0;
				}
			}

			if (rp->pos > i->pos)
				break;

			rp = rp->next;
		} while (rp != mgr->first);

		i->next = rp;
		i->prev = rp->prev;
		rp->prev->next = i;
		rp->prev = i;

		if (rp == mgr->first && i->pos < mgr->first->pos) {
			mgr->first = i;
		}
		mgr->current = i;
	}

	return 1;
}

static xmlNodePtr find_list(xmlNodePtr node, const char *namename,
							const char *devicesname, const char *intfname)
{
	while (node) {
		if (!xmlStrcasecmp(node->name, XML_HWLIST)) {
			xmlChar *hwname = xmlGetProp(node, BAD_CAST XML_HWNAME);

			if (hwname && !xmlStrcasecmp(hwname, BAD_CAST namename)) {
				xmlChar *devname = xmlGetProp(node, BAD_CAST XML_DEVNAME);

				if (devname && !xmlStrcasecmp(devname, BAD_CAST devicesname)) {

					xmlChar *commname =
						xmlGetProp(node, BAD_CAST XML_INTFNAME);
					if (commname
						&& !xmlStrcasecmp(commname, BAD_CAST intfname)) {

						xml2PtrFree(hwname);
						xml2PtrFree(devname);
						xml2PtrFree(commname);
						return node;
					}
					if (commname)
						xml2PtrFree(commname);
				}
				if (devname)
					xml2PtrFree(devname);
			}
			if (hwname)
				xml2PtrFree(hwname);
		}
		node = node->next;
	}
	return 0;
}


static xmlNodePtr find_namelist(xmlNodePtr node, const char *key,
								const char *name)
{
	while (node) {
		DPRINT("find node %s\n ", node->name);
		if (name && !xmlStrcasecmp(node->name, BAD_CAST name)) {

			DPRINT("Entered for comparison");

			xmlChar *keyname = xmlGetProp(node, BAD_CAST XML_NAME);

			DPRINT("printing the key %s %s\n", keyname, key);
			if (keyname && !xmlStrcasecmp(keyname, BAD_CAST key)) {
				DPRINT("success from us");
				xml2PtrFree(keyname);
				return node;
			}
			if (keyname)
				xml2PtrFree(keyname);
		}
		node = node->next;
	}
	return 0;
}


int config_readconfig(config_mgr_t * mgr)
{
	xmlNodePtr cur;
	xmlDocPtr doc;
	xmlNodePtr config;
	xmlNodePtr devicelist;
	xmlNodePtr list;
	xmlNodePtr commlist;
	xmlChar *list_commtypename;

	if (!file_is_openable_for_read(mgr->configrc)) {
		/* There is no file to try to read, so forget it. */
		return 0;
	}

	if (mgr->verbose) {
		LOGMSG("config: Reading config from %s\n", mgr->configrc);
	}
	doc = xmlParseFile(mgr->configrc);
	if (!doc) {
		return 0;
	}

	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		LOGMSG("config: %s: Empty document.\n", mgr->configrc);
		xmlFreeDoc(doc);
		return 0;
	}

	if (xmlStrcasecmp(cur->name, BAD_CAST XML_CONFIGLIST)) {
		LOGMSG("config: %s: Document not a config file.\n", mgr->configrc);
		xmlFreeDoc(doc);
		return 0;
	}

	list =
		find_list(cur->xmlChildrenNode, mgr->hwname, mgr->devname,
				  mgr->commname);
	if (!list) {
		LOGMSG("%s: No existing %s config list \"%s\".\n",
			   mgr->configrc, mgr->hwname, mgr->devname);
		xmlFreeDoc(doc);
		return 0;
	}
	xmlChar *active_dev = xmlGetProp(list, BAD_CAST XML_ACTIVE);
	xmlChar *dev_address = xmlGetProp(list, BAD_CAST XML_DEVADDR);
	xmlChar *vendor_id = xmlGetProp(list, BAD_CAST XML_VENDORID);
	xmlChar *device_id = xmlGetProp(list, BAD_CAST XML_DEVICEID);
	xmlChar *vendor_name = xmlGetProp(list, BAD_CAST XML_VENDORNAME);
	xmlChar *rec_mode = xmlGetProp(list, BAD_CAST XML_RECORD);
	xmlChar *rec_type = xmlGetProp(list, BAD_CAST XML_RECTYPE);
	xmlChar *rec_filefmt = xmlGetProp(list, BAD_CAST XML_RECFILEEXT);
	xmlChar *rec_filepref = xmlGetProp(list, BAD_CAST XML_RECFILEPREF);

	if (rec_filepref) {
		config_set_current_recfileprefix(mgr, (char *) rec_filepref);
		xml2PtrFree(rec_filepref);
	}
	if (rec_filefmt) {
		config_set_current_recformat(mgr, (char *) rec_filefmt);
		xml2PtrFree(dev_address);
	}
	if (rec_type) {
		config_set_current_rectype(mgr, (char *) rec_type);
		xml2PtrFree(rec_type);
	}
	if (rec_mode) {
		config_set_current_recmode(mgr, (char *) rec_mode);
		xml2PtrFree(rec_mode);
	}
	if (vendor_name) {
		config_set_current_vendorname(mgr, (char *) vendor_name);
		xml2PtrFree(vendor_name);
	}
	if (device_id) {
		config_set_current_deviceid(mgr, (char *) device_id);
		xml2PtrFree(device_id);
	}
	if (vendor_id) {
		config_set_current_vendorid(mgr, (char *) vendor_id);
		xml2PtrFree(vendor_id);
	}
	if (dev_address) {
		config_set_current_hwaddr(mgr, (char *) dev_address);
		xml2PtrFree(dev_address);
	}

	int active = 0;

	if (active_dev) {
		active = atoi((char *) active_dev);
		xml2PtrFree(active_dev);
	} else {
		active = 1;
	}
	config_set_current_active(mgr, active);

	/* Find the Device list associated */
	devicelist =
		find_namelist(cur->xmlChildrenNode, mgr->devname, XML_DEVLIST);
	if (!devicelist) {
		xmlFreeDoc(doc);
		return 0;
	}
	config = devicelist->xmlChildrenNode;
	if (!mgr->current) {
		DPRINT("ganesh gu");
		return 0;
	}
	while (config) {
		if (!xmlStrcasecmp(config->name, BAD_CAST XML_DEVNAME)) {
			xmlChar *active_s = xmlGetProp(config, BAD_CAST XML_ACTIVE);
			xmlChar *pos_s = xmlGetProp(config, BAD_CAST XML_POS);
			xmlChar *min_s = xmlGetProp(config, BAD_CAST XML_MINSAMPLE);
			xmlChar *max_s = xmlGetProp(config, BAD_CAST XML_MAXSAMPLE);
			xmlChar *min_c = xmlGetProp(config, BAD_CAST XML_MINSCALE);
			xmlChar *max_c = xmlGetProp(config, BAD_CAST XML_MAXSCALE);
			xmlChar *formula_s = xmlGetProp(config, BAD_CAST XML_FORMULA);
			xmlChar *unit_s = xmlGetProp(config, BAD_CAST XML_UNITS);
			xmlChar *time_s = xmlGetProp(config, BAD_CAST XML_TIME);

			if (max_s && max_c && unit_s) {
				int pos, active;

				if (!pos_s || (1 != sscanf((char *) pos_s, "%d", &pos))) {
					pos = 0;
				}
				if (active_s) {
					active = atoi((char *) active_s);
				} else {
					active = 1;
				}
				config_currenthw_add(mgr, pos, (char *) max_s, (char *) max_c,
									 (char *) unit_s);
				config_set_currenthw_active(mgr, active);
			}
			if (min_c)
				config_set_currenthw_minscale(mgr, (char *) min_c);
			if (min_s)
				config_set_currenthw_minsample(mgr, (char *) min_s);
			if (formula_s)
				config_set_currenthw_formula(mgr, (char *) formula_s);
			if (time_s)
				config_set_currenthw_time(mgr, (char *) time_s);

			if (time_s)
				xml2PtrFree(time_s);
			if (unit_s)
				xml2PtrFree(unit_s);
			if (formula_s)
				xml2PtrFree(formula_s);
			if (max_c)
				xml2PtrFree(max_c);
			if (min_c)
				xml2PtrFree(min_c);
			if (max_s)
				xml2PtrFree(max_s);
			if (min_s)
				xml2PtrFree(min_s);
			if (active_s)
				xml2PtrFree(active_s);
			if (pos_s)
				xml2PtrFree(pos_s);
		}
		config = config->next;
	}
	commlist =
		find_namelist(cur->xmlChildrenNode, mgr->commname, XML_COMMLIST);
	DPRINT("find name commlist\n");
	if (!commlist) {
		xmlFreeDoc(doc);
		return 0;
	}

	xmlFreeDoc(doc);
	return 1;
}

config_mgr_t *config_new(const char *hwname, const char *devname,
						 const char *commname,char *cfgfile, int verbose)
{
	xmlNodePtr cur = NULL;
	xmlDocPtr doc = NULL;
	xmlNodePtr list = NULL;
	const char *ndevices, *commdev;

	config_mgr_t *mgr = malloc(sizeof(config_mgr_t));

	if (!mgr)
		return 0;
     
     memset(mgr->configrc,0,235);
     
     if( cfgfile != NULL)
	strncpy(mgr->configrc, cfgfile, strlen(cfgfile));
     else
     strncpy(mgr->configrc, ACQCFGFILEPATH, 235);
	mgr->verbose = verbose;
	mgr->first = 0;
	mgr->current = 0;
	mgr->last_device = 0;
	mgr->new_install = 0;
	mgr->num_configs = 0;
	mgr->max_position = 0;
	mgr->hwname = strdup(hwname);
	if (!mgr->hwname) {
		free(mgr);
		return 0;
	}

	mgr->devname = strdup(devname);
	if (!mgr->devname) {
		free(mgr->hwname);
		free(mgr);
		return 0;
	}
	mgr->commname = strdup(commname);
	if (!mgr->commname) {
		free(mgr->devname);
		free(mgr->hwname);
		free(mgr);
		return 0;
	}

	DPRINT("End of The Data");

	if (!mgr->first) {
		mgr->new_install = 1;
		commdev = commname;
		if (verbose) {
			LOGMSG("config: Adding new device list %s\n", devname);
		}

		int fileopen = 1;

		if (!file_is_openable_for_read(mgr->configrc)) {
			fileopen = 0;
		}

		if (mgr->verbose) {
			LOGMSG("config: Reading config from %s\n", mgr->configrc);
		}
		if (fileopen) {
			doc = xmlParseFile(mgr->configrc);
			cur = xmlDocGetRootElement(doc);
		}
		if (!cur) {
			LOGMSG("config: %s: Empty document.\n", mgr->configrc);
			xmlFreeDoc(doc);
		}
		int notconfig = 0;

		if (cur && xmlStrcasecmp(cur->name, BAD_CAST XML_CONFIGLIST)) {
			LOGMSG("config: %s: Document not a config file.\n",
				   mgr->configrc);
			xmlFreeDoc(doc);
			notconfig = 1;
		}


		if (!cur) {
			mgr->first = config_info_new(0, hwname);
			DPRINT("Created new config\n");
			if (!mgr->first) {
				if (doc)
					xmlFreeDoc(doc);
				return 0;
			}
			mgr->first->devices = NULL;
			mgr->current = mgr->first;
			DPRINT("Created new devlist\n");
			if (config_add_devlist(mgr, (char *) devname)) {
				if (doc)
					xmlFreeDoc(doc);
				return 0;
			}
			DPRINT("Created new devlist\n");
			if (config_add_commlist(mgr, (char *) commdev)) {
				if (doc)
					xmlFreeDoc(doc);
				return 0;
			}
			DPRINT("Created new commlist\n");
			config_writeconfig(mgr);
			return mgr;
		} else {
			if (cur && !notconfig)
				list =
					find_list(cur->xmlChildrenNode, mgr->hwname, mgr->devname,
							  mgr->commname);
			else
				list = NULL;
			mgr->first = config_info_new(0, hwname);
			if (!mgr->first) {
				if (doc)
					xmlFreeDoc(doc);
				return mgr;
			}
			mgr->current = mgr->first;
			if (!config_adddev(mgr, cur->xmlChildrenNode, devname)) {
				if (doc)
					xmlFreeDoc(doc);
				return mgr;
			}
			if (!config_addcomm(mgr, cur->xmlChildrenNode, commname)) {
				if (doc)
					xmlFreeDoc(doc);
				return mgr;
			}
		}

	}
	return mgr;
}

void config_delete(config_mgr_t * mgr)
{
	config_info_t *cur = mgr->first;

	while (cur) {
		config_info_t *next = cur->next;

		if (next == mgr->first)
			next = 0;
		free(cur);
		cur = next;
	}
	free(mgr->commname);
	free(mgr->devname);
	free(mgr->hwname);
	free(mgr);
}

int config_is_new_install(config_mgr_t * mgr)
{
	return mgr->new_install;
}

int config_get_num_configs(config_mgr_t * mgr)
{
	return mgr->num_configs;
}

int config_get_max_position(config_mgr_t * mgr)
{
	return mgr->max_position;
}

int config_set(config_mgr_t * mgr, int pos)
{
	config_info_t *rp = mgr->first;

	if (rp) {
		do {
			if (rp->pos == pos) {
				if (mgr->current == rp) {
					return 0;
				}
				mgr->last_device = config_get_current_id(mgr);
				mgr->current = rp;
				return 1;
			}
			rp = rp->next;
		} while (rp != mgr->first);
	}

	return 0;
}

int config_set_by_name(config_mgr_t * mgr, const char *name)
{
	config_info_t *rp = mgr->first;

	if (rp) {
		do {
			if (!strcasecmp(rp->name, name)) {
				if (mgr->current == rp) {
					return 0;
				}
				mgr->last_device = config_get_current_id(mgr);
				mgr->current = rp;
				return 1;
			}
			rp = rp->next;
		} while (rp != mgr->first);
	}

	return 0;
}

void config_inc(config_mgr_t * mgr)
{
	mgr->last_device = config_get_current_id(mgr);

	if (mgr->current) {
		config_info_t *i = mgr->current;

		do {
			mgr->current = mgr->current->next;
		} while (!mgr->current->active && mgr->current != i);
	}
}

void config_dec(config_mgr_t * mgr)
{
	mgr->last_device = config_get_current_id(mgr);

	if (mgr->current) {
		config_info_t *i = mgr->current;

		do {
			mgr->current = mgr->current->prev;
		} while (!mgr->current->active && mgr->current != i);
	}
}

void config_prev(config_mgr_t * mgr)
{
	config_set(mgr, mgr->last_device);
}

int config_get_current_id(config_mgr_t * mgr)
{
	if (!mgr->current) {
		return 0;
	} else {
		return mgr->current->pos;
	}
}

int config_get_prev_id(config_mgr_t * mgr)
{
	if (!mgr->current) {
		return 0;
	} else {
		return mgr->last_device;
	}
}

int config_get_current_pos(config_mgr_t * mgr)
{
	config_info_t *rp = mgr->first;
	int i = 0;

	while (rp && rp != mgr->current) {
		rp = rp->next;
		i++;
	}

	return i;
}

const char *config_get_current_name(config_mgr_t * mgr)
{
	if (!mgr->current) {
		return "nothing";
	} else {
		return mgr->current->name;
	}

}

const char *config_get_current_devicename(config_mgr_t * mgr)
{
	if (!mgr->current) {
		return "Nothing";
	} else {
		return mgr->current->devices->name;
	}

}




int config_add(config_mgr_t * mgr, int pos, const char *devname,
			   const char *commname, const char *hwname)
{
	const device_info_t *dev_list = get_devicelist(mgr, (char *) devname);
	config_info_t *info;
	char tempname[32];

	if (!isFreePos(mgr, pos))
		pos = getNextPos(mgr);

	if (!hwname) {
		snprintf(tempname, sizeof(tempname), "HW-%d", pos);
		hwname = tempname;
	}
	if (!devname) {
		info = config_info_new(pos, (const char *) hwname);
		if (info)
			insert(mgr, info, 1);
		return pos;
	} else {
		info = config_info_new(pos, hwname);
		if (info)
			insert(mgr, info, 1);
		return pos;
	}
	return -1;
}

int config_add_devlist(config_mgr_t * mgr, const char *devname)
{
	int i;
	device_info_t *dev_list = NULL;
	device_info_t *prevdevlist = NULL, *nextdevlist = NULL;
	sensordev_t *sensors = NULL, *newdev = NULL, *olddev = NULL;

	if (!mgr)
		return 1;
	if (!mgr->current)
		return 1;
	if (!mgr->current->devices) {
		mgr->current->devices = dev_list = NULL;
	} else {
		int prevdev = 0;

		prevdevlist = mgr->current->devices;
		while (prevdevlist != NULL) {
			if (!strcasecmp(prevdevlist->name, devname)) {
				DPRINT("prev devices\n");
				prevdev = 1;
				break;
			} else
				prevdevlist = prevdevlist->prev;
		}
		if (prevdev) {
			mgr->current->devices = prevdevlist;
			return 0;
		}
		int nextdev = 0;

		nextdevlist = mgr->current->devices->next;
		while (nextdevlist != NULL) {
			if (!strcasecmp(nextdevlist->name, devname)) {
				nextdev = 1;
				break;
			} else
				nextdevlist = nextdevlist->next;
		}
		DPRINT("Ek check\n");
		if (nextdev) {
			mgr->current->devices = nextdevlist;
			return 0;
		}
		dev_list = NULL;
	}
	if (!dev_list) {

		dev_list = (device_info_t *) malloc(sizeof(device_info_t));
		if (!dev_list)
			return 1;
		dev_list->next = (device_info_t *) NULL;
		dev_list->prev = (device_info_t *) NULL;
		dev_list->max_devices = DEFAULT_TOTAL_DEVICES;
		dev_list->min_devices = DEFAULT_MIN_DEVICES;
		dev_list->analog_devices = DEFAULT_ANALOG_DEVICES;
		dev_list->digital_devices = DEFAULT_DIGITAL_DEVICES;
		dev_list->logical_devices = DEFAULT_LOGICAL_DEVICES;
		dev_list->time_devices = DEFAULT_TIME_DEVICES;
		dev_list->freq_devices = DEFAULT_FREQ_DEVICES;
		memset(dev_list->name, 0, 32);
		strncpy(dev_list->name, devname, 32);
		dev_list->sensdev = (sensordev_t *) malloc(sizeof(sensordev_t));
		if (!dev_list->sensdev) {
			free(dev_list);
			return 1;
		}
		sensors = dev_list->sensdev;
		sensors->prev = NULL;
		sensors->next = NULL;
		i = 1;
		while (i <= dev_list->max_devices) {
			sensors->time = NULL;
			sensors->units = NULL;
			sensors->equation = NULL;
			sensors->next = NULL;
			sensors->devicenum = i;
			sensors->active = DEV_ACTIVE;
			sensors->min_sample = DEFAULT_MIN_SAMPLE;
			sensors->min_caliber = DEFAULT_MIN_SCALE;
			sensors->max_sample = DEFAULT_MAX_SCALE;
			sensors->max_caliber = DEFAULT_MAX_SCALE;
			sensors->enablealgo = DEFAULT_ALGO_STATE;
			DPRINT("CReating devices num = %d %d %x %x\n", sensors->devicenum,
				   sensors->max_caliber, sensors, dev_list->sensdev);
			++i;
			if (i > dev_list->max_devices)
				break;
			sensors->next = (sensordev_t *) malloc(sizeof(sensordev_t));
			if (!sensors->next)
				break;
			olddev = sensors;
			sensors = sensors->next;
			sensors->prev = olddev;
		}

		device_info_t *present = mgr->current->devices;

		if (present == NULL) {
			mgr->current->devices = dev_list;
			return 0;
		}
		while (present != NULL && present->next != NULL)
			present = present->next;
		dev_list->prev = present;
		dev_list->next = NULL;
		mgr->current->devices = dev_list;
		return 0;
	}
	return 0;
}

int config_add_commlist(config_mgr_t * mgr, const char *commdev)
{
	int i;
	commtype_t *intflist;
	commtype_t *prevdevlist, *nextdevlist;

	if (!mgr)
		return 1;
	if (!mgr->current)
		return 1;
	if (!mgr->current->commtype) {
		mgr->current->commtype = intflist = NULL;
	} else {
		prevdevlist = mgr->current->commtype;
		while (prevdevlist != NULL) {
			if (!strcasecmp(prevdevlist->name, commdev))
				break;
			else
				prevdevlist = prevdevlist->prev;
		}
		if (prevdevlist) {
			mgr->current->commtype = prevdevlist;
			return 0;
		}
		nextdevlist = mgr->current->commtype;
		while (nextdevlist != NULL) {
			if (!strcasecmp(nextdevlist->name, commdev))
				break;
			else
				nextdevlist = nextdevlist->next;
		}
		if (nextdevlist) {
			mgr->current->commtype = nextdevlist;
			return 0;
		}
		intflist = NULL;
	}
	if (!intflist) {
		intflist = (commtype_t *) malloc(sizeof(commtype_t));
		if (!intflist)
			return 1;
		intflist->next = (commtype_t *) NULL;
		memset(intflist->name, 0, 32);
		strncpy(intflist->name, commdev, 32);
		intflist->mode = MODE_CAPTURE_AND_DISPLAY;
		intflist->type1 = INTF_SERIAL;
		intflist->type2 = INTFDEV_SERIAL_NORMAL;
		memset(intflist->devicefile, 0, 200);
		strncpy(intflist->devicefile, DEFAULT_SERIAL_DEVICE, 200);
		intflist->interface.serial.port = 0;
		intflist->interface.serial.baud = DEFAULT_SERIAL_BAUD;
		intflist->interface.serial.databits = DEFAULT_SERIAL_DATABITS;
		intflist->interface.serial.paritybits = DEFAULT_SERIAL_PARITYBITS;
		intflist->interface.serial.stopbits = DEFAULT_SERIAL_STOPBITS;
		intflist->interface.serial.flowcontrol = DEFAULT_SERIAL_FLWCTL;
		intflist->interface.serial.pinstate = DEFAULT_SERIAL_ENABLE;
		commtype_t *present = mgr->current->commtype;

		if (present == NULL) {
			mgr->current->commtype = intflist;
			return 0;
		}
		while (present != NULL && present->next != NULL)
			present = present->next;
		intflist->prev = present;
		present->next = intflist;
		mgr->current->commtype = intflist;
		return 0;
	}
	return 0;
}


int config_set_current_active(config_mgr_t * mgr, int active)
{
	if (mgr->current) {
		mgr->current->active = active;
		return 1;
	}
	return 0;
}


const char *config_get_current_hwname(config_mgr_t * mgr)
{
	if (mgr->current) {
		return mgr->current->name;
	} else {
		return "";
	}
}

void config_set_current_hwname(config_mgr_t * mgr, const char *hwname)
{
	if (mgr->current) {
		snprintf(mgr->current->name, sizeof(mgr->current->name), "%s",
				 hwname);
	}
}



void config_activate_all_channels(config_mgr_t * mgr)
{
	config_info_t *rp = mgr->first;

	while (rp) {
		rp->active = 1;
		rp = rp->next;
		if (rp == mgr->first)
			break;
	}
}

int config_get_current_active(config_mgr_t * mgr)
{
	if (mgr->current) {
		return mgr->current->active;
	}
	return 0;
}


int config_remove(config_mgr_t * mgr)
{						/* untested? */
	config_info_t *i = mgr->current;

	if (!mgr->current)
		return 0;
	i->next->prev = i->prev;
	i->prev->next = i->next;

	if (i == mgr->first) {
		mgr->first = i->next;
	}

	if (i == mgr->current) {
		mgr->current = i->next;
	}

	if (i == i->next) {
		mgr->current = NULL;
		mgr->first = NULL;
	}
	free(i);
	return 1;
}

config_info_t *ripout(config_mgr_t * mgr, int pos)
{
	config_info_t *rp = mgr->first;

	do {
		if (pos == rp->pos)
			break;
		rp = rp->next;
	} while (rp != mgr->first);

	rp->next->prev = rp->prev;
	rp->prev->next = rp->next;

	if (rp == mgr->first) {
		mgr->first = rp->next;
	}

	if (rp == mgr->current) {
		mgr->current = rp->next;
	}

	if (rp == rp->next) {
		mgr->current = NULL;
		mgr->first = NULL;
	}

	return rp;
}

int config_remap(config_mgr_t * mgr, int pos)
{
	int res;

	if (!mgr->current)
		return 0;
	if (pos == mgr->current->pos)
		return 1;

	if (isFreePos(mgr, pos)) {
		config_info_t *i = ripout(mgr, mgr->current->pos);

		i->pos = pos;
		res = insert(mgr, i, 1);

	} else {
		config_info_t *old = ripout(mgr, pos);
		config_info_t *i = ripout(mgr, mgr->current->pos);

		old->pos = i->pos;
		i->pos = pos;
		res = insert(mgr, old, 1) && insert(mgr, i, 1);
	}
	return res;
}

static xmlNodePtr find_config(xmlNodePtr node, const char *configname)
{
	while (node) {
		if (!xmlStrcasecmp(node->name, BAD_CAST "config")) {
			xmlChar *name = xmlGetProp(node, BAD_CAST "name");

			if (name && !xmlStrcasecmp(name, BAD_CAST configname)) {
				xml2PtrFree(name);
				return node;
			}
			if (name)
				xml2PtrFree(name);
		}

		node = node->next;
	}

	return 0;
}

int config_writeconfig(config_mgr_t * mgr)
{
	char values[32];
	device_info_t *prevdevlist, *nextdevlist;
	xmlDocPtr doc;
	xmlNodePtr top;
	xmlNodePtr list;
	xmlNodePtr configl, sensl;
	char filename[255];
	config_info_t *rp = mgr->first;

	if (!rp)
		return 0;

	strncpy(filename, ACQCFGFILEPATH, 235);

	doc = xmlParseFile(filename);
	if (!doc) {
		if (file_is_openable_for_read(filename)) {
			LOGMSG("config: Config file exists, but cannot be parsed.\n");
			LOGMSG("config: Stations will NOT be saved.\n");
			return 0;
		} else {
			/* Config file doesn't exist, create a new one. */
			LOGMSG("config: No config file found, creating a new one.\n");
			doc = xmlNewDoc(BAD_CAST "1.0");
			if (!doc) {
				LOGMSG("config: Could not create new config file.\n");
				return 0;
			}
		}
	}
	DPRINT("Creating Root Element\n");
	top = xmlDocGetRootElement(doc);
	if (!top) {
		DPRINT("Entered here something wrong\n");
		/* Set the DTD */
		xmlDtdPtr dtd;

		dtd = xmlNewDtd(doc,
						BAD_CAST "acqd",
						BAD_CAST "-//acqd//DTD acqd 1.0//EN",
						BAD_CAST
						"http://acqd.sourceforge.net/DTD/configlist1.dtd");
		doc->intSubset = dtd;
		if (!doc->children) {
			xmlAddChild((xmlNodePtr) doc, (xmlNodePtr) dtd);
		} else {
			DPRINT("prev sibling\n");
			xmlAddPrevSibling(doc->children, (xmlNodePtr) dtd);
		}

		/* Create the root node */
		top = xmlNewDocNode(doc, 0, BAD_CAST "configlist", 0);
		if (!top) {
			LOGMSG
				("config: Could not create toplevel element 'configlist'.\n");
			xmlFreeDoc(doc);
			return 0;
		} else {
			xmlDocSetRootElement(doc, top);
			xmlNewProp(top, BAD_CAST "xmlns",
					   BAD_CAST "http://acqd.sourceforge.net/DTD/");
		}
		DPRINT("newlist \n");
	}
	DPRINT("Checking for everything\n");
	rp = mgr->first;
	while (rp != NULL) {
		list =
			find_list(top->xmlChildrenNode, rp->name, rp->devices->name,
					  rp->commtype->name);
		if (!list) {
			list = xmlNewTextChild(top, 0, BAD_CAST "hwlist", 0);
			xmlNewProp(list, BAD_CAST XML_HWNAME, BAD_CAST rp->name);
			xmlNewProp(list, BAD_CAST XML_DEVNAME,
					   BAD_CAST rp->devices->name);
			xmlNewProp(list, BAD_CAST XML_INTFNAME,
					   BAD_CAST rp->commtype->name);
		}
		rp = rp->next;
	}

	prevdevlist = mgr->current->devices;
	while (prevdevlist != NULL) {
		configl =
			find_namelist(top->xmlChildrenNode, prevdevlist->name,
						  XML_DEVLIST);
		DPRINT("previous Devlopment \n");
		if (!configl) {
			configl = xmlNewTextChild(top, 0, BAD_CAST XML_DEVLIST, 0);
			xmlNewProp(configl, BAD_CAST XML_NAME,
					   BAD_CAST prevdevlist->name);
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->min_devices);
			xmlNewProp(configl, BAD_CAST XML_MINDEVICES, BAD_CAST values);
			DPRINT("min sample\n");
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->min_devices);
			xmlNewProp(configl, BAD_CAST XML_MAXDEVICES,
					   BAD_CAST & values[0]);
			DPRINT("min sample\n");
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->analog_devices);
			xmlNewProp(configl, BAD_CAST XML_ANADEVICES,
					   BAD_CAST & values[0]);
			DPRINT("min sample\n");
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->digital_devices);
			xmlNewProp(configl, BAD_CAST XML_DIGDEVICES,
					   BAD_CAST & values[0]);
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->logical_devices);
			xmlNewProp(configl, BAD_CAST XML_LOGDEVICES,
					   BAD_CAST & values[0]);
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->freq_devices);
			xmlNewProp(configl, BAD_CAST XML_FREQDEVICES,
					   BAD_CAST & values[0]);
			memset(values, 0, 32);
			snprintf(values, 32, "%d", prevdevlist->time_devices);
			xmlNewProp(configl, BAD_CAST XML_TIMEDEVICES,
					   BAD_CAST & values[0]);
			DPRINT("sensors start\n");
			sensordev_t *fwdsens = prevdevlist->sensdev;

			while (fwdsens != NULL) {
				DPRINT("Recreate %d\n", fwdsens->devicenum);
				sensl = xmlNewTextChild(configl, 0, BAD_CAST XML_DEVNAME, 0);
				memset(values, 0, 32);
				snprintf(values, 32, "%d", fwdsens->devicenum);
				xmlNewProp(sensl, BAD_CAST XML_POS, BAD_CAST values);
				xmlNewProp(sensl, BAD_CAST XML_ACTIVE,
						   ((fwdsens->active ==
							 1) ? (BAD_CAST "1") : (BAD_CAST "0")));
				DPRINT("After active check\n");
				memset(values, 0, 32);
				snprintf(values, 32, "%d", fwdsens->min_sample);
				xmlNewProp(sensl, BAD_CAST XML_MINSAMPLE, BAD_CAST values);
				DPRINT("after min sample\n");
				memset(values, 0, 32);
				snprintf(values, 32, "%d", fwdsens->max_sample);
				xmlNewProp(sensl, BAD_CAST XML_MAXSAMPLE, BAD_CAST values);
				DPRINT("after max sample\n");
				memset(values, 0, 32);
				snprintf(values, 32, "%d", fwdsens->min_caliber);
				xmlNewProp(sensl, BAD_CAST XML_MINSCALE, BAD_CAST values);
				DPRINT("caliber min \n");
				memset(values, 0, 32);
				snprintf(values, 32, "%d", fwdsens->max_caliber);
				xmlNewProp(sensl, BAD_CAST XML_MAXSCALE, BAD_CAST values);
				DPRINT("%d %s\n", fwdsens->max_caliber, values);
				if (fwdsens->units != NULL)
					xmlNewProp(sensl, BAD_CAST XML_UNITS,
							   BAD_CAST fwdsens->units);
				DPRINT("after units\n");
				if (fwdsens->equation)
					xmlNewProp(sensl, BAD_CAST XML_FORMULA,
							   BAD_CAST fwdsens->equation);
				if (fwdsens->time)
					xmlNewProp(sensl, BAD_CAST XML_TIME,
							   BAD_CAST fwdsens->time);
				DPRINT("After time values\n");
				fwdsens = fwdsens->next;
				if (fwdsens == NULL)
					DPRINT("What is this\n");
			}
			DPRINT("Checking again\n");
		}
		prevdevlist = prevdevlist->prev;
	}
	xmlKeepBlanksDefault(0);
	xmlSaveFormatFile(filename, doc, 1);
	xmlFreeDoc(doc);
	return 1;
}

int  config(char *configfile,config_mgr_t *mgr)
{
	mgr = config_new("PWRX-1130", "DEV-1", "COMM-1",configfile,1);
return 0;
}

int config_adddev(config_mgr_t * mgr, xmlNodePtr node, char *name)
{
	sensordev_t *sensors, *olddev, *newdev;
	device_info_t *dev_list;
	xmlNodePtr list, configs;

	list = find_namelist(node, name, XML_DEVLIST);
	if (!list) {
		config_add_devlist(mgr, name);
		return 0;
	} else {

		dev_list = (device_info_t *) malloc(sizeof(device_info_t));
		if (!dev_list)
			return 1;
		dev_list->next = (device_info_t *) NULL;
		dev_list->prev = (device_info_t *) NULL;

		xmlChar *maxdevs = xmlGetProp(list, BAD_CAST XML_MAXDEVICES);

		DPRINT("MAX DEVICES xml\n");
		if (maxdevs) {
			dev_list->max_devices = atoi(maxdevs);
			free(maxdevs);
		}
		xmlChar *mindevs = xmlGetProp(list, BAD_CAST XML_MINDEVICES);

		if (mindevs) {
			dev_list->min_devices = atoi(mindevs);
			free(mindevs);
		}
		xmlChar *anadevs = xmlGetProp(list, BAD_CAST XML_ANADEVICES);

		if (anadevs) {
			dev_list->analog_devices = atoi(anadevs);
			free(anadevs);
		}
		xmlChar *digdevs = xmlGetProp(list, BAD_CAST XML_DIGDEVICES);

		if (digdevs) {
			dev_list->digital_devices = atoi(digdevs);
			free(digdevs);
		}
		xmlChar *logdevs = xmlGetProp(list, BAD_CAST XML_LOGDEVICES);

		if (logdevs) {
			dev_list->digital_devices = atoi(logdevs);
			free(logdevs);
		}
		xmlChar *freqdevs = xmlGetProp(list, BAD_CAST XML_FREQDEVICES);

		if (freqdevs) {
			dev_list->freq_devices = atoi(freqdevs);
			free(freqdevs);
		}
		xmlChar *timedevs = xmlGetProp(list, BAD_CAST XML_TIMEDEVICES);

		if (timedevs) {
			dev_list->time_devices = atoi(timedevs);
			free(timedevs);
		}
		memset(dev_list->name, 0, 32);
		strncpy(dev_list->name, name, 32);
		dev_list->sensdev = (sensordev_t *) malloc(sizeof(sensordev_t));
		if (!dev_list->sensdev) {
			free(dev_list);
			return 1;
		}
		sensors = dev_list->sensdev;
		sensors->prev = NULL;
		sensors->next = NULL;
		configs = list->xmlChildrenNode;
		while (configs) {
			sensors->time = (char *) xmlGetProp(configs, BAD_CAST XML_TIME);
			sensors->units = (char *) xmlGetProp(configs, BAD_CAST XML_UNITS);
			sensors->equation =
				(char *) xmlGetProp(configs, BAD_CAST XML_FORMULA);
			sensors->next = NULL;
			xmlChar *ipos = xmlGetProp(configs, BAD_CAST XML_POS);

			if (ipos) {
				sensors->devicenum = atoi(ipos);
				free(ipos);
			}
			xmlChar *actives = xmlGetProp(configs, BAD_CAST XML_ACTIVE);

			if (actives) {
				sensors->active = atoi(actives);
				free(actives);
			}
			xmlChar *mins = xmlGetProp(configs, BAD_CAST XML_MINSAMPLE);

			if (mins) {
				sensors->min_sample = atoi(mins);
				free(mins);
			}
			xmlChar *maxs = xmlGetProp(configs, BAD_CAST XML_MAXSAMPLE);

			if (maxs) {
				sensors->max_sample = atoi(maxs);
				free(maxs);
			}
			xmlChar *minc = xmlGetProp(configs, BAD_CAST XML_MINSCALE);

			if (minc) {
				sensors->min_caliber = atoi(minc);
				free(minc);
			}
			xmlChar *maxc = xmlGetProp(configs, BAD_CAST XML_MAXSCALE);

			if (maxc) {
				sensors->max_caliber = atoi(maxc);
				free(maxc);
			}
			xmlChar *algo = xmlGetProp(configs, BAD_CAST XML_ALGO);

			if (algo) {
				sensors->enablealgo = atoi(algo);
				free(algo);
			}
			DPRINT("Again devices num = %d %d %x %x\n", sensors->devicenum,
				   sensors->max_caliber, sensors, dev_list->sensdev);
			configs = configs->next;
			if (!configs)
				break;
			sensors->next = (sensordev_t *) malloc(sizeof(sensordev_t));
			if (!sensors->next)
				break;
			olddev = sensors;
			sensors = sensors->next;
			sensors->prev = olddev;
		}
		if (!mgr->current->devices)
			dev_list->prev = mgr->current->devices;
		else
			dev_list->prev = NULL;
		mgr->current->devices = dev_list;
	}

}
int config_addcomm(config_mgr_t * mgr, xmlNodePtr node, char *name)
{



}
void free_config(struct acq_device *acqdevs)
{

}
struct acq_device *read_config(FILE * f)
{
	struct acq_device *acqdev;

	return acqdev;
}
