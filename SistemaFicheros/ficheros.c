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
    unsigned char buf_bloque[BLOCKSIZE];
    int nbfisico;
    int bytes_escritos = 0;

    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
        return FALLO;
    }

    // Primer y último bloque lógico
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    // Desplazamiento donde cae el offset y los nbytes escritos a partir del offset
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    if (primerBL == ultimoBL)
    {

        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
            return FALLO;

        bytes_escritos = nbytes;
    }
    else
    {
        // FASE 1
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (nbfisico == FALLO)
            return FALLO;
        if (bread(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        bytes_escritos += BLOCKSIZE - desp1;

        // FASE 2
        for (int bl = primerBL + 1; bl < ultimoBL; bl++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 1);
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE) == FALLO)
                return FALLO;
            bytes_escritos += BLOCKSIZE;
        }

        // FASE 3
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        bytes_escritos += desp2 + 1;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    // Actualizar el tamaño lógico del archivo si hemos escrito más allá del EOF
    if (ultimoBL * BLOCKSIZE + desp2 + 1 > inodo.tamEnBytesLog)
        inodo.tamEnBytesLog = ultimoBL * BLOCKSIZE + desp2 + 1;

    // Actualizar tiempos
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    // Guardar los cambios en el inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    return bytes_escritos;
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
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    int bytesLeidos = 0;
    int nbfisico;

    // Buffer para almacenar el bloque
    unsigned char buf_bloque[BLOCKSIZE];

    // Revisar permisos
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    // Comprobar si el offset es mayor o igual que el tamaño tamEnBytesLog
    if (offset >= inodo.tamEnBytesLog)
    {
        return bytesLeidos;
    }
    // No podemos leer más allá del EOF
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    // Primer y último bloque lógico
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    // Desplazamiento donde cae el offset y los nbytes escritos a partir del offset
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // Contemplar el caso en que el fichero/directorio ocupe un único bloque lógico
    if (primerBL == ultimoBL)
    {

        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;

            memcpy(buf_original, buf_bloque + desp1, nbytes);

            bytesLeidos = nbytes;
        }
    }
    else // Contemplar el caso en que el fichero/directorio ocupe más de un bloque lógico
    {

        // FASE 1
        nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0);
        if (nbfisico != FALLO)
        {

            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        bytesLeidos += BLOCKSIZE - desp1;
        // FASE 2
        for (int bl = primerBL + 1; bl < ultimoBL; bl++)
        {
            nbfisico = traducir_bloque_inodo(ninodo, bl, 0);
            if (nbfisico != FALLO)
            {

                if (bread(nbfisico, buf_bloque) == FALLO)
                    return FALLO;
                memcpy(buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }
            bytesLeidos += BLOCKSIZE;
        }

        // FASE 3
        nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
                return FALLO;
            memcpy(buf_original + (nbytes - (desp2 + 1)), buf_bloque, desp2 + 1);
        }
        bytesLeidos += desp2 + 1;
    }

    inodo.atime = time(NULL);

    return bytesLeidos;
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
    struct inodo inodo;

    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    // Guardamos los datos del inodo en p_stat
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
 * Cambia los permisos de un fichero/directorio (correspondiente al nº de inodo pasado como argumento, ninodo)
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
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    // Actualizar los permisos
    inodo.permisos = permisos;

    // Actualizar ctime
    inodo.ctime = time(NULL);

    // Escribir el inodo modificado
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    return EXITO;
}

/**
 * Trunca un fichero/directorio (correspondiente al nº de inodo, ninodo, pasado como argumento)
 * a los bytes indicados como nbytes, liberando los bloques necesarios. En nuestro sistema de ficheros,
 * esta función será llamada desde la función mi_unlink() de la capa de directorios, la cuál a su vez
 * será llamada desde el programa mi_rm.c, y nos servirá para eliminar una entrada de un directorio.
 *
 * @param ninodo Número de inodo que queremos truncar
 * @param nbytes Número de bytes a los que queremos truncar el fichero/directorio
 * @return Cantidad de bloques liberados o FALLO en caso de error
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    int primerBL, bloques_liberados;
    
    // Leer el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }
    
    // Comprobar permisos de escritura
    if ((inodo.permisos & 2) != 2) {
        return FALLO;
    }
    
    // Comprobar que nbytes no es mayor que el tamaño actual del fichero
    if (nbytes > inodo.tamEnBytesLog) {
        return FALLO;
    }
    
    // Si nbytes es igual al tamaño actual, no hay nada que hacer
    if (nbytes == inodo.tamEnBytesLog) {
        return 0;  // No se liberan bloques
    }
    
    // Calcular el primer bloque lógico a liberar
    if (nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;
    } else {
        primerBL = nbytes / BLOCKSIZE + 1;
    }
    
    // Liberar los bloques a partir de primerBL
    bloques_liberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloques_liberados < 0) {
        return FALLO;
    }
    
    // Actualizar tiempos
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);

    // Actualizar tamEnBytesLog
    inodo.tamEnBytesLog = nbytes;

    // Actualizar el número de bloques ocupados
    inodo.numBloquesOcupados -= bloques_liberados;
    
    // Guardar el inodo actualizado
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }
    
    // Devolver la cantidad de bloques liberados
    return bloques_liberados;
}
