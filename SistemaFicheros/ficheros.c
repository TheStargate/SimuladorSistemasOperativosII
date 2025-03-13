/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar un sistema de ficheros
 */

#include "ficheros.h"

/**
 * Escribe el contenido procedente de un buffer de memoria, buf_original, de tamaño nbytes,
 * en un fichero/directorio (correspondiente al inodo pasado como argumento, ninodo).
 *
 * @param ninodo Número de inodo que queremos leer
 * @param buf_original Puntero al buffer de memoria que contiene los datos a escribir
 * @param offset Desplazamiento en el fichero/directorio donde queremos escribir
 * @param nbytes Número de bytes a escribir
 * @return Número de bytes escritos realmente
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
        return FALLO;
    }
    int primerBL = offset/BLOCKSIZE;
    int ultimoBL = (offset +nbytes-1) / BLOCKSIZE;

    //Desplazamiento donde cae el offset y los nbytes escritos a partir del offset
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes -1 ) % BLOCKSIZE;


    return 7;
}

/**
 * Lee información de un fichero/directorio (correspondiente al nº de inodo, ninodo, pasado
 * como argumento) y la almacena en un buffer de memoria.
 *
 * @param ninodo Número de inodo que queremos leer
 * @param buf_original Puntero al buffer de memoria donde queremos almacenar los datos leídos
 * @param offset Desplazamiento en el fichero/directorio donde queremos leer
 * @param nbytes Número de bytes a leer
 * @return Número de bytes leídos realmente
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    return 7;
}

/**
 * Devuelve la metainformación de un fichero/directorio (correspondiente al nº de inodo pasado
 * como argumento).
 *
 * @param ninodo Número de inodo que queremos leer
 * @param p_stat Puntero a la estructura STAT donde queremos volcar la información
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    mi_read_f(ninodo, );

    struct inodo inodo;

    leer_inodo(ninodo, &inodo);

    p_stat->atime = inodo.atime;
    p_stat->btime = inodo.btime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->permisos = inodo.permisos;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->tipo = inodo.tipo;
    
    return EXITO;
}

/**
 * Cambia los permisos de un fichero/directorio (correspondiente al nº de inodo pasado como argumento, ninodo) con el valor que indique el argumento permisos. * 
 * con el valor que indique el argumento permisos.
 *
 * @param ninodo Número de inodo que queremos leer
 * @param permisos Permisos que queremos asignar al fichero/directorio
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, "Error en mi_chmod_f: No se pudo leer el inodo %u\n", ninodo);
        return FALLO;
    }
    
    // Actualizar los permisos
    inodo.permisos = permisos;
    
    // Actualizar ctime (tiempo de cambio)
    inodo.ctime = time(NULL);
    
    // Escribir el inodo modificado
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, "Error en mi_chmod_f: No se pudo escribir el inodo %u\n", ninodo);
        return FALLO;
    }
    
    return EXITO;
}