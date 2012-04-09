#include <getopt.h>
#include "vns_sys.h"
#include "rtk_api_ext.h"
//#include "rtl8368.h"
#include "rtl8368/smi.h"
#include "mdio.h"


int parse_mask(const char *s, unsigned char *mask){
	return 0;

}


static void print_pability(rtk_uint32 port, rtk_port_phy_ability_t *pAbility){
	printf ("[global]\n");
	printf("port = %d\n", port);
	printf("AutoNegotiation = %d\n", pAbility->AutoNegotiation);
	printf("Half_10 = %d\n", pAbility->Half_10);
	printf("Full_10 = %d\n", pAbility->Full_10);
	printf("Half_100 = %d\n", pAbility->Half_100);
	printf("Full_100 = %d\n", pAbility->Full_100);
	printf("Full_1000 = %d\n", pAbility->Full_1000);
	printf("FC = %d\n", pAbility->FC);
	printf("AsyFC = %d\n", pAbility->AsyFC);
}

static void print_port_ability(rtk_uint32 port, rtk_port_mac_ability_t *Portability){
	printf ("[global]\n");
	printf("port = %d\n", port);
	printf("forcemode = %d\n", Portability->forcemode);
	printf("speed = %d\n", Portability->speed);
	printf("duplex = %d\n", Portability->duplex);
	printf("link = %d\n", Portability->link);
	printf("nway = %d\n", Portability->nway);
	printf("txpause = %d\n", Portability->txpause);
	printf("rxpause = %d\n", Portability->rxpause);
}


///////////////////////////////////////////////////////////
// smi
///////////////////////////////////////////////////////////

const char smi_read_reg_usage[] =
	"mdio_read\n"
	"    --add=i\n"
	"\n";
static int smi_read_reg(int argc, char *argv[]){
	int rv;	
	int c;	
	rtk_uint32 mAddrs = 0;	
	rtk_uint32 rData;			
	static const char * shortopts = "a:";	
	static const struct option longopts[] = {		
		{"add", required_argument, NULL, 'a'},		
		{NULL, 0, NULL, 0}	};	
		
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){		
		switch(c){		
			case 'a':			
				mAddrs = strtol(optarg, NULL, 0);			
				break;		
			default:			
				break;		
		}	
	}		
	rv = smi_read(mAddrs, &rData);	
	if(rv){	
		printf("smi read error!");
		return 1;
	}
	printf("[global]\n");
	printf("reg = %d\n", mAddrs);
	printf("data = 0x%04x\n", rData);
	return 0;

}

const char smi_write_reg_usage[] =
	"mdio_write\n"
	"    --add=i\n"
	"    --data=j\n"
	"\n";
static int smi_write_reg(int argc, char *argv[]){
	int rv;	
	int c;	
	rtk_uint32 mAddrs = 0;	
	rtk_uint32 rData = 0;	
	
	static const char * shortopts = "a:d:";	
	static const struct option longopts[] = {		
		{"add", required_argument, NULL, 'a'},		
		{"data", required_argument, NULL, 'd'},		
		{NULL, 0, NULL, 0}	};	

	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){		
		switch(c){		
			case 'a':			
				mAddrs = strtol(optarg, NULL, 0);			
				break;		
			case 'd':			
				rData = strtol(optarg, NULL, 0);			
				break;		
			default:			
				break;		
			}	
		}		
	rv = smi_write(mAddrs, rData);	
	if(rv){	
		printf("smi write error!");	
		return 1;
	}	
	return 0;

}


///////////////////////////////////////////////////////////
// mdio
///////////////////////////////////////////////////////////
const char mdio_read_usage[] =
	"mdio_read\n"
	"    --pid=i\n"
	"    --rid=i\n"
	"\n";
static int mdio_read(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 preamble_len = 0;
	rtk_uint32 phy_id = 0;
	rtk_uint32 register_id = 0;
	rtk_uint32 pdata;	
	
	static const char * shortopts = "p:r:l:";
	static const struct option longopts[] = {
		{"pid", required_argument, NULL, 'p'},
		{"rid", required_argument, NULL, 'r'},
		{"len", required_argument, NULL, 'l'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p'://phy_id
			phy_id = strtol(optarg, NULL, 0);
			break;
		case 'r'://register_id
			register_id = strtol(optarg, NULL, 0);
			break;
		case 'l'://preamble_len
			preamble_len = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = MDC_MDIO_READ(preamble_len, phy_id, register_id, &pdata);
	if(rv){	
		printf("mdio read error!");	
		return 1;
	}
	printf ("[global]\n");
	printf ("data = 0x%04x\n", pdata);
	return 0;
}

const char mdio_write_usage[] =
	"mdio_write\n"
	"    --pid=i\n"
	"    --rid=j\n"
	"    --data=k\n"
	"\n";

static int mdio_write(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 preamble_len = 0;
	rtk_uint32 phy_id = 0;
	rtk_uint32 register_id = 0;
	rtk_uint32 data = 0;

	static const char * shortopts = "l:p:r:d:";
	static const struct option longopts[] = {
		{"len", required_argument, NULL, 'l'},
		{"pid", required_argument, NULL, 'p'},
		{"rid", required_argument, NULL, 'r'},
		{"data", required_argument, NULL, 'd'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'l':
			preamble_len = strtol(optarg, NULL, 0);
			break;
		case 'p':
			phy_id = strtol(optarg, NULL, 0);
			break;
		case 'r':
			register_id = strtol(optarg, NULL, 0);
			break;
		case 'd':
			data = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = MDC_MDIO_WRITE( preamble_len, phy_id, register_id, data);
	if(rv){
		printf("mdio write error!");
		return 1;
	}
	return 0;
}

///////////////////////////////////////////////////////////
// rtk_api_ext.h
///////////////////////////////////////////////////////////

//rtk_switch_*
//TODO: high priority
const char switch_init_usage[] =
	"switch_init\n"
	"\n";
static int switch_init(int argc, char *argv[]){
	int rv;
	rv = rtk_switch_init();
	if(rv){	
		printf("switch init error!");	
		return 1;
	}
	return 0;
}

//TODO: not now
//rtk_rate_*
//rtk_storm_*
//rtk_qos_*
//rtk_trap_*
//rtk_leaky_*

//rtk_port_*
//TODO: high priority
const char port_phyAutoNegoAbility_set_usage[] =
	"port_phyAutoNegoAbility_set\n"
	"    --port=i\n"
	"    --half10=0/1\n"
	"    --full10=0/1\n"
	"    --half100=0/1\n"
	"    --full100=0/1\n"
	"    --full1000=0/1\n"
	"    --fc=0/1\n"
	"    --asyfc=0/1\n"
	"\n";
static int port_phyAutoNegoAbility_set(int argc, char *argv[]){
	int c;
	int rv;
	rtk_uint32 port = 0;
	rtk_port_phy_ability_t pAbility;

	static const char * shortopts = "p:a:b:c:d:e:f:y:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"half10", required_argument, NULL, 'a'},
		{"full10", required_argument, NULL, 'b'},
		{"half100", required_argument, NULL, 'c'},
		{"full100", required_argument, NULL, 'd'},
		{"full1000", required_argument, NULL, 'e'},
		{"fc", required_argument, NULL, 'f'},
		{"asyfc", required_argument, NULL, 'y'},
		{NULL, 0, NULL, 0}
	};
	memset(&pAbility, 0, sizeof(pAbility));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'a':
			pAbility.Half_10 = strtol(optarg, NULL, 0);
			break;
		case 'b':
			pAbility.Full_10 = strtol(optarg, NULL, 0);
			break;
		case 'c':
			pAbility.Half_100 = strtol(optarg, NULL, 0);
			break;
		case 'd':
			pAbility.Full_100 = strtol(optarg, NULL, 0);
			break;
		case 'e':
			pAbility.Full_1000 = strtol(optarg, NULL, 0);
			break;
		case 'f':
			pAbility.FC = strtol(optarg, NULL, 0);
			break;
		case 'y':
			pAbility.AsyFC= strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	pAbility.AutoNegotiation = 1;
	rv = rtk_port_phyAutoNegoAbility_set( port, &pAbility);
	if(rv){	
		printf("phyAutoNegoAbility set error!");	
		return 1;
	}
	return 0;
}

const char port_phyAutoNegoAbility_get_usage[] =
	"port_phyAutoNegoAbility_get\n"
	"    --port=i\n"
	"\n";
static int port_phyAutoNegoAbility_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_phy_ability_t pAbility;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&pAbility, 0, sizeof(pAbility));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_phyAutoNegoAbility_get(port, &pAbility);
	if(rv){	
		printf("phyAutoNegoAbility get error!");	
		return 1;
	}
	print_pability(port, &pAbility);
	return 0;
}

const char port_phyForceModeAbility_set_usage[] =
	"port_phyForceModeAbility_set\n"
	"    --port=i\n"
	"    --half10=0/1\n"
	"    --full10=0/1\n"
	"    --half100=0/1\n"
	"    --full100=0/1\n"
	"    --full1000=0/1\n"
	"    --fc=0/1\n"
	"    --asyfc=0/1\n"
	"\n";
static int port_phyForceModeAbility_set(int argc, char *argv[]){
	int c;
	int rv;
	rtk_uint32 port = 0;
	rtk_port_phy_ability_t pAbility;

	static const char * shortopts = "p:a:b:c:d:e:f:y:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"half10", required_argument, NULL, 'a'},
		{"full10", required_argument, NULL, 'b'},
		{"half100", required_argument, NULL, 'c'},
		{"full100", required_argument, NULL, 'd'},
		{"full1000", required_argument, NULL, 'e'},
		{"fc", required_argument, NULL, 'f'},
		{"asyfc", required_argument, NULL, 'y'},
		{NULL, 0, NULL, 0}
	};
	memset(&pAbility, 0, sizeof(pAbility));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'a':
			pAbility.Half_10 = strtol(optarg, NULL, 0);
			break;
		case 'b':
			pAbility.Full_10 = strtol(optarg, NULL, 0);
			break;
		case 'c':
			pAbility.Half_100 = strtol(optarg, NULL, 0);
			break;
		case 'd':
			pAbility.Full_100 = strtol(optarg, NULL, 0);
			break;
		case 'e':
			pAbility.Full_1000 = strtol(optarg, NULL, 0);
			break;
		case 'f':
			pAbility.FC = strtol(optarg, NULL, 0);
			break;
		case 'y':
			pAbility.AsyFC= strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_phyForceModeAbility_set(port, &pAbility);
	if(rv){	
		printf("phyForceModeAbility set error!");	
		return 1;
	}
	return 0;
}

const char port_phyForceModeAbility_get_usage[] =
	"port_phyForceModeAbility_get\n"
	"    --port=i\n"
	"\n";
static int port_phyForceModeAbility_get(int argc, char *argv[]){
	int c;
	int rv;
	rtk_uint32 port = 0;
	rtk_port_phy_ability_t pAbility;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&pAbility, 0, sizeof(pAbility));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyForceModeAbility_get(port, &pAbility);
	if(rv){	
		printf("phyForceModeAbility get error!");	
		return 1;
	}
	print_pability(port, &pAbility);
	return 0;
}

const char port_phyStatus_get_usage[] =
	"port_phyStatus_get\n"
	"    --port=i\n"
	"\n";
static int port_phyStatus_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_linkStatus_t pLinkStatus;
	rtk_uint32 pSpeed;
	rtk_uint32 pDuplex;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&pLinkStatus, 0, sizeof(pLinkStatus));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyStatus_get(port, &pLinkStatus, &pSpeed, &pDuplex);
	if(rv){	
		printf("phyStatus get error!");	
		return 1;
	}
	printf("[global]\n");
	printf("port = %d\n", port);
	printf("link = %d\n", pLinkStatus);
	printf("speed = %d\n", pSpeed);
	printf("duplex = %d\n", pDuplex);
	return 0;
}

static int port_phyEnableAll_set(int argc, char *argv[]){
	int rv;
	int c;
	rtk_enable_t enable = ENABLED;

	static const char * shortopts = "e:";
	static const struct option longopts[] = {
		{"enable", required_argument, NULL, 'e'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'e':
			enable = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyEnableAll_set(enable);
	if(rv){	
		printf("phyEnableAll set error!");	
		return 1;
	}
	return 0;
}

static int port_phyEnableAll_get(int argc, char *argv[]){
	int rv;
	rtk_enable_t enable = RTK_ENABLE_END;

	rv = rtk_port_phyEnableAll_get(&enable);
	if(rv){	
		printf("phyEnableAll get error!");	
		return 1;
	}
	printf("[global]\n");
	printf("enable = %d\n", enable);
	return 0;
}

static int port_macForceLink_set(int argc, char *argv[]){
	int c;
	int rv;
	rtk_uint32 port = 0;
	rtk_port_mac_ability_t pPortability;

	static const char * shortopts = "p:m:s:d:l:n:t:r:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"forcemode", required_argument, NULL, 'm'},
		{"speed", required_argument, NULL, 's'},
		{"duplex", required_argument, NULL, 'd'},
		{"link", required_argument, NULL, 'l'},
		{"nway", required_argument, NULL, 'n'},
		{"txpause", required_argument, NULL, 't'},
		{"rxpause", required_argument, NULL, 'r'},
		{NULL, 0, NULL, 0}
	};
	memset(&pPortability, 0, sizeof(pPortability));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'm':
			pPortability.forcemode = strtol(optarg, NULL, 0);
			break;
		case 's':
			pPortability.speed = strtol(optarg, NULL, 0);
			break;
		case 'd':
			pPortability.duplex = strtol(optarg, NULL, 0);
			break;
		case 'l':
			pPortability.link = strtol(optarg, NULL, 0);
			break;
		case 'n':
			pPortability.nway = strtol(optarg, NULL, 0);
			break;
		case 't':
			pPortability.txpause = strtol(optarg, NULL, 0);
			break;
		case 'r':
			pPortability.rxpause = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_macForceLink_set(port, &pPortability);
	if(rv){	
		printf("macForceLink set error!");	
		return 1;
	}
	return 0;
}

static int port_macForceLink_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_mac_ability_t pPortability;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&pPortability, 0, sizeof(pPortability));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_macForceLink_get(port, &pPortability);
	if(rv){	
		printf("macForceLink get error!");	
		return 1;
	}
	//print_result(port, &pPortability);
	return 0;
}

const char port_macForceLink_set_usage[] =
	"port_macForceLink_set\n"
	"    --port=i\n"
	"    --forcemode=i\n"
	"    --speed=i\n"
	"    --duplex=i\n"
	"    --link=i\n"
	"    --nway=i\n"
	"    --txpause=i\n"
	"    --rxpause=i\n"
	"\n";
static int port_macForceLinkExt_set(int argc, char *argv[]){
	int c;
	int rv;
	rtk_uint32 port = 0;
	rtk_mode_ext_t mode = MODE_EXT_DISABLE;
	rtk_port_mac_ability_t pPortability;

	static const char * shortopts = "p:m:f:s:d:l:n:t:r:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"mode", required_argument, NULL, 'm'},
		{"forcemode", required_argument, NULL, 'f'},
		{"speed", required_argument, NULL, 's'},
		{"duplex", required_argument, NULL, 'd'},
		{"link", required_argument, NULL, 'l'},
		{"nway", required_argument, NULL, 'n'},
		{"txpause", required_argument, NULL, 't'},
		{"rxpause", required_argument, NULL, 'r'},
		{NULL, 0, NULL, 0}
	};
	memset(&pPortability, 0, sizeof(pPortability));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'm':
			mode = strtol(optarg, NULL, 0);
			break;
		case 'f':
			pPortability.forcemode = strtol(optarg, NULL, 0);
			break;
		case 's':
			pPortability.speed = strtol(optarg, NULL, 0);
			break;
		case 'd':
			pPortability.duplex = strtol(optarg, NULL, 0);
			break;
		case 'l':
			pPortability.link = strtol(optarg, NULL, 0);
			break;
		case 'n':
			pPortability.nway = strtol(optarg, NULL, 0);
			break;
		case 't':
			pPortability.txpause = strtol(optarg, NULL, 0);
			break;
		case 'r':
			pPortability.rxpause = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_macForceLinkExt_set(port, mode, &pPortability);
	if(rv){	
		printf("macForceLinkExt set error!");	
		return 1;
	}
	return 0;
}

const char port_macForceLink_get_usage[] =
	"port_macForceLink_get\n"
	"    --port=i\n"
	"\n";
static int port_macForceLinkExt_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_mode_ext_t mode;
	rtk_port_mac_ability_t Portability;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&Portability, 0, sizeof(Portability));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_macForceLinkExt_get(port, &mode, &Portability);
	if(rv){	
		printf("macForceLinkExt get error!");	
		return 1;
	}
	print_port_ability(port, &Portability);
	return 0;
}

static int port_macStatus_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_mac_ability_t Portstatus;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	memset(&Portstatus, 0, sizeof(Portstatus));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_macStatus_get(port, &Portstatus);
	if(rv){	
		printf("macStatus get error!");	
		return 1;
	}
	print_port_ability(port, &Portstatus);
	return 0;
}

static int port_phyComboPortMedia_set(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_media_t media = PORT_MEDIA_COPPER;

	static const char * shortopts = "p:m:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"media", required_argument, NULL, 'm'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'm':
			media = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyComboPortMedia_set(port, media);
	if(rv){	
		printf("phyComboPortMedia set error!");	
		return 1;
	}
	printf("rv = %d\n", rv);
	return 0;
}

static int port_phyComboPortMedia_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_media_t media = PORT_MEDIA_COPPER;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyComboPortMedia_get(port, &media);
	if(rv){	
		printf("phyComboPortMedia get error!");	
		return 1;
	}
	printf("[global]\n");
	printf("port = %d\n", port);
	printf("media = %d\n", media);
	return 0;
}

static int port_phyReg_set(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_phy_reg_t reg = PHY_REG_CONTROL;
	rtk_port_phy_data_t regData = 0;
	
	static const char * shortopts = "p:r:d:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"reg", required_argument, NULL, 'r'},
		{"data", required_argument, NULL, 'd'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'r':
			reg = strtol(optarg, NULL, 0);
			break;
		case 'd':
			regData = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	rv = rtk_port_phyReg_set(port, reg, regData);
	if(rv){	
		printf("port phyReg set error!");	
		return 1;
	}
	
	return 0;
}

static int port_phyReg_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_uint32 port = 0;
	rtk_port_phy_reg_t reg = PHY_REG_CONTROL;
	rtk_port_phy_data_t pData;

	static const char * shortopts = "p:r:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"reg", required_argument, NULL, 'r'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'r':
			reg = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_port_phyReg_get(port, reg, &pData);
	if(rv){	
		printf("port phyReg get error!");	
		return 1;
	}
	printf("[global]\n");
	printf("port = %d\n", port);
	printf("reg = %d\n", reg);
	printf("value = %d\n", pData);
	return 0;
}

static int port_rtctEnable_set(int argc, char *argv[]){
#if 0
	int rv;
	int c;
	rtk_portmask_t portmask;

	static const char * shortopts = "m:";
	static const struct option longopts[] = {
		{"mask", required_argument, NULL, 'm'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'm':
			//parse_mask(optarg, portmask.bits);
			break;
		default:
			break;
		}
	}

	
	rv = rtk_port_rtctEnable_set(portmask);
	if(rv){	
		printf("rtctEnable set error!");	
		return 1;
	}
	printf("rv = %d\n", rv);
#endif	
	return 0;

}

static int port_rtctResult_get(int argc, char *argv[]){
#if 0

	int rv;
	int c;

	rtk_port_t port;
	rtk_rtctResult_t pRtctResult;
		
	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}

	rv = rtk_port_rtctResult_get( port, pRtctResult);
	if(rv){	
		printf("rtctEnable get error!");	
		return 1;
	}
	printf("rv = %d\n", rv);
#endif
	return 0;
}


const char acl_init_usage[] =
	"acl_init\n"
	"\n";
static int acl_init(int argc, char *argv[]){
	int rv;
	rv = rtk_filter_igrAcl_init();
	if(rv){	
		printf("acl init error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_template_set_usage[] =
	"acl_template_set\n"
	"\n";
static int acl_template_set(int argc, char *argv[]){
	int rv;
	rtk_filter_template_t aclTemplate;
	int index = 0;
	
	memset(&aclTemplate, 0x00, sizeof(rtk_filter_template_t));

	aclTemplate.Index = 2;
	aclTemplate.fieldType[0] = FILTER_FIELD_RAW_DMAC_15_0;
	aclTemplate.fieldType[1] = FILTER_FIELD_RAW_DMAC_31_16;
	aclTemplate.fieldType[2] = FILTER_FIELD_RAW_DMAC_47_32;
	aclTemplate.fieldType[3] = FILTER_FIELD_RAW_SMAC_15_0;
	aclTemplate.fieldType[4] = FILTER_FIELD_RAW_SMAC_31_16;
	aclTemplate.fieldType[5] = FILTER_FIELD_RAW_SMAC_47_32;
	aclTemplate.fieldType[6] = FILTER_FIELD_RAW_CTAG;
	aclTemplate.fieldType[7] = FILTER_FIELD_RAW_STAG;

	
	rv = rtk_filter_igrAcl_template_set(&aclTemplate);
	if(rv){	
		printf("acl template set error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_field_add_usage[] =
	"acl_field_add\n"
	"\n";
static int acl_field_add(int argc, char *argv[]){
	int rv;
	rtk_filter_cfg_t pFilter_cfg;
	rtk_filter_field_t pFilter_field;
	
	rv = rtk_filter_igrAcl_field_add(&pFilter_cfg, &pFilter_field);
	if(rv){	
		printf("acl field add error!");	
		return 1;
	}
	return 0;
}


//TODO
const char acl_field_sel_set_usage[] =
	"acl_field_sel_set\n"
	"\n";
static int acl_field_sel_set(int argc, char *argv[]){
	int rv;
	rtk_uint32 index = 0;
	rtk_field_sel_t format = 0;
	rtk_uint32 offset = 0;
	
	rv = rtk_filter_igrAcl_field_sel_set(index, format, offset);
	if(rv){	
		printf("acl field add error!");	
		return 1;
	}
	return 0;
}



//TODO
const char acl_cfg_add_usage[] =
	"acl_cfg_add\n"
	"\n";
static int acl_cfg_add(int argc, char *argv[]){
	int rv;
	rtk_filter_id_t filter_id = 0;
	rtk_filter_cfg_t pFilter_cfg;
	rtk_filter_action_t pFilter_action;
	rtk_filter_number_t ruleNum;
	
	rv = rtk_filter_igrAcl_cfg_add(filter_id, &pFilter_cfg, &pFilter_action, &ruleNum);
	if(rv){	
		printf("acl field add error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_cfg_get_usage[] =
	"acl_cfg_get\n"
	"\n";
static int acl_cfg_get(int argc, char *argv[]){
	int rv;
	rtk_filter_id_t filter_id = 0;
	rtk_filter_cfg_raw_t pFilter_cfg;
	rtk_filter_action_t pAction;
	
	rv = rtk_filter_igrAcl_cfg_get(filter_id, &pFilter_cfg , &pAction);
	if(rv){	
		printf("acl field add error!");	
		return 1;
	}
	return 0;
}
//TODO
const char acl_cfg_del_usage[] =
	"acl_cfg_del\n"
	"\n";
static int acl_cfg_del(int argc, char *argv[]){
	int rv;
	rtk_filter_id_t filter_id = 0;
	
	rv = rtk_filter_igrAcl_cfg_del(filter_id);
	if(rv){	
		printf("acl field add error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_state_set_usage[] =
	"acl_state_set\n"
	"\n";
static int acl_state_set(int argc, char *argv[]){
	int rv;
	rtk_port_t port = 0;
	rtk_filter_state_t state = 0;
	
	rv = rtk_filter_igrAcl_state_set(port, state);
	if(rv){	
		printf("acl state set error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_iprange_set_usage[] =
	"acl_iprange_set\n"
	"\n";
static int acl_iprange_set(int argc, char *argv[]){
	int rv;
	rtk_uint32 index = 0;
	rtk_filter_iprange_t type = 0;
	ipaddr_t upperIp = 0;
	ipaddr_t lowerIp = 0;
	
	rv = rtk_filter_iprange_set(index, type, upperIp, lowerIp);
	if(rv){	
		printf("acl state set error!");	
		return 1;
	}
	return 0;
}

//TODO
const char acl_vidrange_set_usage[] =
	"acl_vidrange_set\n"
	"\n";
static int acl_vidrange_set(int argc, char *argv[]){
	int rv;
	rtk_uint32 index = 0;
	rtk_filter_vidrange_t type;
	rtk_uint32 upperVid = 0;
	rtk_uint32 lowerVid = 0;
	
	rv = rtk_filter_vidrange_set(index, type, upperVid, lowerVid);
	if(rv){	
		printf("acl state set error!");	
		return 1;
	}
	return 0;
}

/******************************************************
*vlan set
*******************************************************/
const char vlan_init_usage[] =
	"vlan_init\n"
	"\n";
static int vlan_init(int argc, char *argv[]){
	int rv;
	rv = rtk_vlan_init();
	if(rv){ 
		printf("vlan init error!"); 
		return 1;
	}
	return 0;
}


const char vlan_set_usage[] =
	"vlan_set\n"
	"\n";
static int vlan_set(int argc, char *argv[]){
	int rv;
	int c;
	rtk_vlan_t vid = 1;
	rtk_portmask_t mbrmsk;
	rtk_portmask_t untagmsk;
	rtk_fid_t fid = 0;

	static const char * shortopts = "v:m:u:f:";
	static const struct option longopts[] = {
		{"vid", required_argument, NULL, 'v'},
		{"mbrmsk", required_argument, NULL, 'm'},
		{"untagmsk", required_argument, NULL, 'u'},
		{"fid", required_argument, NULL, 'f'},
		{NULL, 0, NULL, 0}
	};
	memset(&mbrmsk, 0, sizeof(mbrmsk));
	memset(&untagmsk, 0, sizeof(untagmsk));
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			vid = strtol(optarg, NULL, 0);
			break;
		case 'm':
			mbrmsk.bits[0] = strtol(optarg, NULL, 0);
			break;
		case 'u':
			untagmsk.bits[0] = strtol(optarg, NULL, 0);
			break;
		case 'f':
			fid = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_vlan_set(vid, mbrmsk, untagmsk, fid);
	if(rv){ 
		printf("vlan set error!"); 
		return 1;
	}
	return 0;
}


const char vlan_get_usage[] =
	"vlan_get\n"
	"\n";
static int vlan_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_vlan_t vid = 1;
	rtk_portmask_t pMbrmsk;
	rtk_portmask_t pUntagmsk;
	rtk_fid_t pFid;

	static const char * shortopts = "v:";
	static const struct option longopts[] = {
		{"vid", required_argument, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			vid = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}
	
	rv = rtk_vlan_get(vid, &pMbrmsk, &pUntagmsk, &pFid);
	if(rv){ 
		printf("vlan get error!"); 
		return 1;
	}
	printf("[global]\n");
	printf("vid = %d\n", vid);
	printf("pMbrmsk_bit0 = %d", pMbrmsk.bits[0]);
	printf("pUntagmsk_bit0 = %d", pUntagmsk.bits[0]);
	printf("pFid = %d\n", pFid);
	return 0;
}



const char vlan_portpvid_set_usage[] =
	"vlan_portpvid_set\n"
	"\n";
static int vlan_portpvid_set(int argc, char *argv[]){
	int rv;
	int c;
	rtk_port_t port = 0;
	rtk_vlan_t pvid = 1;
	rtk_pri_t priority = 0;

	static const char * shortopts = "p:v:r:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{"pvid", required_argument, NULL, 'v'},
		{"priority", required_argument, NULL, 'r'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 'v':
			pvid = strtol(optarg, NULL, 0);
			break;
		case 'r':
			priority = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}

	rv = rtk_vlan_portPvid_set(port, pvid, priority);
	if(rv){ 
		printf("vlan portPvid set error!"); 
		return 1;
	}
	return 0;
}

const char vlan_portpvid_get_usage[] =
	"vlan_portpvid_get\n"
	"\n";
static int vlan_portpvid_get(int argc, char *argv[]){
	int rv;
	int c;
	rtk_port_t port = 0;
	rtk_vlan_t pPvid;
	rtk_pri_t pPiority;

	static const char * shortopts = "p:";
	static const struct option longopts[] = {
		{"port", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	while((c = getopt_long(argc,argv,shortopts,longopts,NULL)) != -1){
		switch(c){
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		default:
			break;
		}
	}

	rv = rtk_vlan_portPvid_get(port, &pPvid, &pPiority);
	if(rv){ 
		printf("vlan portPvid get error!"); 
		return 1;
	}
	printf("[global]\n");
	printf("port = %d\n", port);
	printf("pPvid = %d\n", pPvid);
	printf("pPiority = %d\n", pPiority);
	
	return 0;
}




//TODO: not now
//rtk_vlan_*
//rtk_stp_*
//rtk_l2_*
//rtk_svlan_*
//rtk_cpu_*
//rtk_dot1x_*
//rtk_trunk_*
//rtk_mirror_*
//rtk_stat_*
//rtk_int_*
//rtk_led_*
//rtk_filter_*
//rtk_eee_*
//rtk_igmp_*



///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
static int cmd_help(int argc, char *argv[]){
	//int i;
	return 0;
}

static struct{
	const char *cmd;
	const char *usage;
	int (*func)(int argc, char *argv[]);
}g_cmd_table[] = {
	// mdio
	{"mdio_read", mdio_read_usage, mdio_read},
	{"mdio_write", mdio_write_usage, mdio_write},
	//switch_init
	{"switch_init", switch_init_usage, switch_init},
	//AutoNegoAbility
	{"port_phyAutoNegoAbility_set", port_phyAutoNegoAbility_set_usage, port_phyAutoNegoAbility_set},
	{"port_phyAutoNegoAbility_get", port_phyAutoNegoAbility_get_usage, port_phyAutoNegoAbility_get},
	//ForceModeAbility
	{"port_phyForceModeAbility_set", port_phyForceModeAbility_set_usage, port_phyForceModeAbility_set},
	{"port_phyForceModeAbility_get", port_phyForceModeAbility_get_usage, port_phyForceModeAbility_get},
	//phyStatus
	{"port_phyStatus_get", port_phyStatus_get_usage, port_phyStatus_get},

	{"port_phyEnableAll_set", NULL, port_phyEnableAll_set},
	{"port_phyEnableAll_get", NULL, port_phyEnableAll_get},
	//macForceLink
	{"port_macForceLink_set", port_macForceLink_set_usage, port_macForceLink_set},
	{"port_macForceLink_get", port_macForceLink_get_usage, port_macForceLink_get},
	{"port_macForceLinkExt_set", NULL, port_macForceLinkExt_set},
	{"port_macForceLinkExt_get", NULL, port_macForceLinkExt_get},

	{"port_macStatus_get", NULL, port_macStatus_get},
	{"port_phyComboPortMedia_set", NULL, port_phyComboPortMedia_set},
	{"port_phyComboPortMedia_get", NULL, port_phyComboPortMedia_get},

	{"port_phyReg_get", NULL, port_phyReg_get},
	{"port_phyReg_set", NULL, port_phyReg_set},
	
	{"port_rtctEnable_set", NULL, port_rtctEnable_set},
	{"port_rtctResult_get", NULL, port_rtctResult_get},

	{"smi_read_reg", smi_read_reg_usage, smi_read_reg},
	{"smi_write_reg", smi_write_reg_usage, smi_write_reg},
	
	{"acl_init", acl_init_usage, acl_init},
	{"acl_template_set", acl_template_set_usage, acl_template_set},
	{"acl_cfg_add", acl_cfg_add_usage, acl_cfg_add},
	{"acl_cfg_get", acl_cfg_get_usage, acl_cfg_get},
	{"acl_field_add", acl_cfg_add_usage, acl_field_add},
	{"acl_state_set", acl_cfg_get_usage, acl_state_set},

	{"vlan_init", vlan_init_usage, vlan_init},
	{"vlan_set", vlan_set_usage, vlan_set},
	{"vlan_get", vlan_get_usage, vlan_get},
	{"vlan_portpvid_set", vlan_portpvid_set_usage, vlan_portpvid_set},
	{"vlan_portpvid_get", vlan_portpvid_get_usage, vlan_portpvid_get},


	

	{NULL, NULL, NULL}
};


int main(int argc, char *argv[]){
	int i;
	if(strcmp("help", argv[1]) == 0){
		cmd_help(argc, argv);
		return 0;
	}	

	mdio_open_local(&g_mdio_bus); 
	for(i = 0; i < sizeof(g_cmd_table)/sizeof(g_cmd_table[0])-1; ++i){
		if(strcmp(g_cmd_table[i].cmd, argv[1]) == 0){
			return g_cmd_table[i].func(argc, argv);
		}
	}
	mdio_close();
	return 1;
}


