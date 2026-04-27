#ifndef BM_INODE_H
#define BM_INODE_H

#include <string>
#include "../../structs.h"

using namespace std;

class BmInode {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb);
};

#endif