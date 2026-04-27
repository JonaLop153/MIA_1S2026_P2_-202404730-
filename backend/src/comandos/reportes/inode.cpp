#include "inode.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

string Inode::generarDOT(const Inodo& inodo, int inodoNum) {
    ostringstream dot;
    dot << "digraph Inode {\n";
    dot << "  rankdir=TB;\n";
    dot << "  node [shape=record, fontname=\"Courier\"];\n";
    
    // Nodo principal del inodo
    dot << "  inode [label=\"Inodo #" << inodoNum;
    dot << "|uid=" << inodo.i_uid;
    dot << "|gid=" << inodo.i_gid;
    dot << "|size=" << inodo.i_s;
    dot << "|type=" << (inodo.i_type == '0' ? "Carpeta" : "Archivo");
    dot << "|perm=" << inodo.i_perm[0] << inodo.i_perm[1] << inodo.i_perm[2];
    dot << "\"];\n";
    
    // Bloques directos
    dot << "  subgraph cluster_blocks {\n";
    dot << "    label=\"Bloques Directos\";\n";
    for (int i = 0; i < 12 && inodo.i_block[i] != -1; i++) {
        dot << "    b" << i << " [label=\"Bloque " << inodo.i_block[i] << "\"];\n";
        dot << "    inode -> b" << i << ";\n";
    }
    dot << "  }\n";
    
    dot << "}\n";
    return dot.str();
}

string Inode::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, int inodoNum) {
    std::cerr << "DEBUG INODE: Generando reporte de inodo " << inodoNum << std::endl;
    
    // Leer inodo
    Inodo inodo;
    ifstream inodeFile(diskPath, ios::binary | ios::in);
    inodeFile.seekg(partStart + sb.s_inode_start + (inodoNum * sizeof(Inodo)), ios::beg);
    inodeFile.read(reinterpret_cast<char*>(&inodo), sizeof(Inodo));
    inodeFile.close();
    
    // Generar DOT
    string dotContent = generarDOT(inodo, inodoNum);
    
    // Escribir y generar imagen
    string dotPath = "/tmp/reporte_inode.dot";
    string imgPath = "/tmp/reporte_inode.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << dotContent;
    dotFile.close();
    
    string cmd = "dot -Tjpg " + dotPath + " -o " + imgPath + " 2>/dev/null";
    system(cmd.c_str());
    
    std::cerr << "DEBUG INODE: Reporte generado en " << imgPath << std::endl;
    
    return imgPath;
}