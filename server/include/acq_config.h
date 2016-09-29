#ifndef ACQ_CONFIG_H
#define ACQ_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif
#define ACQCFGFILE       "acqd.xml"
//#define ACQCFGFILEPATH   ".acqd/"ACQCFGFILE
#define ACQCFGFILEPATH   ACQCFGFILE
#define PIDFILE    "/var/run/acqd"
#define ACQD       "/dev/acqd"
#define LOGFILE    "/var/log/acqd"
#define ACQ_INET_PORT   9888
#define VERSION  "0.0.1"
#define PROGNAME "acqd "VERSION
/* Names of XML  config string */
#define DEFAULT_MAX_SCALE       4095
#define DEFAULT_MAX_SAMPLE       2047
#define DEFAULT_MIN_SAMPLE       1
#define DEFAULT_MIN_SCALE        1
#define DEV_ACTIVE               1
#define DEFAULT_ALGO_STATE       0


#ifndef DEFAULT_TOTAL_DEVICES
#define DEFAULT_TOTAL_DEVICES 100
#endif
#ifndef DEFAULT_TOTAL_DEVICES
#define DEFAULT_TOTAL_DEVICES 100
#endif
#ifndef DEFAULT_MIN_DEVICES
#define DEFAULT_MIN_DEVICES 1
#endif
#ifndef DEFAULT_ANALOG_DEVICES
#define DEFAULT_ANALOG_DEVICES 90
#endif
#ifndef DEFAULT_DIGITAL_DEVICES
#define DEFAULT_DIGITAL_DEVICES 1
#endif
#ifndef DEFAULT_LOGICAL_DEVICES
#define DEFAULT_LOGICAL_DEVICES 1
#endif
#ifndef DEFAULT_TIME_DEVICES
#define DEFAULT_TIME_DEVICES 1
#endif
#ifndef DEFAULT_FREQ_DEVICES
#define DEFAULT_FREQ_DEVICES 1
#endif
#ifndef WIN32
#define DEFAULT_SERIAL_DEVICE  "/dev/ttyS0"
#else
#define DEFAULT_SERIAL_DEVICE  "COM1"
#endif
#define MODE_CAPTURE_AND_DISPLAY   0x01



#define RTSDTR                 0x03
#define BAUD_19200             19200
#define DEFAULT_SERIAL_ENABLE  RTSDTR
#define DEFAULT_SERIAL_BAUD    BAUD_19200
#define DATA_8                 8
#define PARITY_NONE            0
#define STOP_1                 1
#define FLWCTL_NONE            0
#define DEFAULT_SERIAL_DATABITS     DATA_8
#define DEFAULT_SERIAL_PARITYBITS   PARITY_NONE
#define DEFAULT_SERIAL_STOPBITS    STOP_1
#define DEFAULT_SERIAL_FLWCTL      FLWCTL_NONE
#define INTF_SERIAL                0
#define INTFDEV_SERIAL_NORMAL      0




/* All Xml definitions */
#define XML_CONFIGLIST     "configlist"
#define XML_HWLIST         "hwlist"
#define XML_DEVLIST       "devicelist"
#define XML_COMMLIST      "interfacelist"
#define XML_NAME           "name"
#define XML_ACTIVE         "active"
#define XML_POS            "position"
#define XML_HWNAME         "hardware"
#define XML_DEVNAME        "device"
#define XML_INTFNAME       "interface"
#define XML_VENDORNAME     "vendorname"
#define XML_VENDORID       "vendorid"
#define XML_DEVICEID      "deviceid"
#define XML_DEVADDR        "addr"
#define XML_RECORD        "record"
#define XML_RECFILEEXT    "recfileext"
#define XML_RECFILEPREF   "recfilepref"
#define XML_RECTYPE       "rectype"
#define XML_MINSAMPLE     "minsample"
#define XML_MAXSAMPLE     "maxsample"
#define XML_MINSCALE     "minscale"
#define XML_MAXSCALE     "maxscale"
#define XML_FORMULA      "formula"
#define XML_UNITS        "units"
#define XML_TIME         "time"
#define XML_MINDEVICES   "mindevices"
#define XML_MAXDEVICES   "mxxdevices"
#define XML_ANADEVICES   "analogdevices"
#define XML_DIGDEVICES   "digitaldevices"
#define XML_LOGDEVICES   "logicaldevices"
#define XML_FREQDEVICES   "frequencydevices"
#define XML_TIMEDEVICES   "timedevices"
#define XML_ALGO          "algo"

/**
 * Taken from TV Time
 *
 *
 * 
 */



	typedef struct config_mgr_s config_mgr_t;
	typedef struct config_info_s config_info_t;
	typedef struct device_info_s device_info_t;
	typedef struct sensordev_s sensordev_t;
	typedef struct commtype_s commtype_t;
	typedef struct serial_s serial_t;
	typedef struct parport_s parport_t;
	typedef struct usbdev_s usbdev_t;
	typedef struct pcidev_s pcidev_t;
	typedef struct pcmcia_s pcmcia_t;
	typedef struct irda_s irda_t;
	typedef struct bluetooth_s bluetooth_t;
	typedef struct gsmdev_s gsmdev_t;
	typedef struct wifi_s wifi_t;
	typedef struct wimax_s wimax_t;
	typedef struct zigbee_s zigbee_t;
	typedef struct network_s network_t;

/**
 * Creates a new config manager object. 
 */
	config_mgr_t *config_new(const char *hwname, const char *devname,
							 const char *commname,char *cfgfile,int verbose);

/**
 * This will free the config manager object, but will not write any
 * unsaved changes to the config file.
 */
	void config_delete(config_mgr_t * mgr);

/**
 * Is this a clean startup, that is, was there no entry in the
 * config file.
 */
	int config_is_new_install(config_mgr_t * mgr);

/**
 * Set the current device directly to the given position.
 */
	int config_set(config_mgr_t * mgr, int pos);

/**
 * Set the current device to the device with the given name.
 */
	int config_set_by_name(config_mgr_t * mgr, const char *name);

/**
 * Change up one device in the list.
 */
	void config_inc(config_mgr_t * mgr);

/**
 * Change down one device in the list.
 */
	void config_dec(config_mgr_t * mgr);

/**
 * Change to the last device we were at before the current one.
 */
	void config_prev(config_mgr_t * mgr);


/**
 * Returns the id of this device.
 */
	int config_get_current_id(config_mgr_t * mgr);

/**
 * Returns the position of the current device in the list.  This is
 * only used to know when we have wrapped around during device
 * scanning.
 */
	int config_get_current_pos(config_mgr_t * mgr);

/**
 * Returns the name of this device.
 */
	const char *config_get_current_device_name(config_mgr_t * mgr);

/**
 * Returns the frequency band for this device.
 */
	const char *config_get_current_band(config_mgr_t * mgr);


/**
 * Returns true if the current device is active in the browse list.
 */
	int config_get_current_active(config_mgr_t * mgr);


/**
 * The last device we were at, before the current one.
 */
	int config_get_prev_id(config_mgr_t * mgr);

/**
 * Returns how many configs there are in the list.
 */
	int config_get_num_configs(config_mgr_t * mgr);

/**
 * Returns the maximum position in the config list.
 */
	int config_get_max_position(config_mgr_t * mgr);

/**
 * Add or update the device list.
 */
	int config_add(config_mgr_t * mgr, int pos, const char *band,
				   const char *device, const char *name);

/**
 * Add a whole band to the device list.
 */
	int config_add_band(config_mgr_t * mgr, const char *band);

/**
 * Activates or deactivates the current config from the list.
 */
	int config_set_current_active(config_mgr_t * mgr, int active);

/**
 * Re-activates all devices.
 */
	void config_activate_all_devices(config_mgr_t * mgr);

/**
 * Remap the current device to the new position.
 */
	int config_remap(config_mgr_t * mgr, int pos);


/**
 * Writes out the current config file.
 */
	int config_writeconfig(config_mgr_t * mgr);

#ifdef __cplusplus
};
#endif
#endif
