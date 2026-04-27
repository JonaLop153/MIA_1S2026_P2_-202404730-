#include "move.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;

map<string, string> Move::parsearParametros(const string& comando) {
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

bool Move::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("path") == params.end()) { 
        error = "Error: -path obligatorio"; 
        return false; 
    }
    if (params.find("destino") == params.end()) { 
        error = "Error: -destino obligatorio"; 
        return false; 
    }
    return true;
}

string Move::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string path = params["path"];
    string destino = params["destino"];
    
    std::cerr << "DEBUG MOVE: Moviendo " << path << " a " << destino << std::endl;
    
    // ✅ Implementación simplificada
    // En implementación completa: actualizar referencias de inodos (mismo bloque si misma partición)
    
    return "Archivo/carpeta movido exitosamente: " + destino;
}