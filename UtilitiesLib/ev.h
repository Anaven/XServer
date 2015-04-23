#ifndef __EV_H__
#define __EV_H__

#include <sys/epoll.h>

struct eventreq {
  	int    	er_type;
#define EV_FD 1    // file descriptor
  	int    	er_handle;
 	void*	er_data;
  	int     er_eventbits;
#define EV_RE  1
#define EV_WR  2
#define EV_EX  4
#define EV_RM  8
};

#endif /* __EV_H__ */
