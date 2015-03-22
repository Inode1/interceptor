#include "Util.h"
#include "SocketFunctions.h"

using namespace std;

void FillFDSet(int fd1,int fd2,fd_set * fdset)
{
	FD_ZERO(fdset);
	FD_SET(fd1,fdset);FD_SET(fd2,fdset);
}

ulong GetFileSize(const char *filename)
{
	FILE* fd=fopen64(filename,"r");
	cout<<"[FILE OPEN]"<<fd<<endl;
	fseek(fd,0,SEEK_END);
	ulong fsize=ftello64(fd);
	fclose(fd);
	cout<<"[FSIZE] "<<fsize<<endl;
	return fsize;
}


char* MakeHttpAnswer(const char *filenameToSubstitution)
{
	char* httpAnswer=(char*)malloc(200);
	char fsizeLong[16];
	sprintf(fsizeLong,"%lu", GetFileSize(filenameToSubstitution));
	sprintf(httpAnswer, "HTTP/1.1 200 OK\r\nnginx/1.4.6 (Ubuntu)\r\nContent-Type: application/octet-stream\r\nContent-length: ");
	strcat(httpAnswer,fsizeLong);
	strcat(httpAnswer,"\r\nConnection: keep-alive\r\n\r\n");
	return httpAnswer;
}

void FileSubstitution(const char *fileToSubstitution,int sock)
{
	char *ptr=MakeHttpAnswer(fileToSubstitution);
	SendDataToSocket(sock,ptr,strlen(ptr));
	SendFile(fileToSubstitution,sock);
}

void Append(deque<char> &charDeque, const char *buffer, int bufferSize)
{
	charDeque.insert(charDeque.end(), buffer, buffer + bufferSize);
}

int PullBufferFromDeque(deque<char> &charDeque, char *buffer, int count)
{
	deque<char>::iterator startPosition = charDeque.begin();
	int movedCount = std::min(count, (int)charDeque.size());
	deque<char>::iterator endPosition = startPosition + movedCount;
	copy(startPosition, endPosition, buffer);

	charDeque.erase(startPosition, endPosition);
	return movedCount;
}

void InsertInTheBegin(deque<char> &charDeque, const char *buffer, int bufferSize)
{
	charDeque.insert(charDeque.begin(), buffer, buffer + bufferSize);
}

int WaitDescriptors(int sock1,int sock2,fd_set *readset,fd_set *writeset)
{
	FillFDSet(sock1,sock2,readset);
	FillFDSet(sock1,sock2,writeset);
	int maxsockhandle=max(sock1,sock2)+1;
	timeval timeout={5,0};
	return select(maxsockhandle,readset,writeset,NULL,&timeout);
}

bool gotFullHttpRequest(std::vector<char> &cache,char *buffer,int bufsize)
{
	cache.insert(cache.end(),buffer,buffer+bufsize);
	bzero(buffer,BUFFSIZE);
	std::copy(cache.begin(),cache.end(),buffer);
	if(strstr(buffer,"\r\n\r\n")==NULL)
	{
		cout<<"Not enough data to analyze get-request "<<endl;
		return false;
	}
	return true;
}
