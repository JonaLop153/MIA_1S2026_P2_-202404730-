#include "sb.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

string Sb::generarDOT(const SuperBlock& sb) {
    ostringstream dot;
    dot << "digraph SuperBlock {\n";
    dot << "  rankdir=TB;\n";
    dot << "  node [shape=record, fontname=\"Courier\"];\n";
    
    dot << "  sb [label=\"SuperBlock";
    dot << "|type=" << sb.s_filesystem_type << " (EXT" << sb.s_filesystem_type << ")";
    dot << "|inodes_count=" << sb.s_inodes_count;
    dot << "|blocks_count=" << sb.s_blocks_count;
    dot << "|free_inodes=" << sb.s_free_inodes_count;
    dot << "|free_blocks=" << sb.s_free_blocks_count;
    dot << "|inode_size=" << sb.s_inode_s;
    dot << "|block_size=" << sb.s_block_s;
    dot << "|bm_inode_start=" << sb.s_bm_inode_start;
    dot << "|bm_block_start=" << sb.s_bm_block_start;
    dot << "|inode_start=" << sb.s_inode_start;
    dot << "|block_start=" << sb.s_block_start;
    if (sb.s_filesystem_type == 3) {
        dot << "|journal_start=" << sb.s_journal_start;
        dot << "|journal_count=" << sb.s_journal_count;
    }
    dot << "\"];\n";
    
    dot << "}\n";
    return dot.str();
}

string Sb::generarReporte(const SuperBlock& sb) {
    std::cerr << "DEBUG SB: Generando reporte de SuperBlock" << std::endl;
    
    // Generar DOT
    string dotContent = generarDOT(sb);
    
    // Escribir y generar imagen
    string dotPath = "/tmp/reporte_sb.dot";
    string imgPath = "/tmp/reporte_sb.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << dotContent;
    dotFile.close();
    
    string cmd = "dot -Tjpg " + dotPath + " -o " + imgPath + " 2>/dev/null";
    system(cmd.c_str());
    
    std::cerr << "DEBUG SB: Reporte generado en " << imgPath << std::endl;
    
    return imgPath;
}