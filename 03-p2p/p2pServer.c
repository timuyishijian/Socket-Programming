/*
单进程服务器
实现简单点到点的聊天功能
*/
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"common.h"

int main()
{
	int listenfd;
	listenfd = socket(PF_INET , SOCK_STREAM, 0);
	if(listenfd <0)
	{
		err_exit("socket");
	}

	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(5888);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//设置地址可重复利用
	int on = 1;
	setsockopt(listenfd, SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));	

	int ret = bind(listenfd , (const struct sockaddr *)&addr,sizeof(addr));
	if(ret == -1)
	{
		err_exit("bind");
	}
	
	ret = listen(listenfd , SOMAXCONN);
	if(ret < 0)
	{
		err_exit("listen");
	}

	struct sockaddr_in peerAddr ;
	socklen_t peerlen = sizeof(peerAddr);//peerlen一定要有初始值
	
	int conn;
	conn = accept (listenfd ,(struct sockaddr *)&peerAddr,&peerlen);
	if(conn <0)
	{
		err_exit("accept");
	}
	
	printf("客户端的IP地址是：%s,端口号是：%d\n",
		inet_ntoa(peerAddr.sin_addr),ntohs(peerAddr.sin_port));
	
	pid_t pid ;
	pid = fork();
	//需要两个进程，一个负责读取键盘输入
	//另一个进程负责读取	
	
	char writebuf[512];
	char readbuf[512];
	if(pid == 0) //子进程复制从键盘读取
	{
		while(1)
		{
	
			memset(writebuf,0,sizeof(writebuf));
			fgets(writebuf,sizeof(writebuf),stdin);
			write(conn,writebuf,sizeof(writebuf));
		}			
	}
	else if(pid>0)	//父进程负责接收 
	{
		while(1)
		{
		
			memset(readbuf,0,sizeof(readbuf));
			int readBytes = read(conn,readbuf,sizeof(readbuf));
			if(readBytes == 0)
			{
				printf("client close\n");
				break;
			}
			if(readBytes<0)
			{
				err_exit("read");
			}
			fputs(readbuf,stdout);
		}		
		close(conn);
		exit(EXIT_SUCCESS);
	}
	//定义一个应用层的缓冲区，用于接收数据
	close(listenfd);
	close(conn);
	return 0;
}

