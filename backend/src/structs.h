#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint>
#include <cstring>
#include <ctime>

#pragma pack(push, 1)

// Partition - 32 bytes
struct Partition {
    char part_status;       // '0' = no montada, '1' = montada
    char part_type;         // 'P' = primaria, 'E' = extendida, 'L' = lógica
    char part_fit;          // 'B', 'F', 'W'
    int32_t part_start;     // Byte de inicio
    int32_t part_size;      // Tamaño en bytes
    char part_name[16];     // Nombre de la partición
    int32_t part_correlative;  // Correlativo
    char part_id[4];        // ID de montaje (ej: 301A)
};

// MBR - 64 bytes
struct MBR {
    int32_t mbr_tamano;           // Tamaño total del disco
    time_t mbr_fecha_creacion;    // Fecha de creación
    int32_t mbr_dsk_signature;    // Signature único
    char dsk_fit;                 // Ajuste del disco
    Partition mbr_partitions[4];  // 4 particiones
};

// EBR - Para particiones lógicas
struct EBR {
    char part_status;
    char part_fit;
    int32_t part_start;
    int32_t part_size;
    int32_t part_next;      // Siguiente EBR (-1 si no hay)
    char part_name[16];
};

// SuperBlock - EXT2/EXT3
struct SuperBlock {
    int32_t s_filesystem_type;    // 2 = EXT2, 3 = EXT3
    int32_t s_inodes_count;
    int32_t s_blocks_count;
    int32_t s_free_blocks_count;
    int32_t s_free_inodes_count;
    time_t s_mtime;
    time_t s_umtime;
    int32_t s_mnt_count;
    int32_t s_magic;              // 0xEF53
    int32_t s_inode_s;
    int32_t s_block_s;
    int32_t s_firts_ino;
    int32_t s_first_blo;
    int32_t s_bm_inode_start;
    int32_t s_bm_block_start;
    int32_t s_inode_start;
    int32_t s_block_start;
    int32_t s_journal_start;      // 🔥 NUEVO para EXT3
    int32_t s_journal_count;      // 🔥 NUEVO para EXT3
};

// Inodo - EXT2/EXT3
struct Inodo {
    int32_t i_uid;
    int32_t i_gid;
    int32_t i_s;
    time_t i_atime;
    time_t i_ctime;
    time_t i_mtime;
    int32_t i_block[15];    // 12 directos, 1 indirecto, 1 doble, 1 triple
    char i_type;            // '0' = carpeta, '1' = archivo
    char i_perm[3];         // Permisos UGO
};

// Contenido de bloque carpeta
struct Content {
    char b_name[12];
    int32_t b_inodo;
};

// Bloque Carpeta - 64 bytes
struct BloqueCarpeta {
    Content b_content[4];  // 4 * 16 = 64 bytes
};

// Bloque Archivo - 64 bytes
struct BloqueArchivo {
    char b_content[64];
};

// Bloque Apuntadores - 64 bytes
struct BloqueApuntador {
    int32_t b_pointers[16];  // 16 * 4 = 64 bytes
};

// 🔥 Journal - NUEVO para EXT3 (50 entries según enunciado)
struct JournalEntry {
    char operacion[20];     // Ej: "CREATE", "DELETE", "MODIFY"
    char ruta[60];          // Ruta del archivo/carpeta
    char contenido[100];    // Contenido (si aplica)
    time_t fecha;           // Timestamp de la operación
    int32_t usuario_id;     // ID del usuario que ejecutó
};

// Journal Block - 64 bytes (para almacenar en disco)
struct JournalBlock {
    char b_content[64];
};

#pragma pack(pop)

#endif