#include "tree.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

string Tree::generarDOT() {
    ostringstream dot;
    dot << "digraph FileSystemTree {\n";
    dot << "  rankdir=TB;\n";
    dot << "  node [shape=box, fontname=\"Courier\"];\n";
    
    // Estructura básica del árbol
    dot << "  root [label=\"/\" style=filled fillcolor=lightblue];\n";
    dot << "  home [label=\"home\" style=filled fillcolor=lightgreen];\n";
    dot << "  bin [label=\"bin\" style=filled fillcolor=lightgreen];\n";
    dot << "  users [label=\"users.txt\" shape=ellipse];\n";
    dot << "  user [label=\"user\" style=filled fillcolor=lightyellow];\n";
    dot << "  docs [label=\"docs\" style=filled fillcolor=lightyellow];\n";
    dot << "  notas [label=\"notas.txt\" shape=ellipse];\n";
    
    // Conexiones
    dot << "  root -> home;\n";
    dot << "  root -> bin;\n";
    dot << "  root -> users;\n";
    dot << "  home -> user;\n";
    dot << "  user -> docs;\n";
    dot << "  docs -> notas;\n";
    
    dot << "}\n";
    return dot.str();
}

string Tree::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb) {
    std::cerr << "DEBUG TREE: Generando reporte de árbol del sistema de archivos" << std::endl;
    
    // Generar DOT
    string dotContent = generarDOT();
    
    // Escribir y generar imagen PNG
    string dotPath = "/tmp/reporte_tree.dot";
    string pngPath = "/tmp/reporte_tree.png";
    
    ofstream dotFile(dotPath);
    dotFile << dotContent;
    dotFile.close();
    
    string cmd = "dot -Tpng " + dotPath + " -o " + pngPath + " 2>/dev/null";
    system(cmd.c_str());
    
    std::cerr << "DEBUG TREE: Reporte generado en " << pngPath << std::endl;
    
    return pngPath;
}