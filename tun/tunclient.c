/* tunclient.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include "tunclient.h"
#include "log/stitch_log.h"
#include "tun_dev/tun_dev.h"
#include "pkt_process/tun_pkt_hdlr.h"
#include "pkt_process/tun_pkt_recv.h"
#include "pkt_process/tun_pkt_send.h"

char log_file_name[] = "stitch_tun.log";
FILE* log_fd;

int main(int argc, char* argv[]) {
	int if_fd;
	/* Connect to the device */
	int c, result, prefix_len;
	int status, pid;
	char ip6[128], stitch_dp_ip6[128]; //string representation of IPv6
	char stitch_dp[128]; /*DNS name of the stitch data-plane*/
	struct addrinfo *stitch_dp_addr;
	stitch_conn_descr_t stitch_conn;
	pthread_t recv_thread, snd_thread;
	int *th_retval;
	struct sockaddr_in cli_udp_addr;
	struct sockaddr_in6 cli_udp_addr6;
	struct sockaddr *cli_udp;
	socklen_t cli_udp_addr_len;

	strcpy(stitch_conn.tun_dev, "tun0");
	stitch_conn.stitch_tun_fd = tun_alloc(stitch_conn.tun_dev);  /* tun interface */
	log_fd = fopen(log_file_name, "w");

	if (!log_fd) {
		perror("Unable to open the log file");
		exit(ERR_CODE_LOG);
	}

	STITCH_INFO_LOG("*****Starting stitch log***********\n");




	if(stitch_conn.stitch_tun_fd < 0){
		STITCH_ERR_LOG("Allocating interface %s\n", strerror(errno));
		exit(ERR_CODE_TUNN_CREATE);
	}

	/* Tunnel creation successful. Now bring the tunnel up*/
	status = tun_up(stitch_conn.tun_dev);

	if (status) {
		/* We were not able to bring the tunnel device up*/
		STITCH_ERR_LOG("We were not able to bring the tunnel up:%d\n", status);
		exit(ERR_CODE_TUNN_UP);
	}


	/*
	 * Parse the command line parameters
	 */
	opterr = 0;
	while ((c = getopt (argc, argv, "i:p:s:l")) != -1) {
		switch(c) {
			case  'i':
				/*network address*/
				if ((result = inet_pton(AF_INET6, optarg, &stitch_conn.tun_ip6_addr)) < 1) {
					STITCH_ERR_LOG("Unable to convert argument:%s to IPv6 address.\n", optarg);
					exit(4);
				}
				STITCH_DBG_LOG("Got IPv6 address:%s\n", optarg);
				//store the v6 address as a string representation
				inet_ntop(AF_INET6, &stitch_conn.tun_ip6_addr, ip6, sizeof(ip6));
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
					stitch_conn.stitch_dp_addr = (struct sockaddr_in*)stitch_dp_addr->ai_addr;
					//set the stitch DP port. Ideally this should be coming
					//from the webservice.
					stitch_conn.stitch_dp_addr->sin_port = htons(STITCH_DP_PORT);
					cli_udp_addr.sin_family = AF_INET;
					cli_udp_addr.sin_port = htons(STITCH_DP_PORT);
					cli_udp_addr.sin_addr.s_addr = INADDR_ANY;
					cli_udp = (struct sockaddr*)&cli_udp_addr;
					cli_udp_addr_len = sizeof(cli_udp_addr);
				} else {
					inet_ntop(AF_INET, 
							&((struct sockaddr_in6*)stitch_dp_addr->ai_addr)->sin6_addr, 
							stitch_dp_ip6, sizeof(stitch_dp_ip6));
					stitch_conn.stitch_dp_addr6 = (struct sockaddr_in6*)stitch_dp_addr->ai_addr;
					stitch_conn.stitch_dp_addr6->sin6_port = htons(STITCH_DP_PORT);
					memset(&cli_udp_addr6, 0, sizeof(cli_udp_addr6));
					cli_udp_addr.sin_family = AF_INET6;
					cli_udp_addr.sin_port = htons(STITCH_DP_PORT);
					cli_udp = (struct sockaddr*)&cli_udp_addr6;
					cli_udp_addr_len = sizeof(cli_udp_addr6);
					
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
		status = execv("/bin/ifconfig", (char *[]){"ifconfig", stitch_conn.tun_dev, "inet6", "add", ip6, NULL});
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
		if (status != 0) {
			STITCH_ERR_LOG("Was not able to configure IP address on tunnel interface:%d",
			status);
			exit(status);
		}
		STITCH_DBG_LOG("Child process done.\n");
	}

	/*Create a UDP socket to create the external tunnel to the stitch datapath-plane*/
	stitch_conn.stitch_dp_fd = socket(stitch_dp_addr->ai_family, SOCK_DGRAM, 0) ;
	if (stitch_conn.stitch_dp_fd < 0 ) {
		STITCH_ERR_LOG("Unable to create the stitch-datapath socket:%s\n", strerror(errno));
		exit(ERR_CODE_STITCH_DP_SOCKET);
	}
	/* bind the socket to a particular port */
	bind(stitch_conn.stitch_dp_fd, (const struct sockaddr*) cli_udp, cli_udp_addr_len);

	/*
	 * Create the receive thread.
	 */
	pthread_create(&recv_thread, NULL, &recv_tun, &stitch_conn);
	pthread_create(&snd_thread, NULL, &send_tun, &stitch_conn);
	pthread_join(recv_thread, (void**)&th_retval);
	STITCH_DBG_LOG("Child thread returned with value:%p.\n", th_retval);
	pthread_join(snd_thread, (void**)&th_retval);
	STITCH_DBG_LOG("Child thread returned with value:%p.\n", th_retval);
	exit(0);

}
