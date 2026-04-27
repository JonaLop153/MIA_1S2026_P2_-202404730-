#ifndef TREE_H
#define TREE_H

#include <string>
#include "../../structs.h"

using namespace std;

class Tree {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb);
    
private:
    string generarDOT();
};

#endif