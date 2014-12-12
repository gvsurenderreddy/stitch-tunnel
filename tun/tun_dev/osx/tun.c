#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#include "log/stitch_log.h"


int tun_alloc(char* dev) 
{
	char dev_name[80];
	int fd;

	snprintf(dev_name, sizeof(dev_name), "/dev/%s", dev);
	STITCH_DBG_LOG("Opening device %s\n", dev_name);
	
	fd = open(dev_name, O_RDWR);

	if (fd < 0) {
		STITCH_ERR_LOG("Failed to open tun/tap device. Are you root? Are the drivers installed?\n");
		return -1;
	}

	return fd;
}

/*
 * Just use the platform specific ip config command.
 */
int tun_ip_config(char* dev, char* ip6_addr) 
{
	int status;
	status = execv("/sbin/ifconfig", (char *[]){"ifconfig", dev, 
					"inet6", ip6_addr, NULL});

	if (status < 0) {
		STITCH_ERR_LOG("Error occured executing the execv command:%s(%d)\n", strerror(errno), errno);
	}
	exit(errno);
}


/*
 * In OS X the tunnels are always up.
 */
int tun_up(char* dev __attribute__((unused)))
{
	return 0;
}

