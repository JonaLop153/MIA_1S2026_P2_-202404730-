#ifndef FILE_H
#define FILE_H

#include <string>
#include "../../structs.h"

using namespace std;

class File {
public:
    string generarReporte(const string& diskPath, int partStart, const SuperBlock& sb, const string& filePath);
};

#endif