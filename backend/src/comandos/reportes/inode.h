#ifndef INODE_H
#define INODE_H

#include <string>
#include "../../structs.h"

using namespace std;

class Inode {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, int inodoNum);
    
private:
    string generarDOT(const Inodo& inodo, int inodoNum);
};

#endif