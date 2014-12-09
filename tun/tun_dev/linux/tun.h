#ifndef __STITCH_TUN_H__
#define __STITCH_TUN_H__ 

#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IFTUNNAMESZ 80

/*Error codes*/
int tun_alloc(char *dev);
int tun_up(char *dev);
int tun_ip_config(char* dev, char* ip6_addr); 
#endif
