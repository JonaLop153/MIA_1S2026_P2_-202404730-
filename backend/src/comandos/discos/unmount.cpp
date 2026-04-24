#include "unmount.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

map<string, string> Unmount::parsearParametros(const string& comando) {
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

bool Unmount::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("id") == params.end()) { 
        error = "Error: -id obligatorio"; 
        return false; 
    }
    return true;
}

string Unmount::ejecutar(const string& comando) {
    std::cerr << "DEBUG UNMOUNT: Iniciando comando" << std::endl;
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string id = params["id"];
    MountMap& montadas = getParticionesMontadas();
    
    if (montadas.find(id) == montadas.end()) {
        std::cerr << "DEBUG UNMOUNT: ID no encontrado: " << id << std::endl;
        return "Error: No existe una partición montada con ID: " + id;
    }
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    std::cerr << "DEBUG UNMOUNT: diskPath=" << diskPath << ", indice=" << indice << std::endl;
    
    // ✅ Leer MBR
    ifstream file(diskPath, ios::binary | ios::in | ios::out);
    if (!file.is_open()) {
        std::cerr << "DEBUG UNMOUNT: No se pudo abrir el disco" << std::endl;
        return "Error: No se pudo abrir el disco";
    }
    
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    
    // ✅ Actualizar partición: part_status = '0', part_correlative = 0
    mbr.mbr_partitions[indice].part_status = '0';
    mbr.mbr_partitions[indice].part_correlative = 0;
    
    // ✅ Escribir MBR actualizado
    file.seekp(0, ios::beg);
    file.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    file.flush();
    file.close();
    
    std::cerr << "DEBUG UNMOUNT: MBR actualizado exitosamente" << std::endl;
    
    // ✅ Eliminar de particiones montadas
    montadas.erase(id);
    
    std::cerr << "DEBUG UNMOUNT: Partición desmontada exitosamente" << std::endl;
    
    return "Partición desmontada exitosamente: " + id;
}