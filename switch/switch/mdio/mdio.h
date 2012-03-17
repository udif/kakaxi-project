#ifndef __MDIO8306_H__
#define __MDIO8306_H__

struct mdio_bus_t{
	int (*phy_read)(unsigned int phy_addr,unsigned int regnum, unsigned short 	*data);
	int (*phy_write)(unsigned int phy_addr,unsigned int regnum,unsigned short val);

	unsigned int phy_read_num;
	unsigned int phy_write_num;
};

extern struct mdio_bus_t g_mdio_bus;
int mdio_open_local(struct  mdio_bus_t * bus);
int mdio_open_remote(const unsigned char * mac_addr, const unsigned char * interface, struct  mdio_bus_t * bus);
void mdio_close(void);
#endif
