#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <error.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>

#define BLUE_FONT "\033[40;34m%02x\033[0m "
#define RED_FONT "\033[40;31m%02x\033[0m "
#define GREEN_FONT "\033[40;32m%02x\033[0m "
#define YELLOW_FONT "\033[40;33m%02x\033[0m "
#define PURPLE_FONT "\033[40;35m%02x\033[0m "
#define DGREEN_FONT "\033[40;36m%02x\033[0m "
#define WHITE_FONT "\033[40;37m%02x\033[0m "

#pragma pack (1)
typedef struct {
	char user_name[30];
	uint32_t client_ip_bin;
	char client_mac_ascll[17];
	uint8_t no_1[3];
	uint8_t client_isp_bin;
	uint8_t n0_2;
	uint8_t sum_key[4];
} MAC_PACK;
#pragma pack ()


uint8_t *mac_pack_create(char *mac, char *ip, uint8_t isp)
{
	static uint8_t macinfo[60];
	MAC_PACK *mp = (MAC_PACK *)macinfo;
	int32_t i, ecx=0x4e67c6a7, esi, ebx;

	memset(macinfo, 0, sizeof(MAC_PACK));
	memcpy(mp->client_mac_ascll, mac, sizeof(mp->client_mac_ascll));
	mp->client_ip_bin = inet_addr(ip);
	mp->client_isp_bin = isp;
	for(i = 0; i < sizeof(MAC_PACK)-4; i++){
		esi = ecx;
		esi = esi << 5;
		if(ecx > 0){
			ebx = ecx;
			ebx = ebx >> 2;
		}else{
			ebx = ecx;
			ebx = ebx >> 2;
			ebx = ebx | 0xc0000000;
		}
		esi = esi + macinfo[i];
		ebx = ebx + esi;
		ecx = ecx ^ ebx;
	}
	ecx = ecx & 0x7FFFFFFF;
	for(i = 0; i < 4; i++){
	    mp->sum_key[i] = (ecx >> (i*8)) & 0xff;
	}
	return macinfo;
}

int main(int argc, char **argv)
{
	struct sockaddr_in address;
	int sockfd, error, i = 0, opt;
	uint8_t *pbuf, isp;
	char mac[20], ip[20];

	//no error
	opterr = 0;

	while( (opt = getopt(argc, argv, "m:i:s:")) != -1) {
		switch(opt) {
			case 'm':
				strcpy(mac, optarg);
				i++;
				break;
			case 'i':
				strcpy(ip, optarg);
				i++;
				break;
			case 's':
				isp = atoi(optarg);
				i++;
				break;
			default:
				printf("Usage: macopen -i ip -m mac -s isp(1-3)\n\n");
				return 0;
				break;
		}
	}

	if(i != 3){
		printf("Usage: macopen -i IP -m MAC -s ISP(1-3)\n\n");
		return 0;
	}else{
		if(isp > 3 || isp == 0 || strlen(mac) != 17 || strlen(ip) > 15){
			printf("Usage: macopen -i IP -m MAC -s ISP(1-3)\n\n");
			return 0;
		}
		printf("Usage: macopen -i %s -m %s -s %d\n\n", ip, mac, isp);
	}
	pbuf = mac_pack_create(mac, ip, isp);
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("202.193.160.123");
    address.sin_port = htons(20015);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0){
		perror("socket error!");
		return 0;
	}

	while(1){
		error = sendto(sockfd, pbuf, sizeof(MAC_PACK), 0, (struct sockaddr *)&address, sizeof(address));
		if(error < 0){
			perror("sendto error");
			continue;
		}
		for(i = 0; i < sizeof(MAC_PACK); i++){
			if(i%10 == 0) printf("\n");
			if(i < 30) printf(WHITE_FONT, pbuf[i]);
			else if(i < 34) printf(GREEN_FONT, pbuf[i]);
			else if(i < 51) printf(YELLOW_FONT, pbuf[i]);
			else if(i < 54) printf(WHITE_FONT, pbuf[i]);
			else if(i < 55) printf(RED_FONT, pbuf[i]);
			else if(i < 56) printf(WHITE_FONT, pbuf[i]);
			else printf(DGREEN_FONT, pbuf[i]);
		}
		printf(" SEND PACK %dBytes.\n", sizeof(MAC_PACK));
macopen

		sleep(60);
	}

	close(sockfd);
	return 0;
}
