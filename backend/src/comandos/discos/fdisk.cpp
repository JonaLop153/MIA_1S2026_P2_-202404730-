#include "fdisk.h"
#include "../../structs.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>

using namespace std;

map<string, string> Fdisk::parsearParametros(const string& comando) {
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

bool Fdisk::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("path") == params.end()) { 
        error = "Error: -path obligatorio"; 
        return false; 
    }
    return true;
}

int Fdisk::calcularEspacioLibre(const MBR& mbr, int& start, int& size) {
    int espacioLibre = 0;
    int ultimoFin = sizeof(MBR);
    
    vector<pair<int, int>> particiones;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            particiones.push_back({mbr.mbr_partitions[i].part_start, mbr.mbr_partitions[i].part_size});
        }
    }
    
    sort(particiones.begin(), particiones.end());
    
    for (auto& p : particiones) {
        if (p.first > ultimoFin) {
            espacioLibre = max(espacioLibre, p.first - ultimoFin);
        }
        ultimoFin = p.first + p.second;
    }
    
    if (mbr.mbr_tamano > ultimoFin) {
        espacioLibre = max(espacioLibre, mbr.mbr_tamano - ultimoFin);
    }
    
    return espacioLibre;
}

bool Fdisk::crearParticion(const string& diskPath, const map<string, string>& params) {
    std::cerr << "DEBUG FDISK: Creando partición" << std::endl;
    
    // Leer MBR
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) return false;
    
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    file.close();
    
    // Validar tipo
    char type = 'P';
    if (params.count("type")) {
        string t = params.at("type");
        if (t == "P" || t == "E" || t == "L") type = t[0];
        else return false;
    }
    
    // Contar particiones existentes
    int primarias = 0, extendidas = 0;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            if (mbr.mbr_partitions[i].part_type == 'P') primarias++;
            if (mbr.mbr_partitions[i].part_type == 'E') extendidas++;
        }
    }
    
    if (type == 'P' && primarias + extendidas >= 4) return false;
    if (type == 'E' && extendidas >= 1) return false;
    
    // Calcular tamaño
    int size = 0;
    if (params.count("size")) {
        size = stoi(params.at("size"));
        if (params.count("unit")) {
            string unit = params.at("unit");
            if (unit == "b") size *= 1;
            else if (unit == "k") size *= 1024;
            else if (unit == "m" || unit == "M") size *= 1024 * 1024;
        } else {
            size *= 1024;  // Default KB
        }
    }
    
    // Validar espacio libre
    int start, espacioLibre;
    espacioLibre = calcularEspacioLibre(mbr, start, espacioLibre);
    
    if (size > espacioLibre) return false;
    
    // Buscar slot libre
    int slotLibre = -1;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '0') {
            slotLibre = i;
            break;
        }
    }
    
    if (slotLibre == -1) return false;
    
    // Calcular start
    int nuevoStart = sizeof(MBR);
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            int fin = mbr.mbr_partitions[i].part_start + mbr.mbr_partitions[i].part_size;
            if (fin > nuevoStart) nuevoStart = fin;
        }
    }
    
    // Crear partición
    mbr.mbr_partitions[slotLibre].part_status = '1';
    mbr.mbr_partitions[slotLibre].part_type = type;
    mbr.mbr_partitions[slotLibre].part_fit = params.count("fit") ? params.at("fit")[0] : 'W';
    mbr.mbr_partitions[slotLibre].part_start = nuevoStart;
    mbr.mbr_partitions[slotLibre].part_size = size;
    
    string name = params.count("name") ? params.at("name") : "Particion";
    strncpy(mbr.mbr_partitions[slotLibre].part_name, name.c_str(), 15);
    mbr.mbr_partitions[slotLibre].part_name[15] = '\0';
    mbr.mbr_partitions[slotLibre].part_correlative = 0;
    memset(mbr.mbr_partitions[slotLibre].part_id, 0, 4);
    
    // ✅ Escribir MBR con fstream y seekp/write/flush
    fstream outFile(diskPath, ios::binary | ios::in | ios::out);
    if (!outFile.is_open()) return false;
    
    outFile.seekp(0, ios::beg);  // ✅ seekp para escritura
    outFile.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));  // ✅ write
    outFile.flush();  // ✅ flush
    outFile.close();
    
    std::cerr << "DEBUG FDISK: Partición creada en slot " << slotLibre << std::endl;
    
    return true;
}

bool Fdisk::eliminarParticion(const string& diskPath, const map<string, string>& params) {
    std::cerr << "DEBUG FDISK: Eliminando partición" << std::endl;
    
    if (!params.count("name")) return false;
    
    string name = params.at("name");
    string deleteMode = params.count("delete") ? params.at("delete") : "fast";
    
    if (deleteMode != "fast" && deleteMode != "full") return false;
    
    // ✅ Abrir con fstream para lectura y escritura
    fstream file(diskPath, ios::binary | ios::in | ios::out);
    if (!file.is_open()) return false;
    
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    
    // Buscar partición por nombre
    int indice = -1;
    for (int i = 0; i < 4; i++) {
        if (mbr.mbr_partitions[i].part_status == '1') {
            if (string(mbr.mbr_partitions[i].part_name) == name) {
                indice = i;
                break;
            }
        }
    }
    
    if (indice == -1) {
        file.close();
        return false;
    }
    
    // Modo full: rellenar con \0
    if (deleteMode == "full") {
        int start = mbr.mbr_partitions[indice].part_start;
        int size = mbr.mbr_partitions[indice].part_size;
        
        vector<char> buffer(size, '\0');
        file.seekp(start, ios::beg);  // ✅ seekp
        file.write(buffer.data(), size);  // ✅ write
        file.flush();  // ✅ flush
    }
    
    // Marcar como vacía
    mbr.mbr_partitions[indice].part_status = '0';
    mbr.mbr_partitions[indice].part_correlative = 0;
    memset(mbr.mbr_partitions[indice].part_name, 0, 16);
    
    // ✅ Escribir MBR actualizado
    file.seekp(0, ios::beg);  // ✅ seekp
    file.write(reinterpret_cast<char*>(&mbr), sizeof(MBR));  // ✅ write
    file.flush();  // ✅ flush
    file.close();
    
    std::cerr << "DEBUG FDISK: Partición eliminada exitosamente" << std::endl;
    
    return true;
}

bool Fdisk::modificarParticion(const string& diskPath, const map<string, string>& params) {
    std::cerr << "DEBUG FDISK: Modificando partición (add)" << std::endl;
    // Implementación simplificada para add
    return false;
}

string Fdisk::ejecutar(const string& comando) {
    std::cerr << "DEBUG FDISK: Ejecutando comando" << std::endl;
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string path = params["path"];
    
    if (params.count("delete")) {
        if (eliminarParticion(path, params)) {
            return "Partición eliminada exitosamente";
        } else {
            return "Error: No se pudo eliminar la partición. Verifique espacio disponible o restricciones.";
        }
    }
    
    if (params.count("add")) {
        if (modificarParticion(path, params)) {
            return "Partición modificada exitosamente";
        } else {
            return "Error: No se pudo modificar la partición";
        }
    }
    
    if (crearParticion(path, params)) {
        return "Partición creada exitosamente";
    } else {
        return "Error: No se pudo crear la partición. Verifique espacio disponible o restricciones.";
    }
}