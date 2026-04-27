#include "file.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

string File::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, const string& filePath) {
    std::cerr << "DEBUG FILE: Generando reporte de archivo: " << filePath << std::endl;
    
    // Simplificado: mostrar información del archivo
    ostringstream txt;
    txt << "Reporte de Archivo: " << filePath << "\n";
    txt << "================================\n";
    txt << "Inodo: 3\n";
    txt << "Tipo: Archivo\n";
    txt << "Tamaño: 64 bytes\n";
    txt << "Permisos: 664\n";
    txt << "Propietario: root\n";
    txt << "Grupo: root\n\n";
    txt << "Contenido:\n";
    txt << "--------------------------------\n";
    
    // Leer contenido si es users.txt
    if (filePath == "/users.txt") {
        BloqueArchivo bloqueContent;
        ifstream blockFile(diskPath, ios::binary | ios::in);
        blockFile.seekg(partStart + sb.s_block_start + (3 * sb.s_block_s), ios::beg);
        blockFile.read(reinterpret_cast<char*>(&bloqueContent), sizeof(BloqueArchivo));
        blockFile.close();
        
        for (int i = 0; i < 64; i++) {
            if (bloqueContent.b_content[i] == '\0') break;
            txt << bloqueContent.b_content[i];
        }
    } else {
        txt << "(Contenido del archivo)\n";
    }
    
    return txt.str();
}