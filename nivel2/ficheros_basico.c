/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar los bloques de un sistema de ficheros
 */

#include "ficheros_basico.h"

struct superbloque SB; // Declaración de la variable global SB

/**
 * Función para calcular el tamaño en bloques necesario para el mapa de bits.
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return Tamaño en bloques necesario para el mapa de bits.
 */
int tamMB(unsigned int nbloques)
{

    if (nbloques % 8 == 0)
    {
        return nbloques / 8;
    }
    else
    {
        return nbloques / 8 + 1;
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
    return tamAI;
}

/**
 * Función para inicializar los datos del superbloque.
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
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
}

/**
 * Función para inicializar el mapa de bits poniendo a 1 los bits que representan los metadatos.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initMB()
{
}

/**
 * Función para inicializar la lista de inodos libres.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initAI()
{
}