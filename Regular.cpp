#include <boost/regex.hpp>

#include "Regular.h"
#include "ReadConfig.hpp"

using std::string;
using std::vector;


string FindRegular(const char*  buff, const char* regular)
{
	string buffer(buff);
	string reg(regular);
	boost::smatch results;
	boost::regex exp(reg);
	if (boost::regex_search(buffer, results, exp))
	{
		//Возвращаем первое нахождение регулярки
		string foundString = results[0];
		return foundString;
	}
	return string();
}


string GetFileNameFromString(string stringWithFilename)
{
	uint pos=stringWithFilename.rfind(' ');
	if(pos==string::npos) return string();
	string filename=stringWithFilename.substr(0,pos);
	pos=filename.rfind('/');
	if(pos==string::npos) return string();
	filename=filename.substr(pos+1,filename.size()-pos);
	return filename;
}

int NeedReplace(char *buff,vector<substitutionCouple> substitutionList)
{
	if(FindRegular(buff,"GET /.* HTTP").empty() || substitutionList.empty()) return -1;
	string stringWithFilename(FindRegular(buff,"GET /.* HTTP"));
	if(GetFileNameFromString(stringWithFilename).empty()) return -1;
	string requestedFilename(GetFileNameFromString(stringWithFilename));
	return FindFilenameInList(requestedFilename,substitutionList);
}

int FindFilenameInList(string requestedFilename,vector<substitutionCouple> substitutionList)
{
	for(uint i=0;i<substitutionList.size();++i)
	{
		if(!FindRegular(requestedFilename.c_str(),substitutionList[i].what).empty())
			return i;
	}
	return -1;
}
