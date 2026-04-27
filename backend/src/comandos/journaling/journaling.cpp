#include "journaling.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <ctime>
#include <vector>

using namespace std;

map<string, string> Journaling::parsearParametros(const string& comando) {
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

bool Journaling::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("id") == params.end()) { 
        error = "Error: -id obligatorio"; 
        return false; 
    }
    return true;
}

string Journaling::ejecutar(const string& comando) {
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
    
    std::cerr << "DEBUG JOURNALING: Leyendo journal de ID=" << id << std::endl;
    
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
    
    // ✅ Leer journal (50 entries)
    int journalSize = 50 * sizeof(JournalEntry);
    vector<JournalEntry> entries(50);
    
    ifstream journalFile(diskPath, ios::binary | ios::in);
    journalFile.seekg(partStart + sb.s_journal_start, ios::beg);
    journalFile.read(reinterpret_cast<char*>(entries.data()), journalSize);
    journalFile.close();
    
    // ✅ Mostrar entries
    ostringstream oss;
    oss << "Journal de la partición " << id << ":\n";
    oss << "========================================\n";
    
    int entriesCount = 0;
    for (int i = 0; i < 50; i++) {
        if (entries[i].operacion[0] != '\0') {
            entriesCount++;
            oss << "Entry #" << i << ":\n";
            oss << "  Operación: " << entries[i].operacion << "\n";
            oss << "  Ruta: " << entries[i].ruta << "\n";
            oss << "  Contenido: " << entries[i].contenido << "\n";
            
            time_t fecha = entries[i].fecha;
            char fechaStr[100];
            strftime(fechaStr, sizeof(fechaStr), "%Y-%m-%d %H:%M:%S", localtime(&fecha));
            oss << "  Fecha: " << fechaStr << "\n";
            oss << "  Usuario ID: " << entries[i].usuario_id << "\n";
            oss << "----------------------------------------\n";
        }
    }
    
    if (entriesCount == 0) {
        oss << "No hay entradas en el journal.\n";
    }
    
    oss << "========================================\n";
    oss << "Total de entradas: " << entriesCount << "/50\n";
    
    std::cerr << "DEBUG JOURNALING: Journal leído exitosamente (" << entriesCount << " entries)" << std::endl;
    
    return oss.str();
}