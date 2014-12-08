#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include "tun.h"
#include "log/stitch_log.h"

/* Arguments taken by the function:
 *
 * char *dev: the name of an interface (or '\0'). MUST have enough
 *   space to hold the interface name if '\0' is passed
 */

int tun_alloc(char *dev) {

	struct ifreq ifr;
	int fd, err;
	char *clonedev = "/dev/net/tun";


	/* open the clone device */
	if( (fd = open(clonedev, O_RDWR)) < 0 ) {
		return fd;
	}

	/* preparation of the struct ifr, of type "struct ifreq" */
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

	if (*dev) {
		/* if a device name was specified, put it in the structure; otherwise,
		 *       * the kernel will try to allocate the "next" device of the
		 *             * specified type */
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	/* try to create the device */
	if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
		close(fd);
		return err;
	}

	/* if the operation was successful, write back the name of the
	 * interface to the variable "dev", so the caller can know
	 * it. Note that the caller MUST reserve space in *dev (see calling
	 * code below) 
	  */
	strcpy(dev, ifr.ifr_name);

	/* this is the special file descriptor that the caller will use to talk
	 * with the virtual interface 
	 */
	return fd;
}

/*
 * bring the tunnel device up.                                        
 */
int tun_up (char* dev_name)
{
	struct ifreq ifr;
	int if_fd, status;
	strncpy(ifr.ifr_name, dev_name, IFNAMSIZ);
	/*
	 * Create a socket to be used to configure the ioctl interface
	 */
	if_fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
	if (if_fd < 0 ) {
		STITCH_ERR_LOG("Bad socket for interface %s.\n",
				strerror(errno));
		return -1;
	}

	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	status = ioctl(if_fd, SIOCSIFFLAGS, &ifr);

	if (status < 0) {
		STITCH_ERR_LOG("Could not bring the tunnel interface up %s.\n",                                
				strerror(errno));
		return -1;
	}
	return 0;

}
