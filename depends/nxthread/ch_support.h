#ifndef __spporting_hpp__
#define __spporting_hpp__

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/resource.h>

#define sp_syslog      syslog
#define sp_openlog     openlog
#define sp_closelog    closelog

#define sp_inet_aton    inet_aton
#define sp_close        close
#define sp_writev       writev
#define sp_socketpair   socketpair
#define sp_gettimeofday gettimeofday

inline int sp_initsock()
{
	return 0;
}

#ifndef LOG_PERROR
#define LOG_PERROR  0
#endif

#endif