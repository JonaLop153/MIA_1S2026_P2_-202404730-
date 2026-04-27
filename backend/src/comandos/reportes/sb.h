#ifndef SB_H
#define SB_H

#include <string>
#include "../../structs.h"

using namespace std;

class Sb {
public:
    string generarReporte(const SuperBlock& sb);
    
private:
    string generarDOT(const SuperBlock& sb);
};

#endif