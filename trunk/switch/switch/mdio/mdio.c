


#include <stdint.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mdio8306.h"
#include "int6k/int6k.h"
#include "int6k/homeplug.h"
#include "tools/error.h"
#include "tools/flags.h"
#include "tools/memory.h"
#include "tools/format.h"
#include "ether/channel.h"

extern int ctl8306_debug_mode(void);
extern int ctl8306_loop_num(void);


#define ttrace(...) \
	if(ctl8306_debug_mode()){\
		printf(__VA_ARGS__);\
	}else{\
	}


#define VS_MDIO_MEM  0xA09C

//#define SIOCGMIIPHY	(1000UL)
//#define SIOCGMIIREG	(1001UL)
//#define SIOCSMIIREG	(1002UL)

//#define SIOCGMIIPHY	0x8947		/* Get address of MII PHY in use. */
#define SIOCGMIIREG	0x8948		/* Read MII PHY register.	*/
#define SIOCSMIIREG	0x8949		/* Write MII PHY register.	*/

/**************eoc code*********************/
extern struct channel channel;
#include "int6k/int6k-struct.c"
int fd;

struct mdio_bus_t g_mdio_bus={0};


static void parse_read_packet(struct int6k *p, const unsigned int 
phy_address, const unsigned int reg_address)
{
	#ifndef __GNUC__
		#pragma pack (push,1)
	#endif	
	struct __packed vs_mdio_command_request
		{
			struct 		header_eth ethernet;
			struct 		header_int intellon;
			uint8_t 	operation;
			uint8_t 	phy_addr;
			uint8_t 	reg_addr;			
		} *request;
	#ifndef __GNUC__
		#pragma pack (pop)
	#endif	
	
	request = (struct vs_mdio_command_request *)(p->message);
	EthernetHeader (&p->message->ethernet, p->channel->peer, 
		p->channel->host);
	IntellonHeader (&p->message->intellon, (VS_MDIO_MEM | MMTYPE_REQ));
	request->operation = 0;
	request ->phy_addr = phy_address;
	request ->reg_addr = reg_address;	
	p->packetsize = ETHER_MIN_LEN; 
}
static void parse_write_packet(struct int6k *p, const unsigned int 
phy_address, const unsigned int reg_address, 
const unsigned int data)
{

	#ifndef __GNUC__
		#pragma pack (push,1)
	#endif	
	struct __packed vs_mdio_command_request
		{
			struct 		header_eth ethernet;
			struct 		header_int intellon;
			uint8_t 	operation;
			uint8_t 	phy_addr;
			uint8_t 	reg_addr;
			uint16_t   	data;
		} *request;
	#ifndef __GNUC__
		#pragma pack (pop)
	#endif
	
	request = (struct vs_mdio_command_request *)(p->message);
    EthernetHeader (&p->message->ethernet, p->channel->peer, 
			p->channel->host);
	IntellonHeader (&p->message->intellon, (VS_MDIO_MEM | MMTYPE_REQ));
	request->operation = 1;
	request ->phy_addr = phy_address;
	request ->reg_addr = reg_address;
	request -> data = data;
	p->packetsize = ETHER_MIN_LEN; 
}

static int packet_send(struct int6k *p,
int iswrite,
unsigned short data,
unsigned char phy_address,
unsigned char reg_address)
{
	#ifndef __GNUC__
		#pragma pack (push,1)
	#endif
		
	struct __packed vs_mdio_command_confirm
	{
		struct header_eth ethernet;
		struct header_int 	intellon;
		uint8_t MSTATUS;
		uint16_t data;
		uint8_t 	phy_addr;
		uint8_t 	reg_addr;
	}*confirm;
	#ifndef __GNUC__
		#pragma pack (pop)
	#endif
	
	if (SendMME(p) <= 0) {
		return (-1);
	}	

	while(1){
		if (ReadMME(p, (VS_MDIO_MEM | MMTYPE_CNF)) <= 0) {
			return (-2);
		}	
		confirm = (struct vs_mdio_command_confirm *)(int6k.message);
		if(iswrite){
			if(confirm->data == data && confirm->phy_addr==phy_address 
				&& confirm->reg_addr == reg_address){
				return 0;
			}
		}
		else{
			if(confirm->phy_addr==phy_address 
				&& confirm->reg_addr == reg_address){
				return 0;
			}		
		}
	}
	
	return (0);
}

static int  eoc_phy_read(unsigned int phy_address,  unsigned int reg_address, 
unsigned short *data)
{
	int  i;
	int rv=0;
	
	#ifndef __GNUC__
		#pragma pack (push,1)
	#endif
		
	struct __packed vs_mdio_command_confirm
	{
		struct header_eth ethernet;
		struct header_int 	intellon;
		uint8_t MSTATUS;
		uint16_t data;
		uint8_t 	phy_addr;
		uint8_t 	reg_addr;
	}*confirm;
	#ifndef __GNUC__
		#pragma pack (pop)
	#endif

	for(i=0;i<ctl8306_loop_num();i++){
		g_mdio_bus.phy_read_num++;
		confirm = (struct vs_mdio_command_confirm *)(int6k.message);
		parse_read_packet(&int6k, phy_address, reg_address);
		rv = packet_send(&int6k,0,0,phy_address,reg_address);
		if(rv==0){
			if (confirm->MSTATUS) {
				printf("Device refused request\n");
				exit(1);
			}
			
			*data =  confirm->data;

			ttrace("eoc_phy_read %d=%d ok\n",reg_address,*data);

			return 0;
		}
		else{
			ttrace("eoc_phy_read %d err\n",reg_address);
		}
	}

/* 为了兼容脚本，这里需要增加打印 */
	if(rv==-1){
		error ((int6k.flags & INT6K_BAILOUT), ECANCELED, INT6K_CANTSEND);
	}
	else{
		error ((int6k.flags & INT6K_BAILOUT), ECANCELED, INT6K_CANTREAD);
	}
	
	exit(1);
	return rv;
}

static int eoc_phy_write(unsigned int phy_address,  unsigned int reg_address,  unsigned short data)
{
	int i;
	int rv=0;
	
	#ifndef __GNUC__
		#pragma pack (push,1)
	#endif
	struct __packed vs_mdio_command_confirm
	{
		struct header_eth ethernet;
		struct header_int intellon;
		uint8_t MSTATUS;
		uint16_t data;
		
	}*confirm;
	#ifndef __GNUC__
		#pragma pack (pop)
	#endif

	for(i=0;i<ctl8306_loop_num();i++){
		g_mdio_bus.phy_write_num++;
		confirm = (struct vs_mdio_command_confirm *)(int6k.message);
		parse_write_packet(&int6k, phy_address, reg_address,data);
		
		rv = packet_send(&int6k,1,data,phy_address,reg_address);
		if(rv==0){
			if (confirm->MSTATUS) {
				printf("Device refused request");
				exit(1);
			}
			ttrace("eoc_phy_write %d=%d ok\n",reg_address,data);
			return 0;
		}
		else{
			ttrace("eoc_phy_write %d=%d err\n",reg_address,data);
		}
	}
	
/* 为了兼容脚本，这里需要增加打印 */
	if(rv==-1){
		error ((int6k.flags & INT6K_BAILOUT), ECANCELED, INT6K_CANTSEND);
	}
	else{
		error ((int6k.flags & INT6K_BAILOUT), ECANCELED, INT6K_CANTREAD);
	}
	
	exit(1);
	return rv;
}

int mdio_open_remote(const unsigned char * mac_addr, const unsigned char * ifname, struct  mdio_bus_t * bus)
{
	channel.name = (const char *)strdup ((char *)ifname);
	if (channel.name == NULL)
	{
		channel.name="eth0";
	}

	memcpy (channel.peer, mac_addr, sizeof (channel.peer));

	openchannel (&channel);
	if (!(int6k.message = malloc (sizeof (struct message)))) 
	{
		printf("malloc int6k.message failed\n");
		exit(1);
	}	
	bus->phy_read = eoc_phy_read;
	bus->phy_write = eoc_phy_write;
	return 0;
}

/*************一下为ipg的8306的 函数接口**********/

typedef struct {
	unsigned short		phy_id;
	unsigned short		reg_num;
	unsigned short		val_in;
	unsigned short		val_out;
} mii_data;


static int ipg_phy_read(unsigned int phy_addr,unsigned int regnum, unsigned 
short *data)
{
	int ret;
	mii_data  rtl8306_data;
	rtl8306_data.phy_id = phy_addr;
	rtl8306_data.reg_num = regnum;
	rtl8306_data.val_in = 0;
	rtl8306_data.val_out = 0;
	
	g_mdio_bus.phy_read_num++;
	
	ret = ioctl(fd, SIOCGMIIREG, &rtl8306_data);
	
	//printf("read phy%d_reg%d =0x%04x\n", rtl8306_data.phy_id, rtl8306_data.reg_num,   rtl8306_data.val_out);
	if(!ret){
		*data =  rtl8306_data.val_out;
		ttrace("ipg_phy_read %d=%d\n",regnum,*data);
	}
	else{
	//可能需要修改
		printf("ipg_phy_read is failed\n");
		exit(1);
	}
	return 0;
}


static int ipg_phy_write(unsigned int phy_addr,unsigned int regnum,unsigned short val)
{
	int ret;
	mii_data  rtl8306_data;
	rtl8306_data.phy_id = phy_addr;
	rtl8306_data.reg_num = regnum;
	rtl8306_data.val_in = val;
	rtl8306_data.val_out = 0;	
	//printf("write phy%d_reg%d: 0x%04x\n", rtl8306_data.phy_id, rtl8306_data.reg_num,   
//	rtl8306_data.val_in);

	ttrace("ipg_phy_write %d=%d\n",regnum,val);

	g_mdio_bus.phy_write_num++;
	
	ret = ioctl(fd, SIOCSMIIREG, &rtl8306_data);
	if(ret){
		printf("ipg_phy_write is failed\n");
		exit(1);
	}
	return 0;
}

static int s_is_local = 0;
int mdio_open_local(struct  mdio_bus_t * bus){

	//system( "/sbin/mii-tool");

	fd=open("/dev/rtl8306_mdio", O_RDWR);
	if(fd < 0){
		printf("open /dev/rtl8306_mdio failed\n");
		exit(0);	
	}

	//system( "/sbin/mii-tool");
	bus->phy_read = ipg_phy_read;
	bus->phy_write = ipg_phy_write;
	s_is_local=1;
	return 0;
	
}

void mdio_close(void){
	if(s_is_local){
		close(fd);
	}else{
		free (int6k.message);
		closechannel (&channel);
	}
			
}

