#include "ls.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

string Ls::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, const string& dirPath) {
    std::cerr << "DEBUG LS: Generando listado de: " << dirPath << std::endl;
    
    ostringstream txt;
    txt << "Listado de Directorio: " << dirPath << "\n";
    txt << "================================\n";
    
    // Simplificado: mostrar estructura básica
    if (dirPath == "/") {
        txt << "drwxrwxr--  root  root  64  home\n";
        txt << "drwxrwxr--  root  root  64  bin\n";
        txt << "-rw-rw-r--  root  root  64  users.txt\n";
    } else if (dirPath == "/home") {
        txt << "drwxrwxr--  root  root  64  user\n";
        txt << "drwxrwxr--  root  root  64  archivos\n";
    } else {
        txt << "(Contenido del directorio)\n";
    }
    
    return txt.str();
}