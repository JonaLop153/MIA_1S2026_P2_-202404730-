#ifndef LS_H
#define LS_H

#include <string>
#include "../../structs.h"

using namespace std;

class Ls {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, const string& dirPath);
};

#endif