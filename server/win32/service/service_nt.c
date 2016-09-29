/* LIBUSB-WIN32, Generic Windows USB Driver
 * Copyright (C) 2002-2003 Stephan Meyer, <ste_meyer@web.de>
 *
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



#include <windows.h>
#include "service.h"


#define LIBUSB_SERVICE_NAME "libacqdd"

static SERVICE_STATUS service_status;       
static SERVICE_STATUS_HANDLE service_status_handle;

static HANDLE service_stop_event;

static void WINAPI acqd_service_main(int argc, char **argv);
static void WINAPI acqd_service_control(DWORD code);
static void acqd_service_run(int argc, char **argv);
static void acqd_service_set_status(DWORD status, DWORD exit_code);


int main(int argc, char **argv)
{
  SERVICE_TABLE_ENTRY dispatch_table[] =
    {
      { LIBUSB_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)acqd_service_main },
      { NULL, NULL }
    };

  StartServiceCtrlDispatcher(dispatch_table);
  return 0;
}


static void WINAPI acqd_service_main(int argc, char **argv)
{
  
  memset(&service_status, 0, sizeof(service_status));

  service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  service_status.dwCurrentState = SERVICE_START_PENDING; 
  service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
  service_status.dwWaitHint = 5000;

  service_status_handle = RegisterServiceCtrlHandler(LIBUSB_SERVICE_NAME, 
						     acqd_service_control);
  
  if(!service_status_handle)
    {
      return;
    }

  acqd_service_run(argc, argv);

  return;
}


void WINAPI acqd_service_control(DWORD code)
{
  switch(code)
    { 
    case SERVICE_CONTROL_STOP: 
      acqd_service_set_status(SERVICE_STOP_PENDING, NO_ERROR); 
      
      if(service_stop_event)
	{ 
	  SetEvent(service_stop_event);
	}
      return;
    default: 
      ;
    } 
 
  acqd_service_set_status(service_status.dwCurrentState, NO_ERROR);

  return; 
}

static void acqd_service_run(int argc, char **argv)
{
  acqd_service_set_status(SERVICE_START_PENDING, NO_ERROR);
  service_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
  
  if(!service_stop_event)
    {
      acqd_service_set_status(SERVICE_STOPPED, NO_ERROR);
      return;
    }
  
  acqd_service_start_filter();
  acqd_service_set_status(SERVICE_RUNNING, NO_ERROR);

  while(WaitForSingleObject(service_stop_event, 1000) == WAIT_TIMEOUT)
    {
      acqd_service_start_filter();
    } 

  CloseHandle(service_stop_event);
  acqd_service_set_status(SERVICE_STOPPED, NO_ERROR);
}

static void acqd_service_set_status(DWORD status, DWORD exit_code)
{
  service_status.dwCurrentState  = status;
  service_status.dwWin32ExitCode = exit_code;

  SetServiceStatus(service_status_handle, &service_status); 
}
