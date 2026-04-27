#include "loss.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <vector>

using namespace std;

map<string, string> Loss::parsearParametros(const string& comando) {
    map<string, string> params;
    istringstream iss(comando);
    string token;
    iss >> token;
    while (iss >> token) {
        if (token[0] == '-') {
            size_t eqPos = token.find('=');
            if (eqPos != string::npos) {
                string key = token.substr(1, eqPos - 1);
                string value = token.substr(eqPos + 1);
                if (!value.empty() && value.front() == '"' && value.back() == '"')
                    value = value.substr(1, value.length() - 2);
                for (char& c : key) c = tolower(c);
                params[key] = value;
            }
        }
    }
    return params;
}

bool Loss::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("id") == params.end()) { 
        error = "Error: -id obligatorio"; 
        return false; 
    }
    return true;
}

string Loss::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string id = params["id"];
    MountMap& montadas = getParticionesMontadas();
    
    if (montadas.find(id) == montadas.end()) {
        return "Error: No hay partición montada con ID: " + id;
    }
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    std::cerr << "DEBUG LOSS: Simulando pérdida en ID=" << id << std::endl;
    
    // ✅ Leer MBR y SuperBlock
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) return "Error: No se pudo abrir el disco";
    
    MBR mbr; file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR)); file.close();
    int partStart = mbr.mbr_partitions[indice].part_start;
    
    ifstream sbFile(diskPath, ios::binary | ios::in);
    sbFile.seekg(partStart, ios::beg);
    SuperBlock sb; sbFile.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock)); sbFile.close();
    
    // ✅ Verificar que sea EXT3
    if (sb.s_filesystem_type != 3) {
        return "Error: La partición no es EXT3 (tipo=" + to_string(sb.s_filesystem_type) + ")";
    }
    
    // ✅ Abrir para escritura
    fstream writeFile(diskPath, ios::binary | ios::in | ios::out);
    if (!writeFile.is_open()) {
        return "Error: No se pudo abrir el disco para escritura";
    }
    
    // ✅ Limpiar Bitmap de Inodos con \0
    int bitmapInoSize = (sb.s_inodes_count + 7) / 8;
    vector<char> bufferIno(bitmapInoSize, '\0');
    writeFile.seekp(partStart + sb.s_bm_inode_start, ios::beg);
    writeFile.write(bufferIno.data(), bitmapInoSize);
    std::cerr << "DEBUG LOSS: Bitmap Inodos limpiado (" << bitmapInoSize << " bytes)" << std::endl;
    
    // ✅ Limpiar Bitmap de Bloques con \0
    int bitmapBloSize = (sb.s_blocks_count + 7) / 8;
    vector<char> bufferBlo(bitmapBloSize, '\0');
    writeFile.seekp(partStart + sb.s_bm_block_start, ios::beg);
    writeFile.write(bufferBlo.data(), bitmapBloSize);
    std::cerr << "DEBUG LOSS: Bitmap Bloques limpiado (" << bitmapBloSize << " bytes)" << std::endl;
    
    // ✅ Limpiar Área de Inodos con \0
    int inodosSize = sb.s_inodes_count * sizeof(Inodo);
    vector<char> bufferInodos(inodosSize, '\0');
    writeFile.seekp(partStart + sb.s_inode_start, ios::beg);
    writeFile.write(bufferInodos.data(), inodosSize);
    std::cerr << "DEBUG LOSS: Área de Inodos limpiada (" << inodosSize << " bytes)" << std::endl;
    
    // ✅ Limpiar Área de Bloques con \0
    int bloquesSize = sb.s_blocks_count * sb.s_block_s;
    vector<char> bufferBloques(bloquesSize, '\0');
    writeFile.seekp(partStart + sb.s_block_start, ios::beg);
    writeFile.write(bufferBloques.data(), bloquesSize);
    std::cerr << "DEBUG LOSS: Área de Bloques limpiada (" << bloquesSize << " bytes)" << std::endl;
    
    writeFile.flush();
    writeFile.close();
    
    std::cerr << "DEBUG LOSS: Pérdida simulada exitosamente" << std::endl;
    
    return "Pérdida del sistema de archivos EXT3 simulada exitosamente en partición: " + id;
}