#ifndef BM_BLOCK_H
#define BM_BLOCK_H

#include <string>
#include "../../structs.h"

using namespace std;

class BmBlock {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb);
};

#endif