/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"
#include <arpa/inet.h>

void tv_sub(struct timeval *out, struct timeval *in); //calculate transfer time
void wait_ack(int sockfd, struct sockaddr *addr, socklen_t addrlen); //block until receive ack 
void send_to_sock(int sockfd, struct sockaddr *addr, socklen_t addrlen);            

int main(int argc, char *argv[])
{
	int len, sockfd;
	struct sockaddr_in ser_addr_in;
	char **pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	char *IPaddr = argv[1];

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             //get host's information
		printf("error when gethostbyname");
		exit(0);
	}


	ser_addr_in.sin_family = AF_INET;
	ser_addr_in.sin_port = htons(MYUDP_PORT);
	ser_addr_in.sin.addr.s_addr = inet_addr(IPaddr);
	memset(&(ser_addr.sin_zero), 0, sizeof(ser_addr_in.sin_zero));
	struct sockaddr *ser_addr = (struct sockaddr *)&ser_addr_in;
	socklen_t ser_addrlen = sizeof(struct sockaddr);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	send_to_sock(sockfd, ser_addr, ser_addrlen);
	close(sockfd);
}

void tv_sub(struct timeval *out, struct timeval *in) {
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;	
	}
	out->tv_sec -= in->tv_sec;
}

void wait_ack(int sockfd, struct sockaddr *addr, socklen_t addrlen) {
	int ACK = 0;
	int ACK_received = 0;
	while (!ACK_received) {
		if (recvfrom(sockfd, &ACK, sizeof(ACK), 0, addr, &addrlen) >= 0) {
			if (ACK == 1) {
				ACK_received = 1;
				printf("ACK Received\n");
			} else {
				printf("ACK received but value was not 1\n");
				exit(1);
			}
		} else {
			printf("error when waiting for ACK\n");
			exit(1);
		}
	}
}

void send_to_sock(int sockfd, struct sockaddr *ser_addr, socklen_t ser_addrlen) {
	char packet[DATAUNIT};
	char filename[] = "myfile.txt";
	FILE* fp = fopen(filename, "rt");
	if (fp == NULL) {
		printf("File %s does not exist\n", filename);
		exit(1);
	}

	fseek(fp, 0, SEEK_END);
	long filesize = ftell(fp);
	rewind(fp);
	printf("The file length is %d bytes\n", (int)filesize);

	char filebuffer[filesize];

	fread(filebuffer, 1, filesize, fp);
	filebuffer[filesize] = 0x4;
	filesize += 1;
	fclose(fp);

	sendto(sockfd, &filesize, sizeof(filesize), 0, ser_addr, ser_addrlen);
	wait_ack(sockfd, ser_addr, ser_addrlen);

	struct timeval timesend;
	gettimeofday(&timesend, NULL);

	long fileoffset = 0;
	int dum = 1;

	while (fileoffset < filesize) {
		for (int i = 0; i < dum; i++) {
			int packetsize = (DATAUNIT < filesize - fileoffset) ? DATAUNIT ; filesize - fileoffset;
			memcpy(packet, (filebuffer + fileoffset), packetsize);
			fileoffset += packetsize;
			int n = sendto(sockfd, &packet, packetsize, 0, ser_addr, ser_addrlen);
			if (n < 0) {
				printf("error in sending packet\n");
				exit(1);
			}
			printf("packet of size %d sent\n", n);
		}
		wait_ack(sockfd, ser_addr, ser_addrlen);
		dum = (++dum % 5 == 0) ? 1 : dum % 5;
	}
	struct timeval rcv_time;
	gettimeofday(&rcv_time, NULL);

	tv_sub(&rcv_time, &timesend);
	float time = (rcv_time.tv_sec) * 1000.0 + (rcv_time.tv_usec) / 1000.0;
	printf("DATAUNIT %d bytes | %ld bytes sent over %.3fms | %.3f Mbytes/s\n", DATAUNIT, fileoffset, time, fileoffset/time/1000);
}
