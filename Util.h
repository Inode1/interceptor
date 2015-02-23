#ifndef UTIL_H_
#define UTIL_H_

#include <sys/types.h>
#include <sys/io.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <deque>
#include <vector>

#define BUFFSIZE 1500

using namespace std;

//Создать fd_set из двух дескрипторов
void FillFDSet(int fd1,int fd2,fd_set *fdset);

// Ожидание двух дескрипторов на чтение или запись. Возвращаемое значение совпадает с возвращаемым значением функции select
int WaitDescriptors(int fd1,int fd2,fd_set *readset,fd_set *writeset);

// Формируется подставной http-запрос якобы от имени клиента с запросом нужного файла
char* MakeHttpAnswer(const char *filenameToSubstitution);

// Заполняет список элементами из массива. Элементы вставляются поочереди в конец списка
void Append(deque<char> &charDeque, const char *buffer, int bufferSize);

void InsertInTheBegin(deque<char> &charDeque, const char *buffer, int bufferSize);

//Вставляет элементы массива в начало списка. При этом, массив вставляется с конца
int PullBufferFromDeque(deque<char> &charDeque, char *buffer, int bufferSize);

//Заполняем массив элементами из списка. По возможности, заносив count элементов. Если не получилось,
//в эту переменную запишется рельно занесенное в массив количество элементов
int InsertDequePartToCharArray(deque<char> *charDeque,char* charArr,int *count);

bool gotFullHttpRequest(std::vector<char> &cache,char *buffer,int bufsize);

ulong GetFileSize(const char *filename);

#endif
