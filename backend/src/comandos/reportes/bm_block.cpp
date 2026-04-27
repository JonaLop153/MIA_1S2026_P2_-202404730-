#include "bm_block.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

string BmBlock::generarReporte(const string& diskPath, int partStart, const SuperBlock& sb) {
    std::cerr << "DEBUG BM_BLOCK: Generando reporte de bitmap de bloques" << std::endl;
    
    // Calcular tamaño del bitmap
    int bitmapSize = (sb.s_blocks_count + 7) / 8;
    
    // Leer bitmap
    vector<char> bitmap(bitmapSize);
    ifstream bmFile(diskPath, ios::binary | ios::in);
    bmFile.seekg(partStart + sb.s_bm_block_start, ios::beg);
    bmFile.read(bitmap.data(), bitmapSize);
    bmFile.close();
    
    // Generar contenido TXT
    ostringstream txt;
    txt << "Bitmap de Bloques - Partición\n";
    txt << "================================\n";
    txt << "Total bloques: " << sb.s_blocks_count << "\n";
    txt << "Bloques libres: " << sb.s_free_blocks_count << "\n";
    txt << "Bloques ocupados: " << (sb.s_blocks_count - sb.s_free_blocks_count) << "\n\n";
    
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