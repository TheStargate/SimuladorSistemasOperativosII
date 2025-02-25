/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar un sistema de ficheros
 */

#include "ficheros_basico.h"

/**
 * Función para calcular el tamaño en bloques necesario para el mapa de bits.
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return Tamaño en bloques necesario para el mapa de bits.
 */
int tamMB(unsigned int nbloques)
{

    if (nbloques % BYTE == 0 && nbloques / BYTE % BLOCKSIZE == 0)
    {
        return nbloques / BYTE / BLOCKSIZE;
    }
    else
    {
        return nbloques / BYTE / BLOCKSIZE + 1;
    }
}

/**
 * Función para calcular el tamaño en bloques del array de inodos.
 *
 * @param ninodos Número de inodos del dispositivo virtual.
 * @return Tamaño en bloques del array de inodos.
 */
int tamAI(unsigned int ninodos)
{

    if (ninodos % (BLOCKSIZE / INODOSIZE) == 0)
    {
        return ninodos / (BLOCKSIZE / INODOSIZE);
    }
    else
    {
        return ninodos / (BLOCKSIZE / INODOSIZE) + 1;
    }
}

/**
 * Función para inicializar los datos del superbloque.
 *
 * Llama a: tamMB(), tamAI() y bwrite()
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;

    SB.posPrimerBloqueMB = posSB + tamSB; // posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;
    return EXITO;
}

/**
 * Función para inicializar el mapa de bits poniendo a 1 los bits que representan los metadatos.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initMB()
{
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Número de bloques que ocupan los metadatos
    int metadatos = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

    // Inicializamos bufferMB
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 255, BLOCKSIZE);

    // Miramos la cantidad de bloques que ocupan los bits de los metadatos
    int bloquesMeta = tamMB(metadatos);

    // Si hay más de un bloque, rellenamos los bits de todos los bloques a 1 menos el último
    for (int i = 0; i < bloquesMeta - 1; i++)
    {
        if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO)
            return FALLO;
    }

    memset(bufferMB, 0, BLOCKSIZE);

    // Ponemos los bits necesarios del último bloque a 1
    int numBits1 = metadatos / BYTE;
    for (int i = 0; i < numBits1; i++)
    {
        bufferMB[i] = 255;
    }

    // Si la división no es exacta, añadimos los bits que falten
    char resto = 0;
    for (int i = metadatos % BYTE; i > 0; i--)
    {
        resto += (1 << (BYTE - i));
    }

    bufferMB[numBits1] = resto;

    if (bwrite(SB.posPrimerBloqueMB + bloquesMeta - 1, bufferMB) == FALLO)
        return FALLO;

    // Reducimos la cantidad de bloques libres
    SB.cantBloquesLibres -= bloquesMeta;
    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

    return EXITO;
}

/**
 * Función para inicializar la lista de inodos libres.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initAI()
{
    // 8 inodos (BLOCKSIZE / INODOSIZE) para cada bloque de AI
    // struct inodo inodos(BLOCKSIZE / INODOSIZE)
    // inicialmente, cada inodo apunta al siguiente (el 0 al 1, el 1 al 2, el 2 al 3... hasta 24999)
    // bread (i, inodos). bwrite (i, inodos)

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;

    // Leer el superbloque

    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    int contInodos = SB.posPrimerInodoLibre + 1; // Inicializado posPrimerInodoLibre = 0.

    // Iterar sobre los bloques del array de inodos
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Leer el bloque de inodos desde el dispositio virtual
        if (bread(i, inodos) == FALLO)
            return FALLO;
        // Iterar sobre los inodos dentro del bloque
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l'; // Libre
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos; // Enlazar con el siguiente
                contInodos++;
            }
            else
            { // hemos llegado al último nodo.
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break; // Úlitmo bloque no tiene porque estar completo.
            }
        } // Escribir el bloque de inodos actualizado en el dispositivo
        if (bwrite(i, inodos) == FALLO)
            return FALLO;
    }

    return EXITO;
}

/**
 * Escribe en el bloque indicado por MB seleccionado un bit para indicar si está ocupado (1) o libre (0)
 *
 * @param nbloque Posición de MB que queremos modificar
 * @param bit Valor que se le asignará al bit correspondiente (1 o 0)
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Calculamos donde se ecuentra el bit correspondiente a nbloque en MB
    int posbyte = nbloque / BYTE;
    int posbit = nbloque % BYTE;

    // Miramos donde se encuentra el bit a nivel absoluto
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Inicializamos bufferMB
    unsigned char bufferMB[BLOCKSIZE];

    // Leemos el bloque físico que contiene el bit que queremos modificar
    if (bread(nbloqueabs, bufferMB) == FALLO)
        return FALLO;

    // Obtenemos el byte que contiene el bit a modificar
    posbyte = posbyte % BLOCKSIZE;

    // Modificamos el bit
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara; // OR
    }
    else
    {
        bufferMB[posbyte] &= ~mascara; // AND y NOT
    }

    // Escribimos el bloque físico con el bit modificado
    if (bwrite(nbloqueabs, bufferMB) == FALLO)
        return FALLO;

    return EXITO;
};

char leer_bit(unsigned int nbloque);
int reservar_bloque();
int liberar_bloque(unsigned int nbloque);
int escribir_inodo(unsigned int ninodo, struct inodo *inodo);
int leer_inodo(unsigned int ninodo, struct inodo *inodo);
int reservar_inodo(unsigned char tipo, unsigned char permisos);