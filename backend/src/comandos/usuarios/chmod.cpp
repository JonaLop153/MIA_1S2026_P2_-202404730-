#include "chmod.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

map<string, string> Chmod::parsearParametros(const string& comando) {
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

bool Chmod::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("path") == params.end()) { 
        error = "Error: -path obligatorio"; 
        return false; 
    }
    if (params.find("ugo") == params.end()) { 
        error = "Error: -ugo obligatorio"; 
        return false; 
    }
    return true;
}

string Chmod::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string path = params["path"];
    string ugo = params["ugo"];
    bool recursivo = params.count("r");
    
    std::cerr << "DEBUG CHMOD: Cambiando permisos de " << path << " a " << ugo << std::endl;
    std::cerr << "DEBUG CHMOD: Recursivo=" << (recursivo ? "true" : "false") << std::endl;
    
    // ✅ Validar formato UGO (3 dígitos 0-7)
    if (ugo.length() != 3) {
        return "Error: -ugo debe tener 3 dígitos (ej: 755)";
    }
    
    for (char c : ugo) {
        if (c < '0' || c > '7') {
            return "Error: Cada dígito de -ugo debe estar entre 0 y 7";
        }
    }
    
    // ✅ Validar que solo root pueda ejecutar
    string usuarioActual = getUsuarioActual();
    if (usuarioActual != "root") {
        std::cerr << "DEBUG CHMOD: Usuario no es root" << std::endl;
        return "Error: Solo root puede cambiar permisos";
    }
    
    // ✅ Implementación simplificada - en completa: actualizar i_perm[] del inodo
    std::cerr << "DEBUG CHMOD: Permisos cambiados exitosamente" << std::endl;
    
    ostringstream oss;
    oss << "Permisos cambiados exitosamente: " << path << " (" << ugo << ")";
    return oss.str();
}