#ifndef __STITCH_TUN_PKT_HDLR__
#define __STITCH_TUN_PKT_HDLR__
#include <netinet/in.h>
#ifdef OPENSSL_SUPPORT
#include <openssl/sha.h>
#define DIGEST_LEN SHA_DIGEST_LENGTH
#else
#define DIGEST_LEN 64
#endif
#define STITCH_DP_PORT 32000

typedef struct stitch_conn_descr_s {
	char tun_dev[64];
	int stitch_tun_fd;
	int stitch_dp_fd;
	struct in6_addr tun_ip6_addr;
	struct sockaddr_in *stitch_dp_addr;
	struct sockaddr_in6 *stitch_dp_addr6;
	unsigned char stitch_dev_hash[DIGEST_LEN]; // SHA1 hash == 20 bytes
}stitch_conn_descr_t;
#endif//__STITCH_TUN_PKT_HDLR__
