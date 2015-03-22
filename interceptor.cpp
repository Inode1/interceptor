/*
 * interceptor.cpp
 */
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include <vector>

#include "test.h"
#include "SocketFunctions.h"
#include "Util.h"
#include "data_load.h"

#define MY_PORT 7351
#define BUFFSIZE 1500
#define SERVER_PORT 80

static std::map<std::string,std::string> interceptAll;

void * HttpSubstitution(void * arg)
{
	socketInfo clientInfo=(*(socketInfo*)arg);
	socketInfo serverInfo=socketInfo();

	serverInfo.addrSocket=string(GetDestinationIp(clientInfo.hSocket));
	serverInfo.hSocket=CreateSocketServer(SERVER_PORT,serverInfo.addrSocket.c_str());
	cout << serverInfo.addrSocket << endl;
	if(serverInfo.hSocket<0)
	{
		close(clientInfo.hSocket);
		return 0;
	}

	Test client(clientInfo.hSocket);
	Test server(serverInfo.hSocket);

	fd_set *readset=new fd_set;
	fd_set *writeset=new fd_set;

	while(1)
	{
		if(WaitDescriptors(client._socket, server._socket, readset, writeset)==-1)
		{
			cout<<"[ERROR] WaitDescriptors:"<<errno;
			break;
		}
		if(FD_ISSET(server._socket,readset))
		{
		    if(!server.ReadData())
				break;
		}
		if(FD_ISSET(client._socket,writeset))
		{
		    server.SendCollectedDataTo(client);
		}
		if(FD_ISSET(client._socket,readset))
		{
		    if(!client.ReadData(interceptAll))
				break;
		}
		if(FD_ISSET(server._socket,writeset))
		{
		    client.SendCollectedDataTo(server);
		}
	}
	close(client._socket);
	close(server._socket);
/*	free(substitutionList);*/

	return 0;
}

using namespace std;
int main()
{
	int ThreadResult;
	int listenerSocket;


    Config conf;
    Config::data data = conf.ReadConfig("subst.xml");
    //map<string,DataLoad::fullPackageData> interceptData;
    //conf.GetInterceptPackage(data, interceptData);
/*    if (data.loadRepo)
    {
        DataLoad::Load();
    }*/
    // build repo
    //DataLoad intercept(interceptData);
/*    intercept.GiveWorkerJob();*/
    // Get all file that need to intercept
    conf.GetMapAllIntercept(data,interceptAll);

    cout << "__________________________________________________________________________" << endl;


	if((listenerSocket = CreateListener(MY_PORT))<0)
	{
		cout<<errno;
		return errno;
	}
	//В него поместим все созданные указатели на сокеты для того, чтобы при выходе из программы
	// освободить память.
	// С auto_ptz  не получилось,т.к. не знаю, как прикастовать к void*
	// Завершение программы будет через сигнал
	vector<int*> arrSock;
	while(1)
	{
		//Ждём подключения

		int *ClientSocket =(int*)malloc(sizeof(int*));
		struct sockaddr_in addrClient;
		int sizeClientAddr = sizeof(addrClient);
		*ClientSocket = accept(listenerSocket,(sockaddr*)&addrClient,(socklen_t*)&sizeClientAddr);
		if (*ClientSocket < 0)
		{
			cout << "Error" << endl;
			continue;
		}
		socketInfo *clientInfo=new socketInfo;
		*clientInfo={*ClientSocket,inet_ntoa(addrClient.sin_addr)};
		pthread_t ThreadConnect;
		arrSock.push_back(ClientSocket);
		cout<<arrSock.size()<<endl;
		//Создаём обсsлуживающий поток для каждого подключения
		cout<<"NEW THREAD"<<endl;
		HttpSubstitution(clientInfo);
		ThreadResult = pthread_create(&ThreadConnect, NULL, HttpSubstitution, clientInfo);
		if(ThreadResult != 0)
		{
			printf("Didn't create the thread for socket: %d", *ClientSocket);
			return -1;
		}
	}
	return 0;

}
