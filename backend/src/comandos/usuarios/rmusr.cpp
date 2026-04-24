#include "rmusr.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <iostream>

namespace fs = std::filesystem;

map<string, string> RmUsr::parsearParametros(const string& comando) {
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

bool RmUsr::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("user") == params.end()) { error = "Error: -user obligatorio"; return false; }
    return true;
}

string RmUsr::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    if (getUsuarioActual() != "root") return "Error: Solo root puede eliminar usuarios";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string user = params["user"];
    string id = getIdParticionActual();
    MountMap& montadas = getParticionesMontadas();
    if (montadas.find(id) == montadas.end()) return "Error: Partición no montada";
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    // ✅ Leer MBR y SuperBlock
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) return "Error: No se pudo abrir el disco";
    MBR mbr; file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR)); file.close();
    int partStart = mbr.mbr_partitions[indice].part_start;
    
    ifstream sbFile(diskPath, ios::binary | ios::in);
    sbFile.seekg(partStart, ios::beg);
    SuperBlock sb; sbFile.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock)); sbFile.close();
    
    // ✅ Calcular offset del bloque 3
    int bloqueOffset = partStart + sb.s_block_start + (3 * sb.s_block_s);
    
    // ✅ Leer contenido actual (64 bytes exactos)
    char bufferLectura[64];
    ifstream readBlock(diskPath, ios::binary | ios::in);
    readBlock.seekg(bloqueOffset, ios::beg);
    readBlock.read(bufferLectura, 64);
    readBlock.close();
    
    string contenido = "";
    for (int i = 0; i < 64; i++) {
        if (bufferLectura[i] == '\0') break;
        contenido += bufferLectura[i];
    }
    
    // ✅ Buscar usuario y marcar como eliminado
    bool usuarioEncontrado = false;
    string nuevoContenido = "";
    istringstream iss(contenido);
    string linea;
    while (getline(iss, linea)) {
        while (!linea.empty() && (linea.back() == '\n' || linea.back() == '\r' || linea.back() == ' ')) linea.pop_back();
        if (linea.empty()) continue;
        size_t p1 = linea.find(',');
        size_t p2 = (p1 != string::npos) ? linea.find(',', p1 + 1) : string::npos;
        size_t p3 = (p2 != string::npos) ? linea.find(',', p2 + 1) : string::npos;
        size_t p4 = (p3 != string::npos) ? linea.find(',', p3 + 1) : string::npos;
        if (p1 != string::npos && p2 != string::npos && p3 != string::npos && p4 != string::npos) {
            string type = linea.substr(p1 + 1, p2 - p1 - 1);
            if (type.find('U') != string::npos) {
                string username = linea.substr(p3 + 1, p4 - p3 - 1);
                while (!username.empty() && (username.front() == ' ' || username.front() == '\t')) username.erase(0, 1);
                while (!username.empty() && (username.back() == ' ' || username.back() == '\t')) username.pop_back();
                if (username == user) {
                    usuarioEncontrado = true;
                    string uidStr = linea.substr(0, p1);
                    string grupo = linea.substr(p2 + 1, p3 - p2 - 1);
                    string pass = linea.substr(p4 + 1);
                    string nuevaLinea = "0," + type + "," + grupo + "," + username + "," + pass + "\n";
                    nuevoContenido += nuevaLinea;
                    continue;
                }
            }
        }
        nuevoContenido += linea + "\n";
    }
    if (!usuarioEncontrado) return "Error: Usuario no encontrado: " + user;
    
    // ✅ Escribir EXACTAMENTE 64 bytes con fstream (NO ofstream)
    char bufferEscritura[64];
    memset(bufferEscritura, 0, 64);
    if (nuevoContenido.size() > 63) nuevoContenido = nuevoContenido.substr(0, 63);
    strncpy(bufferEscritura, nuevoContenido.c_str(), 63);
    
    // ✅ fstream con ios::in | ios::out | ios::binary (NO solo ios::out)
    fstream writeBlock(diskPath, ios::binary | ios::in | ios::out);
    if (!writeBlock.is_open()) return "Error: No se pudo abrir el disco para escritura";
    
    // ✅ Seek INMEDIATO después de abrir
    writeBlock.seekp(bloqueOffset, ios::beg);
    
    // ✅ Verificar posición ANTES de escribir
    if (writeBlock.tellp() != bloqueOffset) {
        writeBlock.close();
        return "Error: No se pudo posicionar en el bloque";
    }
    
    // ✅ Escribir EXACTAMENTE 64 bytes
    writeBlock.write(bufferEscritura, 64);
    writeBlock.flush();
    writeBlock.close();
    
    return "Usuario eliminado exitosamente: " + user;
}