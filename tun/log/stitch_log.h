#ifndef __STITCH_LOG_H__
#define __STITCH_LOG_H__
extern FILE* log_fd;
#define STITCH_ERR_LOG(...) \
	fprintf(log_fd, "ERROR:%s:",__FUNCTION__);\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#define STITCH_INFO_LOG(...) \
	fprintf(log_fd, "INFO:%s:", __FUNCTION__);\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#define STITCH_DBG_LOG(...) \
	fprintf(log_fd, "DEBUG:%s:",__FUNCTION__);\
	fprintf(log_fd, __VA_ARGS__);\
	fflush(log_fd);
#endif//__STITCH_LOG_H__
