/*



*/




#ifndef ACQ_PROTO_H
#define ACQ_PROTO_H
#ifdef __cplusplus
extern "C" {
#endif
/* START OF THE DATA TRANSMISSION INTIATED BY THIS PROTOCOL */
#define CMD_START    0xCC
/*SUPPORTED  */
#define CMD_SUPP       0xEE
/*NOT supported           */
#define CMD_NOTSUPP   0xEF
/* END OF TRANSMISION */
#define CMD_END      0xFF
#define CMD_SUCCESS  0xFE
#define CMD_FAILURE  0xFD

/* Display modes supported by remote clients */
#define CMD_DISPLAYMODE  0xF0
/* Display types supported by remote clients */
#define CMD_DISPLAYTYPE  0xF1
/* send a configuration block */
#define CMD_CFGBLKSTART  0xF2
#define CMD_CFGCMD       0xF3
/* send a re initialization event */
#define CMD_REINITIALIZE 0xF4
/* reconnect/authentication mobile clients or sensors */
#define CMD_RECONNECTDEVICES 0xF5
#define CMD_ADMINAUTH        0xF6
#define CMD_ADMINNAME        0xF7
#define CMD_ADMINPASSWD      0xF8
#define CMD_GETDEVICE        0xF9
#define CMD_USEPROTOANDALGO  0xFA
#define CMD_STOREDATAANDPKT  0xFB
#define CMD_TIMESYNC         0xFC



#define PROTO       0x01
#define ALGO        0x00
#define ALGO_FFT     0x01
#define ALGO_DFT     0x02
#define ALGOO_CTAVE  0x03




#define DISPMODE_CONS  0x01
#define DISPMODE_SUI    0x02
#define DISPMODE_MUI    0x04

#define DISPTYPE_WITHNORMAL     0x01
#define DISPTYPE_WITHGRAPH  0x02
#define DISPTYPE_WITHCHART  0x04
#define DISPTYPE_WITHTREE       0x08
#define DISPTYPE_WITHHEXDUMP    0x16


#define    CFGCMD_EOFNODEPARENT 0xEC
#define    CFGCMD_EOFNODECMD    0xEB
#define    CFGCMD_EOFNODESENDINDEX    0x01 /* node[index] =value  */ 
#define    CFGCMD_EOFNODESENDVAL      0x02 /* node = value */
#define     CFGCMD_EOFNODEVAL  0xEA

typedef struct cfgcmd_s
       {
               /* 2 byte of one byte index nodes */
       unsigned char  nodelen;
              /*nodes */
       unsigned char *nodenums;
       /*data */
       unsigned char nodevalues[1];
       }cfgcmd_t;






int create_basecmd(unsigned char cmdtype,char *cmdbuff,int cmdlen);
int sendbuffer(int fd,char *buffer,int len,int enablessl);
int recvbuffer(int fd,char *buffer,int len,int enablessl);
int create_client(char *addr,unsigned short int port);
#ifdef __cplusplus
}
#endif
#endif
