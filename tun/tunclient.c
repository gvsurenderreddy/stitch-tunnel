/* tunclient.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tun.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include "tunclient.h"
#include "stitch_log.h"

char tun_name[IFTUNNAMESZ];
char log_file_name[] = "stitch_tun.log";

int main(int argc, char* argv[]) {
	int tun_fd, if_fd, nread = 0;
	char buffer[2048]; //2k buffer
	/* Connect to the device */
	strcpy(tun_name, "tun0");
	tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);  /* tun interface */
	struct in6_addr ipv6_addr;
	int c, result, prefix_len;
	int status, pid;
	char ip6[128], stitch_dp_ip6[128]; //string representation of IPv6
	char stitch_dp[128]; /*DNS name of the stitch data-plane*/
	struct ifreq ifr;
	struct addrinfo *stitch_dp_addr;

	FILE* log_fd = fopen(log_file_name, "w");

	if (!log_fd) {
		perror("Unable to open the log file");
		exit(1);
	}

	STITCH_INFO_LOG("*****Starting stitch log***********\n");




	if(tun_fd < 0){
		STITCH_ERR_LOG("Allocating interface %s\n", strerror(errno));
		exit(ERR_CODE_TUNN_CREATE);
	}

	/*
	 * bring the tunnel device up.
	 */
	strncpy(ifr.ifr_name, tun_name, IFNAMSIZ);
	/*
	 * Create a socket to be used to configure the ioctl interface
	 */
	if_fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
	if (if_fd < 0 ) {
		STITCH_ERR_LOG("Bad socket for interface %s.\n",
				strerror(errno));
		exit(ERR_CODE_SOCK_CREATE);
	}

	 ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	 status = ioctl(if_fd, SIOCSIFFLAGS, &ifr);

	 if (status < 0) {
		 STITCH_ERR_LOG("Could not bring the tunnel interface up %s.\n", 
				 strerror(errno));
		exit(ERR_CODE_TUN_IF_CFG);
	 }

	 STITCH_DBG_LOG("Interface configuration for %s returned %d with errno:%d\n",  tun_name, status, errno);


	/*
	 * Parse the command line parameters
	 */
	opterr = 0;
	while ((c = getopt (argc, argv, "i:p:s:l")) != -1) {
		switch(c) {
			case  'i':
				/*network address*/
				if ((result = inet_pton(AF_INET6, optarg, &ipv6_addr)) < 1) {
					STITCH_ERR_LOG("Unable to convert argument:%s to IPv6 address.\n", optarg);
					exit(4);
				}
				STITCH_DBG_LOG("Got IPv6 address:%s\n", optarg);
				//store the v6 address as a string representation
				inet_ntop(AF_INET6, &ipv6_addr, ip6, sizeof(ip6));
				STITCH_DBG_LOG("Stored the IPv6 address %s\n", ip6);

				break;
			case 'p':
				/*prefix length*/
				prefix_len = atoi(optarg);
				if (prefix_len <= 0 || prefix_len > 128) {
					STITCH_ERR_LOG("Invalid prefix length %d.!!\n", prefix_len);
					exit(ERR_CODE_INCORRECT_PREFIX_LEN);
				}
				STITCH_DBG_LOG("Got prefix length:%d\n", prefix_len);
				//append it to the ip6 string
				snprintf(ip6+strlen(ip6), sizeof(ip6) - strlen(ip6),"/%d", prefix_len);
				break;
			case 's':
				/*The stitich data-plane module to connect to*/
				strncpy(stitch_dp, optarg, sizeof(stitch_dp));
				/*Resolve the address */
				if (getaddrinfo((const char*)stitch_dp, NULL, NULL, &stitch_dp_addr) < 0 ) {
					STITCH_ERR_LOG("Unable to resolve the Stitch dataplane-module %s:%s\n", 
							stitch_dp, strerror(errno)); 

					exit(ERR_CODE_STITCH_DP);
				}
				if (stitch_dp_addr->ai_family == AF_INET) {
					inet_ntop(AF_INET, 
							&((struct sockaddr_in*)stitch_dp_addr->ai_addr)->sin_addr, 
							stitch_dp_ip6, sizeof(stitch_dp_ip6));
				} else {
					inet_ntop(AF_INET, 
							&((struct sockaddr_in6*)stitch_dp_addr->ai_addr)->sin6_addr, 
							stitch_dp_ip6, sizeof(stitch_dp_ip6));
				}
				STITCH_DBG_LOG("Resolved address for Stitch dataplane-module %s address-family:%d(AF_INET:%d, AF_INET6:%d),"
				" ip:%s\n", stitch_dp, stitch_dp_addr->ai_family, AF_INET, AF_INET6, stitch_dp_ip6);
				break;
			case '?':
				if (optopt == 'i' || optopt == 'p'){
					STITCH_ERR_LOG("Option -%c requires an argument.\n", optopt);
				}
				exit(ERR_CODE_MISSING_ARG);
			default:
				STITCH_ERR_LOG("Unknown option %c.\n", optopt);
				exit(ERR_CODE_UNKOWN_OPT);
		}
	}

	/*
	 * Configure the v6 address on the tunnel interface.
	 */
	pid = fork();
	if (pid == 0) {
		STITCH_DBG_LOG("In child process executing ifconfig command for inet6 address %s\n", ip6);
		status = execv("/bin/ifconfig", (char *[]){"ifconfig", tun_name, "inet6", "add", ip6, NULL});
		if (status < 0) {
			STITCH_ERR_LOG("Error occured executing the execv command:%s(%d)\n", strerror(errno), errno);
		}
		exit(errno);
	}

	if (pid < 0) {
		STITCH_ERR_LOG("Unable to fork ... \n");
		exit(ERR_CODE_FORK);
	} else  {
		STITCH_DBG_LOG("Waiting for the child process %d.\n", pid);
		pid = waitpid(pid, &status, WNOHANG);
		STITCH_DBG_LOG("Child process done.\n");
	}


	/* Now read data coming from the kernel */
	while(1) {
		/* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
		nread = read(tun_fd,buffer,sizeof(buffer));
		if(nread < 0) {
			STITCH_DBG_LOG("Tunnel interface closed.\n");
			close(tun_fd);
			exit(ERR_CODE_TUN_FD_CLOSE);
		}

		/* Do whatever with the data */
		STITCH_DBG_LOG("Read %d bytes from device %s\n", nread, tun_name);
	}
}
