#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#define  local_port                 12345
#define  local_ip                   "0.0.0.0"   
#define  MAX_EVENTS                 100   
#define  MAX_BUFFER                 1024

int main(int argc, char *argv[])
{
		int listenFd, clientFd = 0;
		int epfd = 0; 
	  int ret, cLen, rlen, wlen = 0;
	  char rbuf[MAX_BUFFER] = {0};
	  char wbuf[MAX_BUFFER] = {0};
	  
	  struct sockaddr_in local_addr, client_addr;
	  struct epoll_event myevents[MAX_EVENTS] = {0};
	  
	  memset(&client_addr, 0, sizeof(client_addr));
	  
	  listenFd = socket(AF_INET, SOCK_STREAM, 0);
	  if(listenFd == -1)
	  {
	  		perror("can't create stream socket!\n");
	  		return -1;	
	  }

    int addrval = 1;
    if(setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&addrval, sizeof(addrval)) == -1)
    {
	  		perror("setsockopt SO_REUSEADDR socket fail!\n");
	  		return -1;	
	  }       	
	  
	  
	  int flags = fcntl(listenFd, F_GETFL, 0);
	  if(fcntl(listenFd,  F_SETFL, flags|O_NONBLOCK) == -1)
	   {
	  		perror("set socket noblock fail!\n");
	  		return -1;	
	  }       
	  	  	  
	  local_addr.sin_family = AF_INET;
	  local_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr(local_ip);
	  local_addr.sin_port = htons(local_port);	  
	  
	  if(bind(listenFd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1)
    {
	  		perror("bind socket fail!\n");
	  		return -1;	
	  }       

		
	  if(listen(listenFd, 5) == -1)
	  {
	  		perror("listen socket fail!\n");
	  		return -1;	
	  }
	  
	  
    epfd = epoll_create(5);        
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listenFd;    
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenFd,  &ev) == -1)
    {
    		perror("epoll_ctl:: add socket fail!\n");
	  		return -1;	    	
    }
    
    
    //int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
    //int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
    
	  while(true)
	  {
	  		int evnum = epoll_wait(epfd, myevents, MAX_EVENTS, -1);
	  	
	  		for(int i= 0; i<evnum; i++)
	  		{
	  					
	  					if(myevents[i].data.fd == listenFd)
	  					{
	  							clientFd = accept(listenFd, (struct sockaddr*)&client_addr,(socklen_t *)&cLen);
	  							if(clientFd == -1)
	  							{
	  									if((errno == EAGAIN) || (errno == EINTR))
											{
													continue;	
											}
											else
											{
													perror("accept failed. Error");	
													
													epoll_ctl(epfd, EPOLL_CTL_DEL, listenFd, NULL);
													close(clientFd);
													
													return -1;
											}			  	      		  							
									}
									else
									{
											 struct epoll_event nev;
									     nev.events = EPOLLIN;
									     nev.data.fd = clientFd; 
									     if(epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd,  &nev) == -1)   
									     {
									     		perror("epoll_ctl:: add socket fail!\n");
									     		close(clientFd);
									     		return -1;		
									     }
									}
	  								
	  					}
	  					
	  					
	  					if(myevents[i].events == EPOLLIN)
	  					{
	  							memset(rbuf, 0, sizeof(rbuf));
	  							int rfd = myevents[i].data.fd;
	  							int rnum = read(rfd, (void*)rbuf, (size_t)rlen);	  			
	  							if(rnum == -1)
	  							{
	  									if(errno == EAGAIN || errno == EINTR)
	  										 continue;
	  									else
	  									{
	  										 perror("epoll_ctl:: read fail!\n");
	  										 epoll_ctl(epfd, EPOLL_CTL_DEL, rfd, NULL);
	  										 close(rfd);	  										 
	  									}	 
	  							}
	  							else
	  							{
	  								  printf("Server read->%s,len = %d", rbuf, rnum);
	  									memset(&ev, 0, sizeof(ev));
	  									ev.events = EPOLLOUT;
    									ev.data.fd = rfd;    
    									if(epoll_ctl(epfd, EPOLL_CTL_MOD, rfd,&ev) == -1)
									    {
									    		perror("epoll_ctl:: mod socket fail!\n");
										  		return -1;	    	
									    }
	  							}	  						
	  					}
	  			
	  					if(myevents[i].events == EPOLLOUT )
	  					{
	  							int wfd = myevents[i].data.fd;
	  							int wnum = write(wfd, (void*)rbuf, sizeof(rbuf));	  	
	  							if(wnum == -1)
	  							{
	  									if(errno == EAGAIN || errno == EINTR)
	  										 continue;
	  									else
	  									{
	  										 perror("epoll_ctl:: write fail!\n");
	  										 epoll_ctl(epfd, EPOLL_CTL_DEL, wfd, NULL);
	  										 close(wfd);	  										 
	  									}	 
	  							}
	  							else
	  							{
	  								  printf("Server write->%s,len = %d", rbuf,wnum);
	  									memset(&ev, 0, sizeof(ev));
	  									ev.events = EPOLLIN;
    									ev.data.fd = wfd;    
    									if(epoll_ctl(epfd, EPOLL_CTL_MOD, wfd,&ev) == -1)
									    {
									    		perror("epoll_ctl:: mod socket fail!\n");
										  		return -1;	    	
									    }	  									  								
	  							}	  						
	  					}
	  			
	  		}	 	  	
	  }
	
	
	  close(epfd);
		return 0;	
}
