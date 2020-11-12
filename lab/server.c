/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

void str_ser(int sockfd, struct sockaddr *addr, int addrlen);                                                           // transmitting and receiving function

int main(int argc, char *argv[])
{
	int sockfd;
	int ret;
	struct sockaddr_in my_addr;

	if (argc != 2) {
		printf("Parameters error\n");
        exit(0);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0); //create socket
	if (sockfd < 0) {
		printf("error in socket!");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)); //bind socket
	if (ret < 0) {           
		printf("error in binding");
		exit(1);
	}
	printf("waiting for data\n");
	str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
{
	char buffer[BUFSIZE];
	char recv[DATALEN];
	int end = 0;
	int n = 0;
	long lseek = 0;
	float random_num = 0;
	FILE *fp;
	struct ack_so ack;

	while (!end) {
		random_num = (float)rand() / (RAND_MAX);
		printf("random number: %f\n", random_num);
		if (ERROR_P > random_num) { //simulated error(damaged packet)
            ack.len = 0;
            ack.num = -1;
            printf("damaged packet.\n");
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen)) == -1) {
				printf("send error\n");
				exit(1);
			}
			printf("send NACK\n");
		} else { //good packet
			if ((n = recvfrom(sockfd, &recv, DATALEN, 0, addr, (socklen_t *)&addrlen)) == -1) {
				printf("error when receiving\n");
				exit(1);
			}
			printf("receive a packet\n");
			if (recv[n - 1] == '\0') { //end of file
				n--;
				end = 1;
			}
			memcpy((buffer + lseek), recv, n);
            lseek += n;
            ack.len = 0;
            ack.num = 1;
			printf("Received Data(bytes): %d\n", n);
			if ((n = sendto(sockfd, &ack, 2, 0, addr, addrlen)) == -1) {
				printf("send ack error\n");
				exit(1);
			}
			printf("send ACK\n");
		}
        if (end == 1) {printf("end of transmisson.\n");}
	}

	if ((fp = fopen("myReceive.txt", "wt")) == NULL) {
		printf("file does not exist.\n");
		exit(0);
	}
	fwrite(buffer, 1, lseek, fp);
	fclose(fp);
}
