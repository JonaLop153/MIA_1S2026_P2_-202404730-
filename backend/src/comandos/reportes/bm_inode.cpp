#include "bm_inode.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

string BmInode::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb) {
    std::cerr << "DEBUG BM_INODE: Generando reporte de bitmap de inodos" << std::endl;
    
    // Calcular tamaño del bitmap
    int bitmapSize = (sb.s_inodes_count + 7) / 8;
    
    // Leer bitmap
    vector<char> bitmap(bitmapSize);
    ifstream bmFile(diskPath, ios::binary | ios::in);
    bmFile.seekg(partStart + sb.s_bm_inode_start, ios::beg);
    bmFile.read(bitmap.data(), bitmapSize);
    bmFile.close();
    
    // Generar contenido TXT
    ostringstream txt;
    txt << "Bitmap de Inodos - Partición\n";
    txt << "================================\n";
    txt << "Total inodos: " << sb.s_inodes_count << "\n";
    txt << "Inodos libres: " << sb.s_free_inodes_count << "\n";
    txt << "Inodos ocupados: " << (sb.s_inodes_count - sb.s_free_inodes_count) << "\n\n";
    
    // Mostrar en formato binario por filas de 8 bytes
    for (int i = 0; i < bitmapSize && i < 64; i++) {
        txt << "Byte " << i << ": ";
        for (int bit = 7; bit >= 0; bit--) {
            txt << ((bitmap[i] >> bit) & 1);
        }
        txt << "\n";
    }
    if (bitmapSize > 64) {
        txt << "... (" << (bitmapSize - 64) << " bytes más)\n";
    }
    
    return txt.str();
}