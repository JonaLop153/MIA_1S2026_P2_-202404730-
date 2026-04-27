#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include "../../structs.h"  // ✅ ESTO FALTABA - Define SuperBlock

using namespace std;

class Block {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, int bloqueNum);
    
private:
    string generarDOT(int bloqueNum, const string& tipo);
};

#endif