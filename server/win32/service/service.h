/* Some part of libusb
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#ifndef __ACQD_SERVICE_H__
#define __ACQD_SERVICE_H__

#include "registry.h"


#define LIBACQD_SERVICE_NAME   "libacqd"
#define LIBACQD_SERVICE_PATH   "system32\\libacqd-nt.exe"


bool_t acqd_service_load_dll();
bool_t acqd_service_free_dll();


void acqd_service_start_filter(void);
void acqd_service_stop_filter(void);


bool_t acqd_create_service(const char *name, const char *display_name,
			  const char *binary_path, unsigned long type,
			  unsigned long start_type);
bool_t acqd_delete_service(const char *name);
bool_t acqd_start_service(const char *name);
bool_t acqd_stop_service(const char *name);



#endif
