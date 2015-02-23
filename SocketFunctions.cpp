/*
 * SocketFunctiom.cpp
 *
 *  Created on: 31.10.2012
 *      Author: halk
 */

#include "SocketFunctions.h"
#include "Util.h"

#define MY_PORT 7351
#define BUFFSIZE 1500
#define SERVER_PORT 80

int CreateListener(int port)
{
	int listener;
	struct sockaddr_in addr;

	//Создаём сокет на который у нас направлен REDIRECT
	// ****Во все обработчики ошибок добавить exit()****
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0)
	{
	     printf("socket for listening failed \n");
	     return -1;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listener, (struct sockaddr *)&addr,sizeof(addr))<0)
	{
		printf("bind did not work");
		return -1;
	}

	listen(listener,10);

	return listener;

}

int CreateSocketServer(int port,const char* ipAddrServer)
{
	int ServerSocket;
	struct sockaddr_in addr;

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(ServerSocket < 0)
	{
		printf("socket for server failed \n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ipAddrServer);
	if(connect(ServerSocket, (struct sockaddr *)&addr ,sizeof(addr))<0)
	{
		printf("connect to server failed \n");
		return -2;
	}

	return ServerSocket;
}

int SendDataToSocket(int sock,char *data,int len)
{
	if(len<=0) return 0;
	int bytesSend=0;
	while(bytesSend<len)
		bytesSend+=send(sock,data+bytesSend,len,0);
	return bytesSend;
}

void SendFile(const char *filename,int sock)
{
	char buf[BUFFSIZE];
	int fd=open64(filename,O_RDONLY,S_IREAD);
	int bytesRead;
	while(1)
	{
		if((bytesRead=read(fd,(void*)buf,(unsigned int)BUFFSIZE))==0) return;
		SendDataToSocket(sock,buf,bytesRead);
	}
	close(fd);
}

string GetDestinationIp(int sock)
{
	struct sockaddr_in addrServer;
	int size = sizeof(addrServer);
	getsockopt(sock, SOL_IP, 80, &addrServer,(socklen_t *) &size);
	return string(inet_ntoa(addrServer.sin_addr));
}
