#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <acq_proto.h>

int create_basecmd(unsigned char cmdtype, char *cmdbuff, int cmdlen)
{
	if (cmdtype == CMD_START || cmdtype == CMD_CFGBLKSTART
		|| cmdtype == CMD_CFGCMD || cmdbuff == NULL || cmdlen < 2)
		return -1;
	memset(cmdbuff, 0, cmdlen);
	cmdbuff[0] = CMD_START;
	cmdbuff[1] = cmdtype;
	switch (cmdtype) {
	case CMD_DISPLAYMODE:
	case CMD_DISPLAYTYPE:
	case CMD_NOTSUPP:
	case CMD_SUPP:
		break;
	}
	return 0;
}
int create_cfgcmd(char *buff, int cmdlen)
{
	return 0;
}
int create_blockcfgcmd(char *buff, int cmdlen)
{

	return 0;
}

int sendbuffer(int fd, char *buffer, int len, int enablessl)
{
	int dlen = len;
	int left = 0;
	int bytes = 0;

	if (!enablessl) {
		while (dlen > 0) {
			bytes = write(fd, &buffer[left], dlen);
			left += bytes;
			dlen = dlen - bytes;
		}
		return 0;
	}
	return 0;
}
int recvbuffer(int fd, char *buffer, int len, int enablessl)
{
	int dlen = len;
	int left = 0;
	int bytes = 0;

	if (!enablessl) {
		while (dlen > 0) {
			bytes = read(fd, &buffer[left], dlen);
			left += bytes;
			dlen = dlen - bytes;
		}
		return 0;
	}
	return 0;
}

