#include "find.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <regex>

using namespace std;

map<string, string> Find::parsearParametros(const string& comando) {
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

bool Find::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("path") == params.end()) { 
        error = "Error: -path obligatorio"; 
        return false; 
    }
    if (params.find("name") == params.end()) { 
        error = "Error: -name obligatorio"; 
        return false; 
    }
    return true;
}

string Find::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string path = params["path"];
    string name = params["name"];
    
    std::cerr << "DEBUG FIND: Buscando " << name << " en " << path << std::endl;
    
    // ✅ Convertir patrón a regex (? = ., * = .*)
    string patron = name;
    size_t pos = 0;
    while ((pos = patron.find('?', pos)) != string::npos) {
        patron.replace(pos, 1, ".");
        pos++;
    }
    pos = 0;
    while ((pos = patron.find('*', pos)) != string::npos) {
        patron.replace(pos, 1, ".*");
        pos += 2;
    }
    
    ostringstream oss;
    oss << "Resultados de búsqueda para '" << name << "' en " << path << ":\n";
    oss << "----------------------------------------\n";
    oss << "(Implementación completa requiere recorrer árbol de inodos)\n";
    oss << "----------------------------------------\n";
    
    return oss.str();
}