#include "mkdir.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <ctime>

namespace fs = std::filesystem;

map<string, string> MkDir::parsearParametros(const string& comando) {
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

bool MkDir::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("path") == params.end()) { error = "Error: -path obligatorio"; return false; }
    return true;
}

bool MkDir::crearCarpeta(const string& id, const string& path, bool crearPadres) {
    MountMap& montadas = getParticionesMontadas();
    if (montadas.find(id) == montadas.end()) return false;
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    // ✅ Leer MBR
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) return false;
    MBR mbr; file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR)); file.close();
    int partStart = mbr.mbr_partitions[indice].part_start;
    
    // ✅ Leer SuperBlock
    ifstream sbFile(diskPath, ios::binary | ios::in);
    sbFile.seekg(partStart, ios::beg);
    SuperBlock sb; sbFile.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock)); sbFile.close();
    
    // ✅ Calcular tamaños de bitmaps
    int bitmapInodosSize = (sb.s_inodes_count + 7) / 8;
    int bitmapBloquesSize = (sb.s_blocks_count + 7) / 8;
    
    // ✅ Leer bitmaps CON partStart
    char* bitmapInodos = new char[bitmapInodosSize];
    ifstream bitmapInoFile(diskPath, ios::binary | ios::in);
    bitmapInoFile.seekg(partStart + sb.s_bm_inode_start, ios::beg);
    bitmapInoFile.read(bitmapInodos, bitmapInodosSize);
    bitmapInoFile.close();
    
    char* bitmapBloques = new char[bitmapBloquesSize];
    ifstream bitmapBloFile(diskPath, ios::binary | ios::in);
    bitmapBloFile.seekg(partStart + sb.s_bm_block_start, ios::beg);
    bitmapBloFile.read(bitmapBloques, bitmapBloquesSize);
    bitmapBloFile.close();
    
    // ✅ Buscar inodo libre
    int inodoLibre = -1;
    for (int i = sb.s_firts_ino; i < sb.s_inodes_count; i++) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        if ((bitmapInodos[byteIndex] & (1 << bitIndex)) == 0) {
            inodoLibre = i;
            break;
        }
    }
    
    if (inodoLibre == -1) { delete[] bitmapInodos; delete[] bitmapBloques; return false; }
    
    // ✅ Buscar bloque libre para carpeta
    int bloqueLibre = -1;
    for (int i = sb.s_first_blo; i < sb.s_blocks_count; i++) {
        int byteIndex = i / 8;
        int bitIndex = i % 8;
        if ((bitmapBloques[byteIndex] & (1 << bitIndex)) == 0) {
            bloqueLibre = i;
            break;
        }
    }
    
    if (bloqueLibre == -1) { delete[] bitmapInodos; delete[] bitmapBloques; return false; }
    
    // ✅ Obtener usuario actual
    SesionActiva& sesion = getSesionActiva();
    int uid = 1, gid = 1;
    
    // ✅ Crear inodo de carpeta
    Inodo inodoCarpeta;
    memset(&inodoCarpeta, 0, sizeof(Inodo));
    inodoCarpeta.i_uid = uid;
    inodoCarpeta.i_gid = gid;
    inodoCarpeta.i_s = 64;
    inodoCarpeta.i_atime = time(nullptr);
    inodoCarpeta.i_ctime = time(nullptr);
    inodoCarpeta.i_mtime = time(nullptr);
    inodoCarpeta.i_type = '0';
    inodoCarpeta.i_perm[0] = '6';
    inodoCarpeta.i_perm[1] = '6';
    inodoCarpeta.i_perm[2] = '4';
    inodoCarpeta.i_block[0] = bloqueLibre;
    for (int i = 1; i < 15; i++) inodoCarpeta.i_block[i] = -1;
    
    // ✅ Escribir inodo CON partStart
    fstream inodeFile(diskPath, ios::binary | ios::in | ios::out);
    inodeFile.seekp(partStart + sb.s_inode_start + (inodoLibre * sizeof(Inodo)), ios::beg);
    inodeFile.write(reinterpret_cast<char*>(&inodoCarpeta), sizeof(Inodo));
    inodeFile.close();
    
    // ✅ Crear bloque de carpeta con "." y ".."
    BloqueCarpeta bloqueCarpeta;
    memset(&bloqueCarpeta, 0, sizeof(BloqueCarpeta));
    strncpy(bloqueCarpeta.b_content[0].b_name, ".", 12);
    bloqueCarpeta.b_content[0].b_inodo = inodoLibre;
    strncpy(bloqueCarpeta.b_content[1].b_name, "..", 12);
    bloqueCarpeta.b_content[1].b_inodo = inodoLibre;
    
    // ✅ Escribir bloque CON partStart
    fstream blockFile(diskPath, ios::binary | ios::in | ios::out);
    blockFile.seekp(partStart + sb.s_block_start + (bloqueLibre * sb.s_block_s), ios::beg);
    blockFile.write(reinterpret_cast<char*>(&bloqueCarpeta), sizeof(BloqueCarpeta));
    blockFile.close();
    
    // ✅ Actualizar bitmaps CON partStart
    bitmapInodos[inodoLibre / 8] |= (1 << (inodoLibre % 8));
    bitmapBloques[bloqueLibre / 8] |= (1 << (bloqueLibre % 8));
    
    fstream bitmapInoWrite(diskPath, ios::binary | ios::in | ios::out);
    bitmapInoWrite.seekp(partStart + sb.s_bm_inode_start, ios::beg);
    bitmapInoWrite.write(bitmapInodos, bitmapInodosSize);
    bitmapInoWrite.close();
    
    fstream bitmapBloWrite(diskPath, ios::binary | ios::in | ios::out);
    bitmapBloWrite.seekp(partStart + sb.s_bm_block_start, ios::beg);
    bitmapBloWrite.write(bitmapBloques, bitmapBloquesSize);
    bitmapBloWrite.close();
    
    // ✅ Actualizar SuperBlock
    sb.s_free_inodes_count--;
    sb.s_free_blocks_count--;
    sb.s_firts_ino = inodoLibre + 1;
    sb.s_first_blo = bloqueLibre + 1;
    sb.s_mtime = time(nullptr);
    
    fstream sbWrite(diskPath, ios::binary | ios::in | ios::out);
    sbWrite.seekp(partStart, ios::beg);
    sbWrite.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    sbWrite.close();
    
    delete[] bitmapInodos;
    delete[] bitmapBloques;
    
    return true;
}

string MkDir::ejecutar(const string& comando) {
    if (!haySesionActiva()) return "Error: No hay sesión activa";
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string path = params["path"];
    bool p = params.count("p");
    string id = getIdParticionActual();
    
    if (crearCarpeta(id, path, p)) {
        return "Carpeta creada exitosamente: " + path;
    } else {
        return "Error: No se pudo crear la carpeta. Verifique espacio disponible";
    }
}