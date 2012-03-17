
#include "vns_sys.h"
#include "rtk_api_ext.h"

///////////////////////////////////////////////////////////
// mdio
///////////////////////////////////////////////////////////

static int mdio_read(int argc, char *argv[]){
	return 0;
}

static int mdio_write(int argc, char *argv[]){
	return 0;
}

///////////////////////////////////////////////////////////
// rtk_api_ext.h
///////////////////////////////////////////////////////////

//rtk_switch_*
//TODO: high priority
static int switch_init(int argc, char *argv[]){
	rtk_switch_init();
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

static int port_phyAutoNegoAbility_set(int argc, char *argv[]){
	rtk_port_phyAutoNegoAbility_set(rtk_port_t port,rtk_port_phy_ability_t * pAbility);
	return 0;
}

static int port_phyAutoNegoAbility_get(int argc, char *argv[]){
	//rtk_port_phyAutoNegoAbility_get(rtk_port_t port,rtk_port_phy_ability_t * pAbility);
	return 0;
}

static int port_phyForceModeAbility_set(int argc, char *argv[]){
	//rtk_port_phyForceModeAbility_set(rtk_port_t port,rtk_port_phy_ability_t * pAbility);
	return 0;
}

static int port_phyForceModeAbility_get(int argc, char *argv[]){
	//rtk_port_phyForceModeAbility_get(rtk_port_t port,rtk_port_phy_ability_t * pAbility);
	return 0;
}

static int port_phyStatus_get(int argc, char *argv[]){
	//rtk_port_phyStatus_get(rtk_port_t port,rtk_port_linkStatus_t * pLinkStatus,rtk_data_t * pSpeed,rtk_data_t * pDuplex);
	return 0;
}

static int port_macForceLink_set(int argc, char *argv[]){
	//rtk_port_macForceLink_set(rtk_port_t port,rtk_port_mac_ability_t * pPortability);
	return 0;
}

static int port_macForceLink_get(int argc, char *argv[]){
	//rtk_port_macForceLink_get(rtk_port_t port,rtk_port_mac_ability_t * pPortability);
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

int main(int argc, char *argv[]){
	return 0;
}


