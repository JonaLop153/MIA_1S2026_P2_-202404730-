#ifndef FIND_H
#define FIND_H

#include <string>
#include <map>

using namespace std;

class Find {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
};

#endif