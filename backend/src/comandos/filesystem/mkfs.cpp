#include "mkfs.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <ctime>
#include <vector>

using namespace std;

map<string, string> Mkfs::parsearParametros(const string& comando) {
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

bool Mkfs::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("id") == params.end()) { 
        error = "Error: -id obligatorio"; 
        return false; 
    }
    return true;
}

bool Mkfs::formatearEXT2(const string& diskPath, int partStart, SuperBlock& sb) {
    std::cerr << "DEBUG MKFS: Formateando EXT2" << std::endl;
    
    // Calcular inodos y bloques (según enunciado P1)
    int particionSize = sb.s_blocks_count * 64;  // Tamaño de partición
    int n = (particionSize - 157) / (64 + 3*64 + 128 + 3*64);  // Aproximado
    if (n > 100) n = 100;  // Límite práctico
    
    sb.s_filesystem_type = 2;  // EXT2
    sb.s_inodes_count = n;
    sb.s_blocks_count = 3 * n;
    sb.s_free_blocks_count = 3 * n - 4;  // Reservar primeros bloques
    sb.s_free_inodes_count = n - 4;  // Reservar primeros inodos
    sb.s_mtime = time(nullptr);
    sb.s_umtime = 0;
    sb.s_mnt_count = 0;
    sb.s_magic = 0xEF53;
    sb.s_inode_s = sizeof(Inodo);
    sb.s_block_s = 64;
    sb.s_firts_ino = 4;
    sb.s_first_blo = 4;
    
    // Offsets
    sb.s_bm_inode_start = 157 + sizeof(SuperBlock);
    sb.s_bm_block_start = sb.s_bm_inode_start + (n + 7) / 8;
    sb.s_inode_start = sb.s_bm_block_start + (3 * n + 7) / 8;
    sb.s_block_start = sb.s_inode_start + n * sizeof(Inodo);
    sb.s_journal_start = 0;  // EXT2 no tiene journal
    sb.s_journal_count = 0;
    
    // Escribir SuperBlock
    fstream sbFile(diskPath, ios::binary | ios::out | ios::in);
    sbFile.seekp(partStart, ios::beg);
    sbFile.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    sbFile.flush();
    
    // Inicializar bitmaps (todos 0 = libres)
    int bitmapInoSize = (n + 7) / 8;
    int bitmapBloSize = (3 * n + 7) / 8;
    vector<char> bitmapIno(bitmapInoSize, 0);
    vector<char> bitmapBlo(bitmapBloSize, 0);
    
    // Marcar primeros como ocupados
    bitmapIno[0] = 0x0F;  // Primeros 4 inodos ocupados
    bitmapBlo[0] = 0x0F;  // Primeros 4 bloques ocupados
    
    sbFile.seekp(partStart + sb.s_bm_inode_start, ios::beg);
    sbFile.write(bitmapIno.data(), bitmapInoSize);
    sbFile.seekp(partStart + sb.s_bm_block_start, ios::beg);
    sbFile.write(bitmapBlo.data(), bitmapBloSize);
    sbFile.flush();
    
    // Crear inodo raíz (inodo 2)
    Inodo inodoRaiz;
    memset(&inodoRaiz, 0, sizeof(Inodo));
    inodoRaiz.i_uid = 1;
    inodoRaiz.i_gid = 1;
    inodoRaiz.i_s = 64;
    inodoRaiz.i_atime = time(nullptr);
    inodoRaiz.i_ctime = time(nullptr);
    inodoRaiz.i_mtime = time(nullptr);
    inodoRaiz.i_type = '0';  // Carpeta
    inodoRaiz.i_perm[0] = '6';
    inodoRaiz.i_perm[1] = '6';
    inodoRaiz.i_perm[2] = '4';
    inodoRaiz.i_block[0] = 0;  // Bloque 0 relativo
    for (int i = 1; i < 15; i++) inodoRaiz.i_block[i] = -1;
    
    sbFile.seekp(partStart + sb.s_inode_start + 2 * sizeof(Inodo), ios::beg);
    sbFile.write(reinterpret_cast<char*>(&inodoRaiz), sizeof(Inodo));
    sbFile.flush();
    
    // Crear bloque raíz con "." y ".."
    BloqueCarpeta bloqueRaiz;
    memset(&bloqueRaiz, 0, sizeof(BloqueCarpeta));
    strncpy(bloqueRaiz.b_content[0].b_name, ".", 12);
    bloqueRaiz.b_content[0].b_inodo = 2;
    strncpy(bloqueRaiz.b_content[1].b_name, "..", 12);
    bloqueRaiz.b_content[1].b_inodo = 2;
    
    sbFile.seekp(partStart + sb.s_block_start, ios::beg);
    sbFile.write(reinterpret_cast<char*>(&bloqueRaiz), sizeof(BloqueCarpeta));
    sbFile.flush();
    
    // Crear inodo users.txt (inodo 3)
    Inodo inodoUsers;
    memset(&inodoUsers, 0, sizeof(Inodo));
    inodoUsers.i_uid = 1;
    inodoUsers.i_gid = 1;
    inodoUsers.i_s = 64;
    inodoUsers.i_atime = time(nullptr);
    inodoUsers.i_ctime = time(nullptr);
    inodoUsers.i_mtime = time(nullptr);
    inodoUsers.i_type = '1';  // Archivo
    inodoUsers.i_perm[0] = '6';
    inodoUsers.i_perm[1] = '6';
    inodoUsers.i_perm[2] = '4';
    inodoUsers.i_block[0] = 3;  // Bloque 3 relativo
    for (int i = 1; i < 15; i++) inodoUsers.i_block[i] = -1;
    
    sbFile.seekp(partStart + sb.s_inode_start + 3 * sizeof(Inodo), ios::beg);
    sbFile.write(reinterpret_cast<char*>(&inodoUsers), sizeof(Inodo));
    sbFile.flush();
    
    // Crear users.txt con root
    BloqueArchivo usersContent;
    memset(&usersContent, 0, sizeof(BloqueArchivo));
    string usersData = "1,G,root\n1,U,root,root,123\n";
    strncpy(usersContent.b_content, usersData.c_str(), 63);
    
    sbFile.seekp(partStart + sb.s_block_start + 3 * 64, ios::beg);
    sbFile.write(reinterpret_cast<char*>(&usersContent), sizeof(BloqueArchivo));
    sbFile.flush();
    sbFile.close();
    
    std::cerr << "DEBUG MKFS: EXT2 formateado exitosamente" << std::endl;
    return true;
}

bool Mkfs::formatearEXT3(const string& diskPath, int partStart, SuperBlock& sb) {
    std::cerr << "DEBUG MKFS: Formateando EXT3" << std::endl;
    
    // Calcular según fórmula del enunciado
    int particionSize = sb.s_blocks_count * 64;
    int journalSize = 50 * sizeof(JournalEntry);  // 50 entries según enunciado
    int n = (particionSize - sizeof(SuperBlock)) / (sizeof(JournalEntry) + 1 + 3 + sizeof(Inodo) + 3*64);
    if (n > 100) n = 100;
    
    sb.s_filesystem_type = 3;  // EXT3
    sb.s_inodes_count = n;
    sb.s_blocks_count = 3 * n;
    sb.s_free_blocks_count = 3 * n - 5;  // Reservar + journal
    sb.s_free_inodes_count = n - 4;
    sb.s_mtime = time(nullptr);
    sb.s_umtime = 0;
    sb.s_mnt_count = 0;
    sb.s_magic = 0xEF53;
    sb.s_inode_s = sizeof(Inodo);
    sb.s_block_s = 64;
    sb.s_firts_ino = 4;
    sb.s_first_blo = 5;  // Bloque 0 = journal
    
    // Offsets
    sb.s_journal_start = 157 + sizeof(SuperBlock);
    sb.s_bm_inode_start = sb.s_journal_start + journalSize;
    sb.s_bm_block_start = sb.s_bm_inode_start + (n + 7) / 8;
    sb.s_inode_start = sb.s_bm_block_start + (3 * n + 7) / 8;
    sb.s_block_start = sb.s_inode_start + n * sizeof(Inodo);
    sb.s_journal_count = 50;
    
    // Escribir SuperBlock
    fstream sbFile(diskPath, ios::binary | ios::out | ios::in);
    sbFile.seekp(partStart, ios::beg);
    sbFile.write(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    sbFile.flush();
    
    // Inicializar journal (todos 0)
    vector<char> journal(journalSize, 0);
    sbFile.seekp(partStart + sb.s_journal_start, ios::beg);
    sbFile.write(journal.data(), journalSize);
    sbFile.flush();
    sbFile.close();
    
    std::cerr << "DEBUG MKFS: EXT3 formateado exitosamente" << std::endl;
    return true;
}

string Mkfs::ejecutar(const string& comando) {
    std::cerr << "DEBUG MKFS: Ejecutando comando" << std::endl;
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string id = params["id"];
    string fs = params.count("fs") ? params["fs"] : "2fs";  // Default EXT2
    string type = params.count("type") ? params["type"] : "full";
    
    MountMap& montadas = getParticionesMontadas();
    if (montadas.find(id) == montadas.end()) {
        std::cerr << "DEBUG MKFS: ID no encontrado" << std::endl;
        return "Error: No hay partición montada con ID: " + id;
    }
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    // Leer MBR
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) return "Error: No se pudo abrir el disco";
    MBR mbr; file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR)); file.close();
    int partStart = mbr.mbr_partitions[indice].part_start;
    
    // Leer tamaño de partición para SuperBlock temporal
    ifstream tempFile(diskPath, ios::binary | ios::in);
    tempFile.seekg(partStart, ios::beg);
    SuperBlock sb;
    tempFile.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    tempFile.close();
    
    // Formatear según fs
    bool exito = false;
    if (fs == "3fs") {
        exito = formatearEXT3(diskPath, partStart, sb);
    } else {
        exito = formatearEXT2(diskPath, partStart, sb);
    }
    
    if (exito) {
        ostringstream oss;
        oss << "Partición formateada con EXT" << (fs == "3fs" ? "3" : "2") << " exitosamente:\n";
        oss << "  ID: " << id << "\n";
        oss << "  Type: " << type << "\n";
        oss << "  Inodos totales: " << sb.s_inodes_count << "\n";
        oss << "  Bloques totales: " << sb.s_blocks_count << "\n";
        oss << "  Tamaño de bloque: 64 bytes\n";
        oss << "  SuperBlock: byte 157\n";
        oss << "  Bitmap Inodos: byte " << sb.s_bm_inode_start << "\n";
        oss << "  Bitmap Bloques: byte " << sb.s_bm_block_start << "\n";
        oss << "  Tabla Inodos: byte " << sb.s_inode_start << "\n";
        oss << "  Tabla Bloques: byte " << sb.s_block_start << "\n";
        if (fs == "3fs") {
            oss << "  Journal: byte " << sb.s_journal_start << "\n";
        }
        oss << "  users.txt: creado en raíz (inodo 3, bloque 3)";
        return oss.str();
    } else {
        return "Error: No se pudo formatear la partición";
    }
}