#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
	int MySocket, optval,realSend, ret = 0;
	struct sockaddr_in addr_in;
	char SendBuf[1024*4+1] = "";
	int Real_send = 0;
	MySocket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(MySocket == -1)
        {
	
			printf("Create Mysock fail!n");	
			return -1;
	}
	
	optval = 1;
	setsockopt(MySocket, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval) );
	
	
	memset(&addr_in, 0, sizeof(sockaddr_in));	
	addr_in.sin_family = AF_INET;
  addr_in.sin_addr.s_addr = inet_addr(argv[1]);
  addr_in.sin_port = htons(atoi(argv[2]));
  	
	//循环连接server
	while(true)
	{
		 ret = connect(MySocket, (const struct sockaddr *)&addr_in, sizeof(addr_in));
  	 if(ret == 0)
  	 {
  	 		break;	
  	 }	
		 else
		 {
		 		if(errno == EINTR)	
		 		{
		 			continue;	
		 		}
		 		else if(errno == EINTR)	
		 		{
		 			//usleep(1000);	
		 			continue;
		 		}
		 		else
		 		{
					perror("connect fail,error:");
		 			close(MySocket);	
		 			printf("connect server address->%s:%s fail!\n", argv[1], argv[2]);
		 			return -1;
		 		}		 	
		 }		
	}
	
	int flags = fcntl(MySocket, F_GETFL, 0);  
	if(flags == -1)
	{
	 		printf( "get socket flag fail, error: %s\n", strerror(errno));
	 		return -1;	 	
	}
	fcntl(MySocket, F_SETFL, flags | O_NONBLOCK);  
	
	//读写数据
	while(true)
	{
		cout<<"please input you want message"<<endl;
		cin>>SendBuf;
		
		if(strcmp(SendBuf,"q") ==0)
		{
				printf("client quit\n");
				close(MySocket);
				return 1;	
		}
		
		Real_send = send(MySocket, SendBuf, strlen(SendBuf), 0);
		if(Real_send == -1)
		{
			 /*
			 errno为EAGAINE或 EWOULDBLOCK,表示当前缓冲区写满,可以继续写
			 */
			 if(errno == EAGAIN)  
			 {
					 usleep(1000);  
		       continue;  						
			 }
			 							 
			 /*
			 errno为EINTR表示被中断,可以继续写
			 */
			 else if(errno == EINTR)
			 {
					 continue;
			 } 							 
			 
			 /*
			 即errno不为EAGAINE或EWOULDBLOCK或EINTR，则发送出错,此时应该close(sockfd)
			 */
			 else
			 {
			 		printf( "send failed: %s\n", strerror(errno));	
			 		close(MySocket);
			 		return -1;
			 }	 
		}
		else
		{
				printf("send dada:%s successful!\n");
	      continue; 
		}		
	}
	
	return 0;	
}
