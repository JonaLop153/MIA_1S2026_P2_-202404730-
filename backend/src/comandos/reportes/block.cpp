#include "block.h"
#include "../../structs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

string Block::generarDOT(int bloqueNum, const string& tipo) {
    ostringstream dot;
    dot << "digraph Block {\n";
    dot << "  rankdir=TB;\n";
    dot << "  node [shape=record, fontname=\"Courier\"];\n";
    dot << "  block [label=\"Bloque #" << bloqueNum << " (" << tipo << ")";
    
    for (int i = 0; i < 8; i++) {
        dot << "|content[" << i << "]";
    }
    dot << "|...|content[63]\"];\n";
    dot << "}\n";
    return dot.str();
}

string Block::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, int bloqueNum) {
    std::cerr << "DEBUG BLOCK: Generando reporte de bloque " << bloqueNum << std::endl;
    
    // Leer contenido del bloque
    BloqueArchivo bloqueContent;
    ifstream blockFile(diskPath, ios::binary | ios::in);
    blockFile.seekg(partStart + sb.s_block_start + (bloqueNum * sb.s_block_s), ios::beg);
    blockFile.read(reinterpret_cast<char*>(&bloqueContent), sizeof(BloqueArchivo));
    blockFile.close();
    
    // Determinar tipo de bloque (simplificado)
    string tipo = "Archivo";
    
    // Generar DOT
    string dotContent = generarDOT(bloqueNum, tipo);
    
    // Escribir archivo DOT y generar imagen
    string dotPath = "/tmp/reporte_block.dot";
    string imgPath = "/tmp/reporte_block.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << dotContent;
    dotFile.close();
    
    // Ejecutar graphviz
    string cmd = "dot -Tjpg " + dotPath + " -o " + imgPath + " 2>/dev/null";
    system(cmd.c_str());
    
    std::cerr << "DEBUG BLOCK: Reporte generado en " << imgPath << std::endl;
    
    return imgPath;
}