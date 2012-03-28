/*
 * Copyright (C) 2008, Lorenzo Pallara l.pallara@avalpa.com
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#define MULTICAST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "vns_config.h"
#include "vns_list.h"

#define TS_PACKET_SIZE 188

#define LEN sizeof(struct student)


long long int usecDiff(struct timeval* time_stop, struct timeval* time_start)
{
	long long int temp = 0;
	long long int utemp = 0;

	if (time_stop && time_start) {
		if (time_stop->tv_usec >= time_start->tv_usec) {
			utemp = time_stop->tv_usec - time_start->tv_usec;
			temp = time_stop->tv_sec - time_start->tv_sec;
		} else {
			utemp = time_stop->tv_usec + 1000000 - time_start->tv_usec;
			temp = time_stop->tv_sec - 1 - time_start->tv_sec;
		}
		if (temp >= 0 && utemp >= 0) {
			temp = (temp * 1000000) + utemp;
        	} else {
			fprintf(stderr, "start time %ld.%ld is after stop time %ld.%ld\n", time_start->tv_sec, time_start->tv_usec, time_stop->tv_sec, time_stop->tv_usec);
			temp = -1;
		}
	} else {
		fprintf(stderr, "memory is garbaged?\n");
		temp = -1;
	}
        return temp;
}


#define MAX_PORT_NUM 1		//(24*32)
int main (int argc, char *argv[]) {

	int sockfd[MAX_PORT_NUM];
	unsigned int data_rate[MAX_PORT_NUM];
	int repeat[MAX_PORT_NUM];
	char* ipaddr[MAX_PORT_NUM];
	int ports[MAX_PORT_NUM];

	//unsigned int bitrate;
	//char tsfile[80];
	char ts_file[MAX_PORT_NUM][80];

	int len;
	int sent;
	//int transport_fd;
	int trans_fd[MAX_PORT_NUM];
	struct sockaddr_in addr;
	unsigned long int packet_size;
	unsigned char* send_buf;

	//unsigned long long int packet_time;
	unsigned long long int real_time;
	struct timeval time_start;
	struct timeval time_stop;
	struct timespec nano_sleep_packet;

	unsigned int port_num;
	int i;
	int k;
	unsigned long long total_udp = 0;
	unsigned long long total_send = 0;

	int dict_qos;
	char qos_conf[100];
	char key[80];
	int rv;

	unsigned char test_buff[188*7];
	unsigned char * p = test_buff;
	
	char* config_file; 

	for(k=0;k<7;++k){
		p[k*188+0] = 0x47;
		p[k*188+1] = 0x5f;
		p[k*188+2] = 0xff;
		p[k*188+3] = 0x10+(k%16);
		memset(p+k*188+4,0+1,32);
		memset(p+k*188+4+32,0+1,32);
		memset(p+k*188+4+32*2,k+1,32);
		memset(p+k*188+4+32*3,'h',188-4-32*3);
		p[k*188+187] = 0x11;
		p[k*188+186] = 0x22;
		p[k*188+185] = 0x33;
		p[k*188+184] = 0x44;
	}



	memset(&addr, 0, sizeof(addr));
	memset(&time_start, 0, sizeof(time_start));
	memset(&time_stop, 0, sizeof(time_stop));
	memset(&nano_sleep_packet, 0, sizeof(nano_sleep_packet));

	if(argc < 2 ) {
		fprintf(stderr, "Usage: %s config_file [ip]\n", argv[0]);
		fprintf(stderr, "[ip] will replace all ip in config file\n");
		fprintf(stderr, "bit rate refers to transport stream bit rate\n");
		fprintf(stderr, "zero bitrate is 100.000.000 bps\n");
		return 0;
    }
	
	
	addr.sin_family = AF_INET;

	config_file = argv[1];
	//strcpy(qos_conf,"./tsudpsend.conf");
	strcpy(qos_conf,config_file);
	rv = mcfg_dict_new(&dict_qos);
	rv = mcfg_load_file(dict_qos,qos_conf);

	for(i = 0; i < MAX_PORT_NUM; i++){          //read all ports
		sprintf(key,"stream_%d.udp_port",i);
		rv=mcfg_getint(dict_qos,key,&ports[i]);
		if(rv<0)
			break;
	}
	port_num = i;

	printf("port_num %d\n", port_num);

	if(port_num > MAX_PORT_NUM){
		fprintf(stderr, "MAX_PORT_NUM = %d\n",MAX_PORT_NUM);
		return 0;
	}
	
	//ts_file_tmp[0] = 0;
	//file_num = 1;
	for(i = 0; i < port_num; i++){                      //read all ip address

		sprintf(key,"stream_%d.data_rate",i);
		rv = mcfg_getint(dict_qos, key, &data_rate[i]);
		if(rv<0)
			break;
/*
		for(j = 0; j < i + 1; j++){
			if(rate[j] == data_rate[i]){
				break;
			}else if(j == i){
				rate[i] = data_rate[i];
				rate_num++;
			}
		}
*/
		sprintf(key,"stream_%d.repeat",i);
		rv = mcfg_getint(dict_qos, key, &repeat[i]);
		if(rv<0)
			break;

		sprintf(key,"stream_%d.ts_file",i);
		rv = mcfg_getstring(dict_qos, key, ts_file[i],sizeof(ts_file[i]));

		if(rv<0)
			break;
/*
		for(j = 0; j < (i + 1); j++){
			if(*ts_file[ts_file_tmp[j]] == *ts_file[i]){
				break;
			}else if(j == i){
				ts_file_tmp[file_num] = i;
				file_num++;
			}
		}
*/
		sprintf(key,"stream_%d.dst_ip",i);
		rv=mcfg_getstring(dict_qos,key,key,sizeof(key));
		if(rv<0)
			break;
		else
			ipaddr[i] = key;

		if (data_rate[i] <= 0) {
			data_rate[i] = 100000000;
		}
	}

	if(argc >= 3){
		for(i = 0; i < port_num; i++){
			ipaddr[i] = argv[2];
		}
	}
	
	for(i = 0; i < port_num; i++){
		//printf("ipaddr  %d %s\n", i, ipaddr[i]);
		addr.sin_addr.s_addr = inet_addr(ipaddr[i]);
		addr.sin_port = htons(ports[i]);
		sockfd[i] = socket(AF_INET, SOCK_DGRAM, 0);

		if(sockfd[i] < 0) {
			printf("socket(): error ");
			return 0;
		}

		int buffsize = 16*1024*1024;
		if(setsockopt(sockfd[i],SOL_SOCKET,SO_SNDBUF,&buffsize,sizeof(buffsize)) < 0){
			printf("setsockopt error\n");
		}
	}

/*
	for(i = 0; i < file_num; i++){
		trans_fd[i] = open(ts_file[ts_file_tmp[i]], O_RDONLY);
		if(trans_fd[i] < 0)
		{
			fprintf(stderr, "can't open file %s\n", ts_file[ts_file_tmp[i]]);
			for(i = 0; i < port_num; ++i){
				close(sockfd[i]);
			}
			return 0;
		}
		else
			printf("file opend %s\n", ts_file[ts_file_tmp[i]]);
	}
*/
	for(i = 0; i < port_num; i++){
		trans_fd[i] = open(ts_file[i], O_RDONLY);
		if(trans_fd[i] < 0)
		{
			fprintf(stderr, "can't open file %s\n", ts_file[i]);
			for(i = 0; i < port_num; ++i){
				close(sockfd[i]);
			}
			return 0;
		}
		//else
			//printf("file opend %s\n", ts_file[i]);
	}
	

	int completed = 0;
	packet_size = 7 * TS_PACKET_SIZE;
	send_buf = malloc(packet_size);

//restart:
	//packet_time = 0;
	real_time = 0;

	unsigned long long int p_time[MAX_PORT_NUM];	

	nano_sleep_packet.tv_nsec = 665778/port_num;

	gettimeofday(&time_start, 0);
	while (!completed)
	{
	
		gettimeofday(&time_stop, 0);
		real_time = usecDiff(&time_stop, &time_start);
		
		for(i = 0; i < port_num; i++)
		{
			if(repeat[i] >= 0)
			{
				while(real_time * data_rate[i] > p_time[i] * 1000000)	//theorical bits against sent bits 
				{
resend:		
					len = read(trans_fd[i], send_buf, packet_size);
					
					if(len < 0)
					{
						printf("file %s read error\n",ts_file[i]);
						fprintf(stderr, "ts file read error \n");
						goto exit;
					}
					else if(len == 0)
					{
						if(repeat[i] == 1)
						{
							repeat[i] = -1;
							close(trans_fd[i]);
							close(sockfd[i]);
							printf("udp port %d closed\n", ports[i]);
							break;
						}
						else 
						{
							if(repeat[i] > 1)
							{
								repeat[i] -= 1;
							}
							lseek(trans_fd[i], 0, SEEK_SET);
							printf("port %d file %s sended\n", i, ts_file[i]);
							goto resend;
						}

					}
					else 
					{
						if(len % 188 != 0){
							printf("%s %d\n",__FILE__,__LINE__);
							printf("len = %d\n",len);
							printf("total send %llu\n",total_send);
							printf("total udp %llu\n",total_udp);
							goto exit;
						}
						addr.sin_addr.s_addr = inet_addr(ipaddr[i]);
						addr.sin_port = htons(ports[i]);
						sent = sendto(sockfd[i], send_buf, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
						
						if(sent == len)
						{
							total_send += len;
							++total_udp;
						}
						else
						{
							printf("%s %d\n",__FILE__,__LINE__);
							printf("sent = %d, len = %d\n",sent,len);
						}
						
						if(sent <= 0) {
							perror("send(): error ");
							completed = 1;
							goto exit;
						} 
						else 
						{
							p_time[i] += packet_size * 8;
						}
					}
				}
			}
		}
		nanosleep(&nano_sleep_packet, 0);
	}

exit:

	for(i = 0; i < port_num; ++i){
		close(sockfd[i]);
		close(trans_fd[i]);
	}
	free(send_buf);
	rv = mcfg_dict_free(dict_qos);
	return 0;
}

