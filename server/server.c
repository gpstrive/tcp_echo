#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sched.h>
#include<errno.h>

#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include "poll.h"
#include "debug.h"
#include "request.h"
#define BUFFSIZE 1024

#define CPUS = 
char buf[BUFFSIZE]={0};

void read_cb (poll_event_t * poll_event, poll_event_element_t * node, struct epoll_event ev)
{
    // NOTE -> read is also invoked on accept and connect
    //INFO("in read_cb");
    // we just read data and print
    memset(buf, 0, BUFFSIZE);
    int val = read(node->fd, buf, BUFFSIZE);
    if (val>0)
    {
        // if we actually get data print it
       //buf[val] = '\0';
       //LOG(" received data -> %s %t\n", buf,clock());
		 int sent=write(node->fd, buf, strlen(buf));
		 if(sent==-1)
			 INFO("sent error");
         /**
       struct request *req=request_new();
       if(req)
       {
	  req->data=strdup(buf);
       	  parse_request(req);
	  if(req->method)
	  LOG("method:%s",req->method);
	  LOG("length:%d",req->length);
	  if(req->body){
	 	 LOG("body:%s",req->body);
		 int sent=write(node->fd,req->body,strlen(req->body));
		 if(sent==-1)
			 INFO("sent error");
		 else
			 LOG("sent:%d",sent);
	  }
	  free_request(req);
       }
       */
       
    }
}


void close_cb (poll_event_t * poll_event, poll_event_element_t * node, struct epoll_event ev)
{
    INFO("in close_cb");
    // close the socket, we are done with it
    poll_event_remove(poll_event, node->fd);
}

void accept_cb(poll_event_t * poll_event, poll_event_element_t * node, struct epoll_event ev)
{
    INFO("in accept_cb");
    // accept the connection 
    struct sockaddr_in clt_addr;
    socklen_t clt_len = sizeof(clt_addr);
    int listenfd = accept(node->fd, (struct sockaddr*) &clt_addr, &clt_len);
    if (listenfd < 0) {
        fprintf(stderr, "accept error %s \n", strerror(errno));
        return;
    }
    //fprintf(stderr, "got the socket %d on %d\n", listenfd, getpid());
    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    // set flags to check 
    uint32_t flags = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
    poll_event_element_t *p;
    // add file descriptor to poll event
    poll_event_add(poll_event, listenfd, flags, &p);
    // set function callbacks 
    p->read_callback = read_cb;
    p->close_callback = close_cb;
}

//time out function 
int timeout_cb (poll_event_t *poll_event)
{
    // just keep a count
    if (!poll_event->data)
    {
        // no count initialised, then initialize it
        INFO("init timeout counter");
        poll_event->data=calloc(1,sizeof(int));
    }
    else
    {
        // increment and print the count
        int * value = (int*)poll_event->data;
        *value+=1;
        //LOG("time out number %d", *value);
       // printf("tick (%d)\n", *value);
    }
    return 0;
}

int processFork(poll_event_t *pe)
{
    cpu_set_t mask;
    int i = 0;
    pid_t pid = 0;
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1)
    {
        fprintf(stderr, "Could not determine number of CPUs online:\n%s\n", 
                    strerror (errno));
        exit (-1);
    }
    for (i = 0 ; i < nprocs; i ++) {
        CPU_ZERO(&mask);
        CPU_SET(i, &mask);
        if (fork() == 0) {
            pid = getpid();
            // start the event loop
            if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
                perror("sched_setaffinity");
            }
            use_the_force(pe);
        }
    }
}

int main()
{
    //SIGPIPE handle by kernel	
    struct sigaction sa;
    sa.sa_handler=SIG_IGN;
    sigaction(SIGPIPE,&sa,0);

    // create a TCP socket, bind and listen
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0 , sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htons(INADDR_ANY);
    svr_addr.sin_port = htons(8080);
    bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr));
    listen(sock, 10);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    // create a poll event object, with time out of 1 sec
    poll_event_t *pe = poll_event_new(1000);
    // set timeout callback
    pe->timeout_callback = timeout_cb;
    poll_event_element_t *p;
    // add sock to poll event
    poll_event_add(pe, sock, EPOLLIN, &p);
    // set callbacks
    //p->read_callback = read_cb;
    p->accept_callback = accept_cb;
    p->close_callback = close_cb;
    // enable accept callback
    p->cb_flags |= ACCEPT_CB;
    processFork(pe);

    return 0;
}

