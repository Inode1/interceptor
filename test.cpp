/*
 * test.cpp
 *
 *  Created on: 15.11.2012
 *      Author: halk
 */

#include <iostream>
#include <algorithm>

#include "Regular.h"
#include "SocketFunctions.h"
#include "Util.h"

#include "test.h"

using std::string;
using std::vector;

Test::Test(int socket) : _socket(socket)
{
}

bool Test::ReadData(std::map<std::string,std::string> &substitutionList)
{
	char buffer[BUFFSIZE];
	// вместо BUFFSIZE в функции recv может быть любое число. Проверено!
	int bytesRead = ::recv(_socket, buffer, BUFFSIZE, 0);
	if(bytesRead <= 0)
	{
		cout << "[CLOSE THREAD]" << endl;
		return false;
	}

	if(!gotFullHttpRequest(_cache,buffer,bytesRead)) return true;

	string index=NeedReplace(buffer,substitutionList);
	if(!index.empty())
	{
		cout << "[MAKE SUBSTITUTION]" << endl;
		FileSubstitution(substitutionList[index].c_str(), _socket);
		_cache.clear();
		return true;
	}
	Append(_buffer, buffer, _cache.size());
	_cache.clear();
	return true;
}

bool Test::ReadData()
{
	char buffer[BUFFSIZE];

	int bytesRead = ::recv(_socket, buffer, BUFFSIZE, 0);
	if(bytesRead <= 0)
	{
		cout << "[CLOSE THREAD]" << endl;
		return false;
	}
	Append(_buffer, buffer, bytesRead);

	return true;
}

void Test::SendCollectedDataTo(IDataSender &sender)
{
	char buffer[BUFFSIZE] = {};

	int bytesToSend = PullBufferFromDeque(_buffer, buffer, BUFFSIZE - 1);
	if (bytesToSend == 0)
		return;

	int sentBytes = sender.SendData(buffer, bytesToSend);
	if (sentBytes < 0)
		return;

	if(sentBytes < bytesToSend)
		InsertInTheBegin(_buffer, buffer + sentBytes, bytesToSend - sentBytes);
}

int Test::SendData(char *buffer, int bufferSize)
{
	return SendDataToSocket(_socket, buffer, bufferSize);
}
