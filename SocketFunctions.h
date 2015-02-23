/*
 * SocketFunction.h
 *
 *  Created on: 31.10.2012
 *      Author: halk
 */

#ifndef SOCKETFUNCTION_H_
#define SOCKETFUNCTION_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

struct socketInfo
{
	int hSocket;
	std::string addrSocket;
};

//Функция создания слушаещего сокета
//Она возвращает дескриптор слушаещего сокета
//На вход подаётся номер порта, через который осуществляется прослушивание
int CreateListener(int port);

//Функция создания сокета сервера
//Возвращает дескриптор сокета сервера
//На вход подаётся порт сервера
//*** Надо ещё передавать IP-address, но функции плучения IP ещё нет в рабочем состоянии***
int CreateSocketServer(int port,const char* ipAddrServer);


/*
 * Получает по дескриптору сокета ip-адрес назначения
 */
std::string GetDestinationIp(int sock);

//Отослать данные на сокет.
//data - данные для посылки
//len - размер данных
//sock - сокет, куда отсылаем
int SendDataToSocket(int sock,char *data,int len);

// Отправляет файл клиенту
void SendFile(const char *filename,int sock);

/*
 * Отправляет файл на заданный сокет
 * Параметры функции:
 * 	fileToSubstitution - имя файла для отправки
 * 	sock - сокет, на который отправлять
 */
void FileSubstitution(const char *fileToSubstitution,int sock);

#endif /* SOCKETFUNCTION_H_ */
