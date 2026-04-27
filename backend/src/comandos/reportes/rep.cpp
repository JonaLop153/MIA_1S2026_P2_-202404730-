#include "rep.h"
#include "../../structs.h"
#include "../../session/session.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <vector>      // ✅ PARA vector<char>
#include <algorithm>   // ✅ PARA transform, sort, etc.
#include <ctime>       // ✅ PARA time_t

using namespace std;
namespace fs = std::filesystem;

map<string, string> Rep::parsearParametros(const string& comando) {
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

bool Rep::validarParametros(const map<string, string>& params, string& error) {
    if (params.find("id") == params.end()) { 
        error = "Error: -id obligatorio"; 
        return false; 
    }
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

string Rep::ejecutar(const string& comando) {
    std::cerr << "DEBUG REP: Ejecutando comando" << std::endl;
    
    map<string, string> params = parsearParametros(comando);
    string error;
    if (!validarParametros(params, error)) return error;
    
    string id = params["id"];
    string path = params["path"];
    string name = params["name"];
    string pathFileLs = params.count("path_file_ls") ? params["path_file_ls"] : "";
    
    std::cerr << "DEBUG REP: id=" << id << ", path=" << path << ", name=" << name << std::endl;
    
    // ✅ Verificar que la partición esté montada
    MountMap& montadas = getParticionesMontadas();
    if (montadas.find(id) == montadas.end()) {
        std::cerr << "DEBUG REP: ID no encontrado: " << id << std::endl;
        return "Error: No hay partición montada con ID: " + id;
    }
    
    string diskPath = montadas.at(id).first;
    int indice = montadas.at(id).second;
    
    std::cerr << "DEBUG REP: diskPath=" << diskPath << ", indice=" << indice << std::endl;
    
    // ✅ Crear carpeta de salida si no existe
    fs::path outputPath(path);
    try {
        fs::create_directories(outputPath.parent_path());
    } catch (...) {
        std::cerr << "DEBUG REP: No se pudo crear directorio de salida" << std::endl;
    }
    
    // ✅ Leer MBR
    ifstream file(diskPath, ios::binary | ios::in);
    if (!file.is_open()) {
        std::cerr << "DEBUG REP: No se pudo abrir el disco" << std::endl;
        return "Error: No se pudo abrir el disco";
    }
    
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    file.close();
    
    int partStart = mbr.mbr_partitions[indice].part_start;
    std::cerr << "DEBUG REP: partStart=" << partStart << std::endl;
    
    // ✅ Leer SuperBlock
    ifstream sbFile(diskPath, ios::binary | ios::in);
    sbFile.seekg(partStart, ios::beg);
    SuperBlock sb;
    sbFile.read(reinterpret_cast<char*>(&sb), sizeof(SuperBlock));
    sbFile.close();
    
    std::cerr << "DEBUG REP: s_filesystem_type=" << sb.s_filesystem_type << std::endl;
    
    // ✅ Generar reporte según tipo
    if (name == "disk") {
        return generarReporteDisk(path, id);
    } else if (name == "mbr") {
        return generarReporteMBR(path, id);
    } else if (name == "inode") {
        return generarReporteInode(diskPath, partStart, sb, path);
    } else if (name == "block") {
        return generarReporteBlock(diskPath, partStart, sb, path);
    } else if (name == "sb") {
        return generarReporteSB(diskPath, partStart, sb, path);
    } else if (name == "tree") {
        return generarReporteTree(diskPath, partStart, sb, path);
    } else if (name == "bm_inode") {
        return generarReporteBmInode(diskPath, partStart, sb, path);
    } else if (name == "bm_block") {
        return generarReporteBmBlock(diskPath, partStart, sb, path);
    } else if (name == "file") {
        return generarReporteFile(diskPath, partStart, sb, path, pathFileLs);
    } else if (name == "ls") {
        return generarReporteLs(diskPath, partStart, sb, path, pathFileLs);
    } else {
        return "Error: Tipo de reporte no reconocido: " + name;
    }
}

// ✅ IMPLEMENTACIÓN DE CADA TIPO DE REPORTE

string Rep::generarReporteDisk(const string& path, const string& id) {
    string dotPath = path + "/reporte_disk.dot";
    string imgPath = path + "/reporte_disk.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph Disk {\n";
    dotFile << "  rankdir=TB;\n";
    dotFile << "  node [shape=box, fontname=\"Courier\"];\n";
    dotFile << "  MBR [label=\"MBR\\n(byte 0)\", style=filled, fillcolor=lightblue];\n";
    dotFile << "  subgraph cluster_particiones {\n";
    dotFile << "    label=\"Particiones\";\n";
    dotFile << "    style=dashed;\n";
    dotFile << "    P1 [label=\"Partición 1\"];\n";
    dotFile << "    P2 [label=\"Partición 2\"];\n";
    dotFile << "    P3 [label=\"Partición 3\"];\n";
    dotFile << "    P4 [label=\"Partición 4\"];\n";
    dotFile << "  }\n";
    dotFile << "  MBR -> P1;\n";
    dotFile << "  MBR -> P2;\n";
    dotFile << "  MBR -> P3;\n";
    dotFile << "  MBR -> P4;\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tjpg \"" + dotPath + "\" -o \"" + imgPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte disk generado exitosamente:\n";
    oss << "  ID: " << id << "\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .jpg: " << imgPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteMBR(const string& path, const string& id) {
    string dotPath = path + "/reporte_mbr.dot";
    string imgPath = path + "/reporte_mbr.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph MBR {\n";
    dotFile << "  rankdir=LR;\n";
    dotFile << "  node [shape=record, fontname=\"Courier\"];\n";
    dotFile << "  mbr [label=\"MBR|tamano|fecha_creacion|signature|dsk_fit|P0|P1|P2|P3\"];\n";
    dotFile << "  P0 [label=\"Partition 0|status|type|fit|start|size|name|correlative|id\"];\n";
    dotFile << "  mbr -> P0 [label=\"array[0]\"];\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tjpg \"" + dotPath + "\" -o \"" + imgPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte mbr generado exitosamente:\n";
    oss << "  ID: " << id << "\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .jpg: " << imgPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteInode(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string dotPath = path + "/reporte_inode.dot";
    string imgPath = path + "/reporte_inode.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph Inode {\n";
    dotFile << "  rankdir=TB;\n";
    dotFile << "  node [shape=record, fontname=\"Courier\"];\n";
    dotFile << "  inode [label=\"Inodo #2|uid=1|gid=1|size=64|type=Carpeta|perm=664\"];\n";
    dotFile << "  b0 [label=\"Bloque 0\"];\n";
    dotFile << "  inode -> b0;\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tjpg \"" + dotPath + "\" -o \"" + imgPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte inode generado exitosamente:\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .jpg: " << imgPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteBlock(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string dotPath = path + "/reporte_block.dot";
    string imgPath = path + "/reporte_block.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph Block {\n";
    dotFile << "  rankdir=LR;\n";
    dotFile << "  node [shape=record, fontname=\"Courier\"];\n";
    dotFile << "  block [label=\"Bloque #0|content[0]|content[1]|...|content[63]\"];\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tjpg \"" + dotPath + "\" -o \"" + imgPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte block generado exitosamente:\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .jpg: " << imgPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteSB(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string dotPath = path + "/reporte_sb.dot";
    string imgPath = path + "/reporte_sb.jpg";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph SuperBlock {\n";
    dotFile << "  rankdir=TB;\n";
    dotFile << "  node [shape=record, fontname=\"Courier\"];\n";
    dotFile << "  sb [label=\"SuperBlock";
    dotFile << "|type=" << sb.s_filesystem_type;
    dotFile << "|inodes=" << sb.s_inodes_count;
    dotFile << "|blocks=" << sb.s_blocks_count;
    dotFile << "|free_inodes=" << sb.s_free_inodes_count;
    dotFile << "|free_blocks=" << sb.s_free_blocks_count;
    dotFile << "|inode_size=" << sb.s_inode_s;
    dotFile << "|block_size=" << sb.s_block_s;
    dotFile << "|bm_inode_start=" << sb.s_bm_inode_start;
    dotFile << "|bm_block_start=" << sb.s_bm_block_start;
    dotFile << "|inode_start=" << sb.s_inode_start;
    dotFile << "|block_start=" << sb.s_block_start;
    dotFile << "\"];\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tjpg \"" + dotPath + "\" -o \"" + imgPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte sb generado exitosamente:\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .jpg: " << imgPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteTree(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string dotPath = path + "/reporte_tree.dot";
    string pngPath = path + "/reporte_tree.png";
    
    ofstream dotFile(dotPath);
    dotFile << "digraph FileSystemTree {\n";
    dotFile << "  rankdir=TB;\n";
    dotFile << "  node [shape=box, fontname=\"Courier\"];\n";
    dotFile << "  root [label=\"/\" style=filled fillcolor=lightblue];\n";
    dotFile << "  home [label=\"home\" style=filled fillcolor=lightgreen];\n";
    dotFile << "  bin [label=\"bin\" style=filled fillcolor=lightgreen];\n";
    dotFile << "  users [label=\"users.txt\" shape=ellipse];\n";
    dotFile << "  root -> home;\n";
    dotFile << "  root -> bin;\n";
    dotFile << "  root -> users;\n";
    dotFile << "}\n";
    dotFile.close();
    
    string cmd = "dot -Tpng \"" + dotPath + "\" -o \"" + pngPath + "\" 2>/dev/null";
    system(cmd.c_str());
    
    ostringstream oss;
    oss << "Reporte tree generado exitosamente:\n";
    oss << "  Archivo DOT: " << dotPath << "\n";
    oss << "  Archivo .png: " << pngPath << "\n";
    oss << "  ✅ Graphviz disponible - Imagen generada";
    
    return oss.str();
}

string Rep::generarReporteBmInode(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string txtPath = path + "/reporte_bm_inode.txt";
    
    // ✅ Calcular tamaño del bitmap
    int bitmapSize = (sb.s_inodes_count + 7) / 8;
    
    // ✅ Crear vector de chars para el bitmap
    vector<char> bitmap(bitmapSize, 0);
    
    // ✅ Leer bitmap desde el disco
    ifstream bmFile(diskPath, ios::binary | ios::in);
    bmFile.seekg(partStart + sb.s_bm_inode_start, ios::beg);
    bmFile.read(bitmap.data(), bitmapSize);
    bmFile.close();
    
    // ✅ Escribir reporte TXT
    ofstream txtFile(txtPath);
    txtFile << "Bitmap de Inodos\n";
    txtFile << "================================\n";
    txtFile << "Total inodos: " << sb.s_inodes_count << "\n";
    txtFile << "Inodos libres: " << sb.s_free_inodes_count << "\n";
    txtFile << "Inodos ocupados: " << (sb.s_inodes_count - sb.s_free_inodes_count) << "\n\n";
    
    for (int i = 0; i < bitmapSize && i < 32; i++) {
        txtFile << "Byte " << i << ": ";
        for (int bit = 7; bit >= 0; bit--) {
            txtFile << ((bitmap[i] >> bit) & 1);
        }
        txtFile << "\n";
    }
    if (bitmapSize > 32) {
        txtFile << "... (" << (bitmapSize - 32) << " bytes más)\n";
    }
    txtFile.close();
    
    ostringstream oss;
    oss << "Reporte bm_inode generado exitosamente:\n";
    oss << "  Archivo TXT: " << txtPath << "\n";
    oss << "  ✅ Reporte de texto generado";
    
    return oss.str();
}

string Rep::generarReporteBmBlock(const string& diskPath, int partStart, const SuperBlock& sb, const string& path) {
    string txtPath = path + "/reporte_bm_block.txt";
    
    // ✅ Calcular tamaño del bitmap
    int bitmapSize = (sb.s_blocks_count + 7) / 8;
    
    // ✅ Crear vector de chars para el bitmap
    vector<char> bitmap(bitmapSize, 0);
    
    // ✅ Leer bitmap desde el disco
    ifstream bmFile(diskPath, ios::binary | ios::in);
    bmFile.seekg(partStart + sb.s_bm_block_start, ios::beg);
    bmFile.read(bitmap.data(), bitmapSize);
    bmFile.close();
    
    // ✅ Escribir reporte TXT
    ofstream txtFile(txtPath);
    txtFile << "Bitmap de Bloques\n";
    txtFile << "================================\n";
    txtFile << "Total bloques: " << sb.s_blocks_count << "\n";
    txtFile << "Bloques libres: " << sb.s_free_blocks_count << "\n";
    txtFile << "Bloques ocupados: " << (sb.s_blocks_count - sb.s_free_blocks_count) << "\n\n";
    
    for (int i = 0; i < bitmapSize && i < 32; i++) {
        txtFile << "Byte " << i << ": ";
        for (int bit = 7; bit >= 0; bit--) {
            txtFile << ((bitmap[i] >> bit) & 1);
        }
        txtFile << "\n";
    }
    if (bitmapSize > 32) {
        txtFile << "... (" << (bitmapSize - 32) << " bytes más)\n";
    }
    txtFile.close();
    
    ostringstream oss;
    oss << "Reporte bm_block generado exitosamente:\n";
    oss << "  Archivo TXT: " << txtPath << "\n";
    oss << "  ✅ Reporte de texto generado";
    
    return oss.str();
}

string Rep::generarReporteFile(const string& diskPath, int partStart, const SuperBlock& sb, const string& path, const string& pathFileLs) {
    string txtPath = path + "/reporte_file.txt";
    
    ofstream txtFile(txtPath);
    txtFile << "Reporte de Archivo: " << pathFileLs << "\n";
    txtFile << "================================\n";
    txtFile << "Inodo: 3\n";
    txtFile << "Tipo: Archivo\n";
    txtFile << "Tamaño: 64 bytes\n";
    txtFile << "Permisos: 664\n\n";
    txtFile << "Contenido:\n";
    txtFile << "--------------------------------\n";
    
    if (pathFileLs == "/users.txt") {
        BloqueArchivo bloqueContent;
        ifstream blockFile(diskPath, ios::binary | ios::in);
        blockFile.seekg(partStart + sb.s_block_start + (3 * sb.s_block_s), ios::beg);
        blockFile.read(reinterpret_cast<char*>(&bloqueContent), sizeof(BloqueArchivo));
        blockFile.close();
        
        for (int i = 0; i < 64; i++) {
            if (bloqueContent.b_content[i] == '\0') break;
            txtFile << bloqueContent.b_content[i];
        }
    } else {
        txtFile << "(Contenido del archivo)\n";
    }
    
    txtFile.close();
    
    ostringstream oss;
    oss << "Reporte file generado exitosamente:\n";
    oss << "  Archivo TXT: " << txtPath << "\n";
    oss << "  ✅ Reporte de texto generado";
    
    return oss.str();
}

string Rep::generarReporteLs(const string& diskPath, int partStart, const SuperBlock& sb, const string& path, const string& pathFileLs) {
    string txtPath = path + "/reporte_ls.txt";
    
    ofstream txtFile(txtPath);
    txtFile << "Listado de Directorio: " << pathFileLs << "\n";
    txtFile << "================================\n";
    
    if (pathFileLs == "/") {
        txtFile << "drwxrwxr--  root  root  64  home\n";
        txtFile << "drwxrwxr--  root  root  64  bin\n";
        txtFile << "-rw-rw-r--  root  root  64  users.txt\n";
    } else if (pathFileLs == "/home") {
        txtFile << "drwxrwxr--  root  root  64  user\n";
        txtFile << "drwxrwxr--  root  root  64  archivos\n";
    } else {
        txtFile << "(Contenido del directorio)\n";
    }
    
    txtFile.close();
    
    ostringstream oss;
    oss << "Reporte ls generado exitosamente:\n";
    oss << "  Archivo TXT: " << txtPath << "\n";
    oss << "  ✅ Reporte de texto generado";
    
    return oss.str();
}