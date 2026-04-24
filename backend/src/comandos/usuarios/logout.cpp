#include "logout.h"
#include "../../session/session.h"
#include <sstream>
#include <iostream>

string Logout::ejecutar(const string& comando) {
    std::cerr << "========== DEBUG LOGOUT ==========" << std::endl;
    
    bool haySesion = haySesionActiva();
    std::cerr << "DEBUG: haySesionActiva=" << (haySesion ? "true" : "false") << std::endl;
    
    if (!haySesion) {
        std::cerr << "DEBUG: No hay sesión activa" << std::endl;
        std::cerr << "========== DEBUG LOGOUT FIN ==========" << std::endl;
        return "Error: No hay sesión activa";
    }
    
    string usuario = getUsuarioActual();
    std::cerr << "DEBUG: usuario='" << usuario << "'" << std::endl;
    
    std::cerr << "DEBUG: Llamando a cerrarSesion()" << std::endl;
    cerrarSesion();
    
    std::cerr << "DEBUG: Sesión cerrada" << std::endl;
    std::cerr << "========== DEBUG LOGOUT FIN ==========" << std::endl;
    
    ostringstream oss;
    oss << "Sesión cerrada exitosamente:\n";
    oss << "  Usuario: " << usuario;
    
    return oss.str();
}