#ifndef __STITCH_TUN_H__
#define __STITCH_TUN_H__ 

#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IFTUNNAMESZ 80
int tun_alloc(char *dev, int flags);
#endif
