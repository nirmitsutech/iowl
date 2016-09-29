#include "acq_hw.h"
#include "acqd.h"

int init_hwlist(char *device,char *cfgf,acqmanager_t *acqmg)
{
        if(acqmg == NULL ) return -1; 
     if( cfgf != NULL )
         config(cfgf,acqmg->mgr);
     else
         config("acqd.xml",acqmg->mgr);
     
 return 0;
}

void simulation_send()
{


}

int check_hw(commtype_t *comm_dev)
{



        return 0;
}

int open_device()
{
    return 0;
}

int write_device(commtype_t *commhw)
{
return 0;
}
commtype_t *next_commdevice()
{
        commtype_t *commdevl;
return commdevl;
}

int check_hwlist(commtype_t *commhw)
{
if(open_device()) return 1;
if(commhw == NULL ) return 1;
while(commhw != NULL)
{
write_device(commhw);
commhw=next_commdevice();
}

        return 0;
}
