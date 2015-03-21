#include <boost/regex.hpp>

#include "Regular.h"
#include <iostream>

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
		string foundString = results[1];
		return foundString;
	}
	return string();
}

string NeedReplace(char *buff,const std::map<string,DataLoad::fullPackageData> &substitutionList)
{
	if(FindRegular(buff,"GET (/.*) HTTP").empty() || substitutionList.empty()) return string{};
	string stringWithFilename(FindRegular(buff,"GET (/.*) HTTP"));

	for (const auto a: substitutionList)
	{
	    std::size_t found = stringWithFilename.find(a.first);

	    if (found!=std::string::npos)
	       return a.first;
	}
	return string{};
}

