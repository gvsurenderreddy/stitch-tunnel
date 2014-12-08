#ifndef __STITCH_LOG_H__
#define __STITCH_LOG_H__
extern FILE* log_fd;
#define STITCH_INFO_LOG(...) \
	fprintf(log_fd, "INFO:");\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#define STITCH_DBG_LOG(...) \
	fprintf(log_fd, "DEBUG:");\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);

#define STITCH_ERR_LOG(...) \
	fprintf(log_fd, "ERROR:");\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#define STITCH_DBG_LOG(...) \
	fprintf(log_fd, "DEBUG:");\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#endif//__STITCH_LOG_H__
