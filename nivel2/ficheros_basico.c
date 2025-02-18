/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar los bloques de un sistema de ficheros
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

    return 0;

}

/**
 * Función para calcular el tamaño en bloques del array de inodos.
 * 
 * @param ninodos Número de inodos del dispositivo virtual.
 * @return Tamaño en bloques del array de inodos.
 */
int tamAI(unsigned int ninodos)
{

    return 0;

}

/**
 * Función para inicializar los datos del superbloque.
 * 
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{

    return 0;

}

/**
 * Función para inicializar el mapa de bits poniendo a 1 los bits que representan los metadatos.
 * 
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initMB()
{

    return 0;

}

/**
 * Función para inicializar la lista de inodos libres.
 * 
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initAI()
{

    return 0;

}