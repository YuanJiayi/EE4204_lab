/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

void tv_sub(struct timeval *out, struct timeval *in); //calculate transfer time
float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);          

int main(int argc, char *argv[])
{
	int sockfd;
	long len;
	float ti, rt, th;
	struct sockaddr_in ser_addr_in;
	char **pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

	sh=gethostbyname(argv[1]); //get host's information
	if (sh==NULL) {             
		printf("error when gethostbyname");
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr = sh->h_aliases; *pptr != NULL; pptr++) {
		printf("the aliases name: %s\n", *pptr);
	}

	switch (sh->h_addrtype) {
		case AF_INET:
			printf("AF_INET\n");
			break;
		default:
			printf("unknown addrtype\n");
			break;
	}

	ser_addr_in.sin_family = AF_INET;
	ser_addr_in.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr_in.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr_in.sin_zero), 8);

	if ((fp = fopen("myfile.txt", "r+t")) == NULL) {
		printf("file does not exist\n");
		exit(0);
	}

	ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr_in, sizeof(struct sockaddr_in), &len);
	rt = (len / (float)ti); //average transmission rate
	th = 8.0 * rt / 1000.0; //throughput
	printf("Time(ms): %.3f\nData Sent(bytes): %d\nData Rate(Kbytes/s): %f\nThroughput(Mbps): %f\n", ti, (int)len, rt, th);

	close(sockfd);
	fclose(fp);
	exit(0);
}

void tv_sub(struct timeval *out, struct timeval *in) {
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;	
	}
	out->tv_sec -= in->tv_sec;
}

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len) {
	char *buffer;
	char send[DATALEN];
	int n, send_len;
	long file_size;
	long buf_index = 0;
	float time_inv = 0.0;
	struct ack_so ack;
	struct timeval send_t, recv_t;

	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);
	printf("File Length(bytes): %d\n", (int)file_size);
	printf("Packet Length(bytes): %d\n", DATALEN);

	buffer = (char *)malloc(file_size+1);
	if(buffer == NULL) exit(1);
	fread(buffer, 1, file_size, fp);
	buffer[file_size] = '\0';

	gettimeofday(&send_t, NULL);
	while (buf_index <= file_size) {
		printf("#%ld\n", buf_index);
		if ((file_size+1-buf_index) <= DATALEN) {
			send_len = file_size + 1 - buf_index;
		} else {
			send_len = DATALEN;
		}
		memcpy(send, (buffer + buf_index), send_len);
		n = sendto(sockfd, &send, send_len, 0, addr, addrlen);
		if (n == -1) {
			printf("send error\n");
			exit(1);
		}
		printf("send a packet\n");

		//ack
		if ((n = recvfrom(sockfd, &ack, 2, 0, addr, (socklen_t*)&addrlen)) == -1) {
			printf("receive ack error");
			exit(1);
		}
		if (ack.num == 1 && ack.len == 0) {
			buf_index += send_len;
			printf("receive ack\n");
		} else if (ack.len == 0 && ack.num == -1) { //NACK
			printf("receive nack");
		} else {
			buf_index += send_len;
		}
	}

	gettimeofday(&recv_t, NULL);
	*len = buf_index;
	tv_sub(&recv_t, &send_t);
	time_inv += (recv_t.tv_sec) * 1000.0 + (recv_t.tv_usec) / 1000.0;

	return time_inv;
}
