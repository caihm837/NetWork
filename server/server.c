
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define INADDR_ANY ((in_addr_t) 0x00000000)
#define HOST_PORT  8888

int main(int argc, char*argv[])
{
	char RecvBuf[1024*4+1] = "";
        char ipAddr[255]="";
	int hSocket,connfd,optval, peerLen, hostLen, flags = 0;
	
	socklen_t C_addr_len = 0;
	size_t Recv_len = 1024 * 4;
	size_t RetSend,RetRecv = 0;
	
	hSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(hSocket == -1)
	{
		perror("can't create stream socket!\n");
		return 1;
	}
	
	 flags = fcntl(hSocket, F_GETFL, 0);  
	 if(flags == -1)
	 {
	 		printf( "get socket flag fail, error: %s\n", strerror(errno));
	 		return -1;	 	
	 }
	 fcntl(hSocket, F_SETFL, flags | O_NONBLOCK);   
	
	optval = 1;
	setsockopt(hSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, sizeof(optval) );
	
	/*
	When a socket is created with socket(2), it exists in a name space (address family) but has no address assigned to it.  
	bind() assigns the address specified to by  addr to  the  socket  referred  to  by the file descriptor sockfd.  addrlen 
	specifies the size, in bytes, of the address structure pointed to by addr.
  */        
  struct sockaddr_in addr_in, client_addr, peerAddr, hostAddr;
  memset(&addr_in, 0, sizeof(sockaddr_in));
  memset(&peerAddr, 0, sizeof(sockaddr_in));
  
  addr_in.sin_family = AF_INET;
  addr_in.sin_addr.s_addr = INADDR_ANY; //inet_addr("45.78.56.12"); 
  addr_in.sin_port = htons(HOST_PORT);
           
	int ret = bind(hSocket, (struct sockaddr*)&addr_in, sizeof(addr_in));
	if(ret == -1)
	{
		  perror("bind failed. Error");
      return 1;
	}
	
	ret = listen(hSocket, 5);
	if(ret == -1)
	{
		  perror("listen failed. Error");
      return 1;
	}
	
	getsockname(hSocket, (struct sockaddr *)&hostAddr, (socklen_t*)&hostLen);//获取server本机的ip地址
  printf("server address = %s:%d\n", inet_ntoa(hostAddr.sin_addr), ntohs(hostAddr.sin_port));

	
	//接受客户端的连接请求
	memset(&client_addr, 0, sizeof(client_addr));
	while(true)
	{
		connfd = accept(hSocket, (struct sockaddr*)&client_addr, (socklen_t *)&C_addr_len);
		if(connfd == -1)
		{
				if((errno == EAGAIN) || (errno == EINTR))
				{
					usleep(1000);  
					continue;	
				}
				else
				{
					perror("accept failed. Error");	
					return -1;
				}			  	      
		}
		else
		{
				printf("connect establish!, client ip = %s, port=%d\n", inet_ntoa(client_addr.sin_addr),	ntohl(client_addr.sin_port));	
				break;
		}
	}
	
	//读取客户端发送的信息,并直接返回给客户端
	/*
	ssize_t recv(int sockfd, void *buf, size_t len, int flags);
	These  calls  return  the number of bytes received, or -1 if an error occurred.  
	The return value will be 0 when the peer has performed an orderly shutdown.
	*/
	while(true)
	{  
			memset(RecvBuf, 0, sizeof(RecvBuf));
			RetRecv = recv(connfd, (void*)RecvBuf, Recv_len, 0);
			/*
			表示对端已经关闭了这个链接
			*/
			if(RetRecv == 0)
			{
					printf("client close socket,exit!\n");
					close(connfd);
					break;
			}
			/*
			分情况判断返回错误
			*/
			else if(RetRecv == -1)
			{
					/*
				  errno为EAGAINE或EWOULDBLOCK, 表示暂时无数据可读，可以继续读 
				  原因：可能是多进程读同一个sockfd，可能一个进程读到数据，其他进程就读取不到数据        
					*/
					if(errno == EAGAIN)  
					{
							usleep(1000);  
              continue;  						
					}					
					
					/*
				  errno为 EINTR, 表示被中断了,可以继续读
					*/
					else if(errno == EINTR)  
					{
              continue;  						
					}															
					else
					{
							printf( "recv failed: %s\n", strerror(errno));	
							close(connfd);
							break;
					}
			}
			//接收正常
			else
			{
					RetSend = send(connfd, RecvBuf, RetRecv, 0);
					if(RetSend == -1)
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
							 		close(connfd);
							 }	 
					}
					else
					{
							/*
							>=0且不等于要求发送的长度，应该继续send，如果等于要求发送的长度，发送完毕,后续补充
							*/
							getpeername(connfd, (struct sockaddr *)&peerAddr, (socklen_t*)&peerLen);
							printf("recv msg:%s from address = %s:%d\n", RecvBuf, inet_ntop(AF_INET, &peerAddr.sin_addr, ipAddr, sizeof(ipAddr)), ntohs(peerAddr.sin_port));
					}
			}

	}
	
	close(hSocket);
	return 0;	
	
}
