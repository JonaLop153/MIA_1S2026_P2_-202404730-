#include "login.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

map<string, string> Login::parsearParametros(const string& comando) {
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
                
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                
                for (char& c : key) c = tolower(c);
                params[key] = value;
            }
        }
    }
    return params;
}

bool Login::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("user") == params.end()) {
        error = "Error: El parámetro -user es obligatorio";
        return false;
    }
    if (params.find("pass") == params.end()) {
        error = "Error: El parámetro -pass es obligatorio";
        return false;
    }
    if (params.find("id") == params.end()) {
        error = "Error: El parámetro -id es obligatorio";
        return false;
    }
    return true;
}

bool Login::autenticarUsuario(const string& id, const string& user, const string& pass, string& diskPath) {
    std::cerr << "========== DEBUG LOGIN INICIO ==========" << std::endl;
    std::cerr << "DEBUG: user='" << user << "', pass='" << pass << "', id='" << id << "'" << std::endl;
    
    MountMap& montadas = getParticionesMontadas();
    
    if (montadas.find(id) == montadas.end()) {
        std::cerr << "DEBUG: ID no encontrado en particiones montadas" << std::endl;
        std::cerr << "========== DEBUG LOGIN FIN ==========" << std::endl;
        return false;
    }
    
    diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    std::cerr << "DEBUG: diskPath='" << diskPath << "', indice=" << indice << std::endl;
    
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) {
        std::cerr << "DEBUG: No se pudo abrir el disco" << std::endl;
        std::cerr << "========== DEBUG LOGIN FIN ==========" << std::endl;
        return false;
    }
    
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    int partStart = mbr.mbr_partitions[indice].part_start;
    
    std::cerr << "DEBUG: partStart=" << partStart << std::endl;
    
    file.seekg(partStart, ios::beg);
    SuperBlock sb;
    file.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    
    std::cerr << "DEBUG: sb.s_inode_start=" << sb.s_inode_start << std::endl;
    std::cerr << "DEBUG: sb.s_block_start=" << sb.s_block_start << std::endl;
    std::cerr << "DEBUG: sb.s_block_s=" << sb.s_block_s << std::endl;
    
    file.seekg(partStart + sb.s_inode_start + (3 * sizeof(Inodo)), ios::beg);
    Inodo inodoUsers;
    file.read(reinterpret_cast<char*>(&inodoUsers), sizeof(Inodo));
    
    int bloqueNum = inodoUsers.i_block[0];
    int bloqueOffset = partStart + sb.s_block_start + (bloqueNum * sb.s_block_s);
    
    std::cerr << "DEBUG: bloqueNum=" << bloqueNum << std::endl;
    std::cerr << "DEBUG: bloqueOffset=" << bloqueOffset << std::endl;
    
    file.seekg(bloqueOffset, ios::beg);
    char buffer[64];
    memset(buffer, 0, 64);
    file.read(buffer, 64);
    file.close();
    
    std::cerr << "DEBUG: buffer='" << buffer << "'" << std::endl;
    
    string contenido = "";
    for (int i = 0; i < 64; i++) {
        if (buffer[i] == '\0') break;
        contenido += buffer[i];
    }
    
    std::cerr << "DEBUG: contenido='" << contenido << "'" << std::endl;
    
    istringstream iss(contenido);
    string linea;
    
    bool encontrado = false;
    
    while (getline(iss, linea)) {
        while (!linea.empty() && (linea.back() == '\n' || linea.back() == '\r' || linea.back() == ' ')) {
            linea.pop_back();
        }
        if (linea.empty()) continue;
        
        std::cerr << "DEBUG: linea='" << linea << "'" << std::endl;
        
        istringstream lineStream(linea);
        string uidStr, type, grupo, username, password;
        
        getline(lineStream, uidStr, ',');
        getline(lineStream, type, ',');
        
        if (type.find('U') != string::npos && uidStr != "0") {
            getline(lineStream, grupo, ',');
            getline(lineStream, username, ',');
            getline(lineStream, password);
            
            while (!username.empty() && (username.front() == ' ' || username.front() == '\t')) username.erase(0, 1);
            while (!username.empty() && (username.back() == ' ' || username.back() == '\t' || username.back() == '\r' || username.back() == '\n')) username.pop_back();
            
            while (!password.empty() && (password.front() == ' ' || password.front() == '\t')) password.erase(0, 1);
            while (!password.empty() && (password.back() == ' ' || password.back() == '\t' || password.back() == '\r' || password.back() == '\n')) password.pop_back();
            
            std::cerr << "DEBUG: Comparando user='" << username << "' (buscando: '" << user << "')" << std::endl;
            std::cerr << "DEBUG: Comparando pass='" << password << "' (buscando: '" << pass << "')" << std::endl;
            
            if (username == user && password == pass) {
                std::cerr << "DEBUG: AUTENTICACIÓN EXITOSA" << std::endl;
                encontrado = true;
                break;
            }
        }
    }
    
    if (!encontrado) {
        std::cerr << "DEBUG: AUTENTICACIÓN FALLIDA - usuario no encontrado" << std::endl;
    }
    
    std::cerr << "========== DEBUG LOGIN FIN ==========" << std::endl;
    return encontrado;
}

string Login::ejecutar(const string& comando) {
    std::cerr << "========== DEBUG LOGIN EJECUTAR ==========" << std::endl;
    
    map<string, string> params = parsearParametros(comando);
    
    string error;
    if (!validarParametros(params, error)) {
        std::cerr << "DEBUG: Validación fallida: " << error << std::endl;
        std::cerr << "========== DEBUG LOGIN EJECUTAR FIN ==========" << std::endl;
        return error;
    }
    
    if (haySesionActiva()) {
        std::cerr << "DEBUG: Ya hay sesión activa" << std::endl;
        std::cerr << "========== DEBUG LOGIN EJECUTAR FIN ==========" << std::endl;
        return "Error: Ya hay una sesión activa. Debe hacer logout primero";
    }
    
    string user = params["user"];
    string pass = params["pass"];
    string id = params["id"];
    string diskPath;
    
    std::cerr << "DEBUG: Intentando autenticar user='" << user << "', id='" << id << "'" << std::endl;
    
    if (!autenticarUsuario(id, user, pass, diskPath)) {
        std::cerr << "DEBUG: Autenticación fallida" << std::endl;
        std::cerr << "========== DEBUG LOGIN EJECUTAR FIN ==========" << std::endl;
        return "Error: Autenticación fallida. Usuario o contraseña incorrectos";
    }
    
    iniciarSesion(user, pass, id, diskPath);
    
    std::cerr << "DEBUG: Sesión iniciada exitosamente" << std::endl;
    std::cerr << "========== DEBUG LOGIN EJECUTAR FIN ==========" << std::endl;
    
    ostringstream oss;
    oss << "Sesión iniciada exitosamente:\n";
    oss << "  Usuario: " << user << "\n";
    oss << "  ID Partición: " << id;
    
    return oss.str();
}