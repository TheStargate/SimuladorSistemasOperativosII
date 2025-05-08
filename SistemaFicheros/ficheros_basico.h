/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Libreria de funciones para la gestión de un sistema de ficheros básico.
 */

#include <time.h>
#include <limits.h>
#include "bloques.h"

#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 0
#define DEBUGN6 0
#define DEBUGN7 1
#define DEBUGN8 1

#define posSB 0 // el superbloque se escribe en el primer bloque de nuestro FS
#define tamSB 1

#define INODOSIZE 128 // tamaño en bytes de un inodo
#define BYTE 8        // tamaño en bits de un Byte
#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) // 256 punteros por bloque
#define DIRECTOS 12
#define INDIRECTOS0 (NPUNTEROS + DIRECTOS) // 268
#define INDIRECTOS1 (NPUNTEROS * NPUNTEROS + INDIRECTOS0) // 65.804
#define INDIRECTOS2 (NPUNTEROS * NPUNTEROS * NPUNTEROS + INDIRECTOS1) // 16.843.020

struct superbloque
{
    unsigned int posPrimerBloqueMB;                      // Posición absoluta del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB;                      // Posición absoluta del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI;                      // Posición absoluta del primer bloque del array de inodos
    unsigned int posUltimoBloqueAI;                      // Posición absoluta del último bloque del array de inodos
    unsigned int posPrimerBloqueDatos;                   // Posición absoluta del primer bloque de datos
    unsigned int posUltimoBloqueDatos;                   // Posición absoluta del último bloque de datos
    unsigned int posInodoRaiz;                           // Posición del inodo del directorio raíz (relativa al AI)
    unsigned int posPrimerInodoLibre;                    // Posición del primer inodo libre (relativa al AI)
    unsigned int cantBloquesLibres;                      // Cantidad de bloques libres (en todo el disco)
    unsigned int cantInodosLibres;                       // Cantidad de inodos libres (en el AI)
    unsigned int totBloques;                             // Cantidad total de bloques del disco
    unsigned int totInodos;                              // Cantidad total de inodos (heurística)
    char padding[BLOCKSIZE - 12 * sizeof(unsigned int)]; // Relleno para ocupar el bloque completo
};

struct inodo
{
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)

    unsigned char reservado_alineacion1[6];

    time_t atime; // Fecha y hora del último acceso a datos
    time_t mtime; // Fecha y hora de la última modificación de datos
    time_t ctime; // Fecha y hora de la última modificación del inodo
    time_t btime; // Fecha y hora de creación del inodo (birth)

    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos (EOF)
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos

    unsigned int punterosDirectos[12];  // 12 punteros a bloques directos
    unsigned int punterosIndirectos[3]; /* 3 punteros a bloques indirectos:
    1 indirecto simple, 1 indirecto doble, 1 indirecto triple */

    /* Utilizar una variable de alineación si es necesario  para vuestra plataforma/compilador   */
    char padding[INODOSIZE - 2 * sizeof(unsigned char) - 4 * sizeof(time_t) - 18 * sizeof(unsigned int) - 6 * sizeof(unsigned char)];
    // Fijarse que también se resta lo que ocupen las variables de alineación utilizadas!!!
};


int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB();
int initAI();

int escribir_bit(unsigned int nbloque, unsigned int bit);
char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, struct inodo *inodo);
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);

int traducir_bloque_inodo(unsigned int inodo, unsigned int nblogico, unsigned char reservar);
int obtener_indice(unsigned int nblogico, int nivel_punteros);
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ent);

int liberar_inodo(unsigned int ninodo);
int liberar_bloques_inodo (unsigned int primerBL, struct inodo *inodo);