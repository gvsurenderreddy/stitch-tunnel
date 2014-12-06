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

char tun_name[IFTUNNAMESZ];

int main(int argc, char* argv[]) {
	int tun_fd, if_fd, nread = 0;
	char buffer[2048]; //2k buffer
	/* Connect to the device */
	strcpy(tun_name, "tun0");
	tun_fd = tun_alloc(tun_name, IFF_TUN | IFF_NO_PI);  /* tun interface */
	struct in6_addr ipv6_addr;
	int c, result, prefix_len;
	int status, pid;
	char ip6[128]; //string representation of IPv6
	struct ifreq ifr;

	if(tun_fd < 0){
		perror("Allocating interface");
		exit(1);
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
		perror("Bad socket for interface");
		exit(2);
	}

	 ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

	 status = ioctl(if_fd, SIOCSIFFLAGS, &ifr);

	 if (status < 0) {
	 	perror("Could not bring the tunnel interface up");
		exit(3);
	 }

	 printf("Interface configuration for %s returned %d with errno:%d\n",  tun_name, status, errno);


	/*
	 * Parse the command line parameters
	 */
	opterr = 0;
	while ((c = getopt (argc, argv, "i:p:")) != -1) {
		switch(c) {
			case  'i':
				/*network address*/
				if ((result = inet_pton(AF_INET6, optarg, &ipv6_addr)) < 1) {
					printf("Unable to convert argument:%s to IPv6 address.\n", optarg);
				}
				printf(" Got IPv6 address:%s\n", optarg);
				//store the v6 address as a string representation
				inet_ntop(AF_INET6, &ipv6_addr, ip6, sizeof(ip6));
				printf("Stored the IPv6 address %s\n", ip6);

				break;
			case 'p':
				/*prefix length*/
				prefix_len = atoi(optarg);
				if (prefix_len <= 0 || prefix_len > 128) {
					printf("Invalid prefix length %d.!!\n", prefix_len);
					return 1;
				}
				printf("Got prefix length:%d\n", prefix_len);
				//append it to the ip6 string
				snprintf(ip6+strlen(ip6), sizeof(ip6) - strlen(ip6),"/%d", prefix_len);
				break;
			case '?':
				if (optopt == 'i' || optopt == 'p'){
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				}
				return 1;
			default:
				fprintf(stderr, "Unknown option %c.\n", optopt);
				exit(-1);
		}
	}

	/*
	 * Configure the v6 address on the tunnel interface.
	 */
	pid = fork();
	if (pid == 0) {
		printf("In child process executing ifconfig command for inet6 address %s\n", ip6);
		status = execv("/bin/ifconfig", (char *[]){"ifconfig", tun_name, "inet6", "add", ip6, NULL});
		if (status < 0) {
			printf("Error occured executing the execv command:%s(%d)\n", strerror(errno), errno);
		}
		exit(errno);
	}

	if (pid < 0) {
		printf("Unable to fork ... \n");
		return -1;
	} else  {
		printf("Waiting for the child process %d.\n", pid);
		pid = waitpid(pid, &status, WNOHANG);
		printf("Child process done.\n");
	}


	/* Now read data coming from the kernel */
	while(1) {
		/* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
		nread = read(tun_fd,buffer,sizeof(buffer));
		if(nread < 0) {
			perror("Reading from interface");
			close(tun_fd);
			exit(1);
		}

		/* Do whatever with the data */
		printf("Read %d bytes from device %s\n", nread, tun_name);
	}
}
