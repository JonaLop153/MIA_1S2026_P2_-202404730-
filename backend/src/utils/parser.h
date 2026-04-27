#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <map>
#include <vector>

using namespace std;

class Parser {
public:
    // Parsear comando completo en tokens
    static vector<string> tokenize(const string& comando);
    
    // Parsear parámetros tipo -key=value
    static map<string, string> parsearParametros(const string& comando);
    
    // Validar que un parámetro exista
    static bool tieneParametro(const map<string, string>& params, const string& key);
    
    // Obtener valor de parámetro con default
    static string getParametro(const map<string, string>& params, const string& key, const string& defaultValue = "");
    
    // Convertir string a int con validación
    static bool parsearInt(const string& str, int& result);
    
    // Remover comillas de un string
    static string removerComillas(const string& str);
    
    // Convertir a minúsculas
    static string toLower(const string& str);
};

#endif