/*
 * Regular.h
 *
 *  Created on: 22.11.2012
 *      Author: halk
 */

#ifndef REGULAR_H_
#define REGULAR_H_

#include <string>
#include "ReadConfig.hpp"

/*
 * Функция для поиска регулярного выражения в буфере.
 * Параметры функции:
 * 	buffer - буффер, в котором ищется регулярное выражение
 * 	regular - регулярное выражение
 * Возвращаемое значение:
 * 	строка, содержащая регулярное выражение - в случае успеха
 *  NULL иначе
 */
std::string FindRegular(const char*  buff, const char* regular);

/*
 * Функия получает имя файла из первой строки GET-запроса
 * Параметры функции:
 *  первая строка GET-запроса
 * Возвращаемое значение:
 * 	имя файла - в случае успеха
 *  NULL иначе
 */
std::string GetFileNameFromString(std::string stringWithFilename);

/*
 * Функция получает буффер данных (часть HTTP-запроса) и список файлов для замены. Сравнивает запрошенный файл с теми,
 * которые надо заменить.
 * Возвращаемое значение:
 * 	индекс записи в списке замен - в случае успеха
 *  -1 иначе
 */
int NeedReplace(char *buff,std::vector<substitutionCouple> substitutionList);

/*
 * Функция ищет имя файла в списке замен
 * Возвращаемое значение:
 *	 индекс записи в списке замен - в случае успеха
 *  -1 иначе
 */
int FindFilenameInList(std::string requestedFilename,std::vector<substitutionCouple> substitutionList);


#endif /* REGULAR_H_ */
