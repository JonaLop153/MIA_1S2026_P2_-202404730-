#include "mkusr.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <iostream>

namespace fs = std::filesystem;

map<string, string> MkUsr::parsearParametros(const string& comando) {
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

bool MkUsr::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("user") == params.end()) { error = "Error: -user obligatorio"; return false; }
    if (params.find("pass") == params.end()) { error = "Error: -pass obligatorio"; return false; }
    if (params.find("grp") == params.end()) { error = "Error: -grp obligatorio"; return false; }
    return true;
}

string MkUsr::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    if (getUsuarioActual() != "root") return "Error: Solo root puede crear usuarios";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string user = params["user"], pass = params["pass"], grp = params["grp"];
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
    
    // ✅ Validar usuario no exista
    istringstream checkIss(contenido);
    string checkLinea;
    while (getline(checkIss, checkLinea)) {
        while (!checkLinea.empty() && (checkLinea.back() == '\n' || checkLinea.back() == '\r' || checkLinea.back() == ' ')) checkLinea.pop_back();
        if (checkLinea.empty()) continue;
        size_t p1 = checkLinea.find(',');
        size_t p2 = (p1 != string::npos) ? checkLinea.find(',', p1 + 1) : string::npos;
        size_t p3 = (p2 != string::npos) ? checkLinea.find(',', p2 + 1) : string::npos;
        size_t p4 = (p3 != string::npos) ? checkLinea.find(',', p3 + 1) : string::npos;
        if (p1 != string::npos && p2 != string::npos && p3 != string::npos && p4 != string::npos) {
            string type = checkLinea.substr(p1 + 1, p2 - p1 - 1);
            if (type.find('U') != string::npos) {
                string username = checkLinea.substr(p3 + 1, p4 - p3 - 1);
                while (!username.empty() && (username.front() == ' ' || username.front() == '\t')) username.erase(0, 1);
                while (!username.empty() && (username.back() == ' ' || username.back() == '\t')) username.pop_back();
                if (username == user) return "Error: El usuario ya existe: " + user;
            }
        }
    }
    
    // ✅ Validar grupo exista
    bool grupoExiste = false;
    istringstream grpIss(contenido);
    string grpLinea;
    while (getline(grpIss, grpLinea)) {
        while (!grpLinea.empty() && (grpLinea.back() == '\n' || grpLinea.back() == '\r' || grpLinea.back() == ' ')) grpLinea.pop_back();
        if (grpLinea.empty()) continue;
        size_t p1 = grpLinea.find(',');
        size_t p2 = (p1 != string::npos) ? grpLinea.find(',', p1 + 1) : string::npos;
        if (p1 != string::npos && p2 != string::npos) {
            string type = grpLinea.substr(p1 + 1, p2 - p1 - 1);
            if (type.find('G') != string::npos) {
                string gnombre = grpLinea.substr(p2 + 1);
                while (!gnombre.empty() && (gnombre.front() == ' ' || gnombre.front() == '\t')) gnombre.erase(0, 1);
                while (!gnombre.empty() && (gnombre.back() == ' ' || gnombre.back() == '\t')) gnombre.pop_back();
                if (gnombre == grp) { grupoExiste = true; break; }
            }
        }
    }
    if (!grupoExiste) return "Error: El grupo no existe: " + grp;
    
    // ✅ Calcular nuevo UID
    int nuevoUID = 2;
    istringstream maxIss(contenido);
    string maxLinea;
    while (getline(maxIss, maxLinea)) {
        while (!maxLinea.empty() && (maxLinea.back() == '\n' || maxLinea.back() == '\r' || maxLinea.back() == ' ')) maxLinea.pop_back();
        if (maxLinea.empty()) continue;
        size_t p1 = maxLinea.find(',');
        if (p1 != string::npos) {
            try { int uid = stoi(maxLinea.substr(0, p1)); if (uid >= nuevoUID) nuevoUID = uid + 1; } catch (...) {}
        }
    }
    
    // ✅ Agregar nueva línea
    string nuevaLinea = to_string(nuevoUID) + ",U," + grp + "," + user + "," + pass + "\n";
    if (contenido.length() + nuevaLinea.length() > 63) return "Error: No hay espacio en users.txt para más usuarios";
    if (!contenido.empty() && contenido.back() != '\n') contenido += "\n";
    contenido += nuevaLinea;
    
    // ✅ Escribir EXACTAMENTE 64 bytes con fstream
    char bufferEscritura[64];
    memset(bufferEscritura, 0, 64);
    strncpy(bufferEscritura, contenido.c_str(), 63);
    
    // ✅ fstream con ios::in | ios::out | ios::binary
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
    
    return "Usuario creado exitosamente: " + user;
}