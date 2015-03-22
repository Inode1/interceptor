#ifndef TEST_H_
#define TEST_H_

#include <deque>
#include <map>

#define BUFFSIZE 1500

struct IDataSender
{
	virtual ~IDataSender() {}
	virtual int SendData(char *buffer, int bufferSize) = 0;
};

class Test : public IDataSender
{
private:
	std::deque<char> _buffer;
	std::vector<char> _cache;

public:
	int _socket;

	Test(int socket);
	//Вариант для серверной стороны, когда не надо проверять замену
	bool ReadData();
	//Вариант для клиентской стороны, когда надо проверять замену
	bool ReadData(std::map<std::string,std::string> &substitutionList);
	void SendCollectedDataTo(IDataSender &sender);

private:
	int SendData(char *buffer, int bufferSize);
};

#endif
