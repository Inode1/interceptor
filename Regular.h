/*
 * Regular.h
 *
 *  Created on: 22.11.2012
 *      Author: halk
 */

#ifndef REGULAR_H_
#define REGULAR_H_

#include <string>

#include "data_load.h"

std::string FindRegular(const char*  buff, const char* regular);

std::string NeedReplace(char *buff,const std::map<std::string,DataLoad::fullPackageData> &substitutionList);

#endif /* REGULAR_H_ */
