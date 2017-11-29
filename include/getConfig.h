#ifndef _GET_CONFIG_H_
#define _GET_CONFIG_H_

#include <string>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;

#define COMMENT_CHAR '#'

bool readConfig(const string & filename, map<string, string> & m);
void rrintConfig(const map<string, string> & m);


#endif //_GET_CONFIG_H_
