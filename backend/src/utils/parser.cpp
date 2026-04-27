#include "parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

vector<string> Parser::tokenize(const string& comando) {
    vector<string> tokens;
    istringstream iss(comando);
    string token;
    
    while (iss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

map<string, string> Parser::parsearParametros(const string& comando) {
    map<string, string> params;
    istringstream iss(comando);
    string token;
    
    // Saltar el nombre del comando
    iss >> token;
    
    while (iss >> token) {
        if (token[0] == '-') {
            size_t eqPos = token.find('=');
            if (eqPos != string::npos) {
                string key = token.substr(1, eqPos - 1);
                string value = token.substr(eqPos + 1);
                
                // Remover comillas si existen
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                
                // Convertir key a minúsculas
                transform(key.begin(), key.end(), key.begin(), ::tolower);
                
                params[key] = value;
            }
        }
    }
    
    return params;
}

bool Parser::tieneParametro(const map<string, string>& params, const string& key) {
    return params.find(key) != params.end();
}

string Parser::getParametro(const map<string, string>& params, const string& key, const string& defaultValue) {
    auto it = params.find(key);
    return (it != params.end()) ? it->second : defaultValue;
}

bool Parser::parsearInt(const string& str, int& result) {
    try {
        result = stoi(str);
        return true;
    } catch (...) {
        return false;
    }
}

string Parser::removerComillas(const string& str) {
    if (str.empty()) return str;
    string result = str;
    if (result.front() == '"' && result.back() == '"') {
        result = result.substr(1, result.length() - 2);
    }
    return result;
}

string Parser::toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}