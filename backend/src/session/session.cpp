#include "session.h"
#include <iostream>

static MountMap* particionesMontadasPtr = nullptr;
static SesionActiva* sesionActivaPtr = nullptr;

MountMap& getParticionesMontadas() {
    if (!particionesMontadasPtr) {
        particionesMontadasPtr = new MountMap();
    }
    return *particionesMontadasPtr;
}

SesionActiva& getSesionActiva() {
    if (!sesionActivaPtr) {
        sesionActivaPtr = new SesionActiva();
        sesionActivaPtr->activa = false;
        sesionActivaPtr->indiceParticion = -1;
    }
    return *sesionActivaPtr;
}

void mountPartition(const string& id, const string& path, int indice) {
    std::cerr << "DEBUG SESSION: mountPartition id='" << id << "', path='" << path << "', indice=" << indice << std::endl;
    getParticionesMontadas()[id] = make_pair(path, indice);
}

void unmountPartition(const string& id) {
    std::cerr << "DEBUG SESSION: unmountPartition id='" << id << "'" << std::endl;
    getParticionesMontadas().erase(id);
}

bool isMounted(const string& path, int indice) {
    for (auto const& [id, data] : getParticionesMontadas()) {
        if (data.first == path && data.second == indice) {
            return true;
        }
    }
    return false;
}

string getMountID(const string& path, int indice) {
    for (auto const& [id, data] : getParticionesMontadas()) {
        if (data.first == path && data.second == indice) {
            return id;
        }
    }
    return "";
}

void iniciarSesion(const string& user, const string& pass, const string& id, const string& diskPath) {
    std::cerr << "DEBUG SESSION: iniciarSesion user='" << user << "', id='" << id << "', diskPath='" << diskPath << "'" << std::endl;
    SesionActiva& sesion = getSesionActiva();
    sesion.usuario = user;
    sesion.password = pass;
    sesion.idParticion = id;
    sesion.pathDisco = diskPath;
    sesion.indiceParticion = -1;
    sesion.activa = true;
    std::cerr << "DEBUG SESSION: sesion.activa=" << (sesion.activa ? "true" : "false") << std::endl;
}

void cerrarSesion() {
    std::cerr << "DEBUG SESSION: cerrarSesion INICIO" << std::endl;
    SesionActiva& sesion = getSesionActiva();
    std::cerr << "DEBUG SESSION: antes de cerrar - sesion.activa=" << (sesion.activa ? "true" : "false") << std::endl;
    std::cerr << "DEBUG SESSION: antes de cerrar - sesion.usuario='" << sesion.usuario << "'" << std::endl;
    sesion.usuario = "";
    sesion.password = "";
    sesion.idParticion = "";
    sesion.pathDisco = "";
    sesion.indiceParticion = -1;
    sesion.activa = false;
    std::cerr << "DEBUG SESSION: después de cerrar - sesion.activa=" << (sesion.activa ? "true" : "false") << std::endl;
    std::cerr << "DEBUG SESSION: cerrarSesion FIN" << std::endl;
}

bool haySesionActiva() {
    bool activa = getSesionActiva().activa;
    std::cerr << "DEBUG SESSION: haySesionActiva=" << (activa ? "true" : "false") << std::endl;
    return activa;
}

string getUsuarioActual() {
    return getSesionActiva().usuario;
}

string getIdParticionActual() {
    return getSesionActiva().idParticion;
}

string getPathDiscoActual() {
    return getSesionActiva().pathDisco;
}