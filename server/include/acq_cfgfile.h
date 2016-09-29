/*  
 *
 */

#ifndef  _CONFIG_FILE_H
#define  _CONFIG_FILE_H

#include <sys/types.h>
#include <unistd.h>

#include "acq_device.h"

struct flaglist {
	char *name;
	int flag;
};

static struct flaglist all_flags[] = {
	{"GENERIC", GENERIC},
	{"SPEC", SPEC},
	{"PICO", PICO},
	{NULL, 0},
};

/*
  config stuff
*/

enum directive { ID_none, ID_acqdev, ID_codes, ID_raw_codes, ID_raw_name };

struct ptr_array {
	void **ptr;
	size_t nr_items;
	size_t chunk_size;
};

struct void_array {
	void *ptr;
	size_t item_size;
	size_t nr_items;
	size_t chunk_size;
};

void **init_void_array(struct void_array *ar, size_t chunk_size,
					   size_t item_size);
int add_void_array(struct void_array *ar, void *data);
INLINEVOID *get_void_array(struct void_array *ar);

/* some safer functions */
void *s_malloc(size_t size);
char *s_strdup(char *string);
unsigned long s_strtoul(char *val);
int s_strtoi(char *val);
unsigned int s_strtoui(char *val);

int checkMode(int is_mode, int c_mode, char *error);
int parseFlags(char *val);
int addSignal(struct void_array *signals, char *val);
void *defineCode(char *key, char *val, void *code);
void *defineNode(void *code, const char *val);
int defineAcqDev(char *key, char *val, char *val2, struct acq_device *rem);
struct acq_device *read_config(FILE * f);
void free_config(struct acq_device *acqdevs);

#endif
