/*
 * Packet reception from the tunnel interface will be handled here.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>       // IPPROTO_TCP, INET6_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <netinet/ip6.h>      // struct ip6_hdr
#include <net/ethernet.h>
#include "tun_pkt_hdlr.h"
#include "tun_pkt_recv.h"
#include "stitch_log.h"

void* recv_tun(void* stitch_conn)
{
	int nread, nwrite;
	char buffer[2048];
	stitch_conn_descr_t *stitch_descr = (stitch_conn_descr_t*)stitch_conn;
	STITCH_DBG_LOG("In child thread, waiting for packets from the tunnel.\n");
	struct ip6_hdr *ip6hdr;
	uint8_t ipver;
	while(1) {
		/* 
		 * Note that "buffer" should be at least the MTU size of the interface, 
		 * eg. 1500 bytes 
		 * */               
		nread = read(stitch_descr->stitch_tun_fd,buffer,sizeof(buffer));
		if(nread < 0) {
			STITCH_DBG_LOG("Tunnel interface closed.\n");
			close(stitch_descr->stitch_tun_fd);
			pthread_exit((void*)(-1));
		}

		/* Do whatever with the data */
		STITCH_DBG_LOG("Read %d bytes from device %s\n", nread, stitch_descr->tun_dev);
		/* We are forwarding only IP packets */
		ip6hdr = (struct ip6_hdr*) buffer;
		ipver = ip6hdr->ip6_ctlun.ip6_un2_vfc >> 4;
		STITCH_DBG_LOG("ipver:%d\n", ipver);
		if (ipver == 6) {
			STITCH_DBG_LOG("Read an IP6 frame \n");
			/* Send to stitch-DP . Kernel does UDP encapsulation*/
			if (stitch_descr->stitch_dp_addr) {
				/* IPv4 */
				if ((nwrite = 	sendto(stitch_descr->stitch_dp_fd, buffer, nread, 0, 
								(struct sockaddr*) stitch_descr->stitch_dp_addr, 
								sizeof(struct sockaddr_in))) < 0) {
					STITCH_ERR_LOG("UDP socke to stitch-dp close with error:%s\n", 
							strerror(errno));
					pthread_exit((void*)(-1));

				};
			} else {
				/* IPv6 */
				if ( (nwrite = sendto(stitch_descr->stitch_dp_fd, buffer, nread, 0, 
								(struct sockaddr*) stitch_descr->stitch_dp_addr6, 
								sizeof(struct sockaddr_in6))) < 0) {
					STITCH_ERR_LOG("UDP socket to stitch-dp close with error:%s\n", 
							strerror(errno));
					pthread_exit((void*)(-1));
				};
			}
		} else {
			/* We should never expect anything other an IPv6 frame here */
			STITCH_DBG_LOG("Received a non-IPv6 frame.\n");
		}
	}

	pthread_exit(0);
}

