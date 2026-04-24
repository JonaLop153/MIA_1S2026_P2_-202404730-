#ifndef MKFS_H
#define MKFS_H

#include <string>
#include <map>

using namespace std;

class Mkfs {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
    bool formatearEXT2(const string& diskPath, int partStart, SuperBlock& sb);
    bool formatearEXT3(const string& diskPath, int partStart, SuperBlock& sb);
};

#endif