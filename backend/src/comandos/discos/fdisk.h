#ifndef FDISK_H
#define FDISK_H

#include <string>
#include <map>

using namespace std;

class Fdisk {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
    bool crearParticion(const string& diskPath, const map<string, string>& params);
    bool eliminarParticion(const string& diskPath, const map<string, string>& params);
    bool modificarParticion(const string& diskPath, const map<string, string>& params);
    int calcularEspacioLibre(const MBR& mbr, int& start, int& size);
};

#endif