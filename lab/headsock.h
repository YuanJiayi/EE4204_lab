// headfile for UDP program
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define DATALEN 500
#define BUFSIZE 60000
#define PACKLEN 508
#define HEADLEN 8
#define ERROR_P 0.5

struct ack_so {
	uint8_t num;
	uint8_t len;
};

struct packet_so {
	uint32_t num;
	uint32_t len;
	char data[DATALEN];
};
