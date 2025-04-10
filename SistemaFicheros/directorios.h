/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 */

#include "ficheros.h"

#define TAMNOMBRE 60 // tamaño del nombre de directorio o fichero, en Ext2 = 256
struct entrada
{
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};

#define ERROR_CAMINO_INCORRECTO (-2)
#define ERROR_PERMISO_LECTURA (-3)
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA (-4)
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO (-5)
#define ERROR_PERMISO_ESCRITURA (-6)
#define ERROR_ENTRADA_YA_EXISTENTE (-7)
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO (-8)

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -2:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -3:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -6:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -7:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -8:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);

int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer);
//int mi_dir(const char *camino, char *buffer, char tipo);
//int mi_dir(const char *camino, char *buffer, char flag);
//int mi_dir(const char *camino, char *buffer, char tipo, char flag);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
