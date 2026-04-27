#ifndef LOSS_H
#define LOSS_H

#include <string>
#include <map>

using namespace std;

class Loss {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
};

#endif