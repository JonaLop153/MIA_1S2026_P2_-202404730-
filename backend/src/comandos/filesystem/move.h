#ifndef MOVE_H
#define MOVE_H

#include <string>
#include <map>

using namespace std;

class Move {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
};

#endif