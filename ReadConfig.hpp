#ifndef READCONFIG_H_
#define READCONFIG_H_

#include <vector>
#include "xmlParser.h"

struct substitutionCouple
{
	const char *what;
	const char *to;
};

/*
 *	Функция разбирает xml-файл configFileName. И ищет в нем блок <Entity>...</Entity>, поля
 *	server addr=serverAddr и client addr=serverAddr.
 *	В случае успеха функция возвращает указатель на XMLNode, которая указывает на подходящий блок<Entity>...</Entity>
 *	иначе NULL
*/
XMLNode* GetSubstitutionEntity(const char *configFileName,const char *serverAddr,const char *clientAddr);

/*
 * Функция разбирает блок <Entity>...</Entity> xml-файла configFileName и заносит в массив имена файлов,
 * заменяемого и заменяющего.
 * В случае успеха-указатель на заполненный массив
 */
std::vector<substitutionCouple>* GetSubstitutionList(const char *configFileName,const char *serverAddr,const char *clientAddr);

/*
 * Функция инициализирует глобальныю переменную pathToConfig.
 */
void InitConfigPath(const char* path);

#endif /* READCONFIG_H_ */
