#ifndef REP_H
#define REP_H

#include <string>
#include <map>
#include "../../structs.h"  //  Para SuperBlock

using namespace std;

class Rep {
public:
    string ejecutar(const string& comando);
    
private:
    map<string, string> parsearParametros(const string& comando);
    bool validarParametros(const map<string, string>& params, string& error);
    
    // ✅ Función auxiliar para generar cada tipo de reporte
    string generarReporteDisk(const string& path, const string& id);
    string generarReporteMBR(const string& path, const string& id);
    string generarReporteInode(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteBlock(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteSB(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteTree(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteBmInode(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteBmBlock(const string& diskPath, int partStart, const SuperBlock& sb, const string& path);
    string generarReporteFile(const string& diskPath, int partStart, const SuperBlock& sb, const string& path, const string& pathFileLs);
    string generarReporteLs(const string& diskPath, int partStart, const SuperBlock& sb, const string& path, const string& pathFileLs);
};

#endif