/*
 * ReadConfig.cpp
 *
 *  Created on: 30.11.2012
 *      Author: halk
 */
#include <string.h>
#include <stdio.h>

#include "ReadConfig.hpp"

using std::vector;

char pathToConfig[200];

XMLNode* GetSubstitutionEntity(const char *configFileName,const char *serverAddr,const char *clientAddr)
{
	XMLNode xMainNode=XMLNode::openFileHelper(configFileName,"PMML");
	int entityCount=xMainNode.nChildNode("Entity");
	for(int i=0;i<entityCount;++i)
	{
		XMLNode *entityNode=new XMLNode(xMainNode.getChildNode("Entity",i));
		if(strcmp(entityNode->getChildNode("Server").getAttribute("addr"),serverAddr)==0)
			if(strcmp(entityNode->getChildNode("Client").getAttribute("addr"),clientAddr)==0)
				return entityNode;
	}
	return NULL;
}

vector<substitutionCouple>* GetSubstitutionList(const char *configFileName,const char *serverAddr,const char *clientAddr)
{
	XMLNode * entityNode=GetSubstitutionEntity(configFileName,serverAddr,clientAddr);
	vector<substitutionCouple> *substitutionList=new std::vector<substitutionCouple>;
	if(entityNode==NULL) return substitutionList;

	XMLNode filesNode=entityNode->getChildNode("Files");
	int substitutionCount=filesNode.nChildNode("substitution");
	for(int i=0;i<substitutionCount;++i)
	{
		substitutionCouple couple;
		couple.what=filesNode.getChildNode("substitution",i).getAttribute("what");
		couple.to=filesNode.getChildNode("substitution",i).getAttribute("to");
		substitutionList->push_back(couple);
	}
	return substitutionList;
}

void InitConfigPath(const char* path)
{
	sprintf(pathToConfig,path);
}


