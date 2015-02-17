#ifndef __STITCH_TUN_H__
#define __STITCH_TUN_H__ 
#define IFTUNNAMESZ 80

int tun_alloc(char *dev);
int tun_up(char *dev);
int tun_ip_config(char* dev, char* ip6_addr);
int tun_set_mtu(char* dev, int mtu);
#endif
