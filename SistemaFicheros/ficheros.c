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
    }
    else
    {
        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO)
            ;
        return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
            return FALLO;
    }

    return 7;
}

// Esto enteoria es un mi_write_f que funciona
/**
 * int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {

    //Declaraciones
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    int nbfisico;
    char unsigned buf_bloque[BLOCKSIZE];

    int bytesEscritos = 0; //Leva el recuento de bytes escritos realmente

    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Comprobamos que tenga los permisos para escribir
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr, "No hay permisos de escritura\n");
        return FALLO;
    }

    nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if(nbfisico == -1) {
        return FALLO;
    }

    if(bread(nbfisico, buf_bloque) == -1){
        return FALLO;
    }

    //Caso en el que el buffer cabe en un bloque fisico
    if(primerBL == ultimoBL) {

        memcpy(buf_bloque + desp1, buf_original, nbytes);

        bytesEscritos = nbytes;

    } else { //Caso en el que la escritura ocupa mas de un bloque

        //1. Primer bloque lógico
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        bytesEscritos = BLOCKSIZE - desp1;

        if(bwrite(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }


        //2. Bloques lógicos intermedios
        for(int i = primerBL + 1; i < ultimoBL; i++) {

            nbfisico = traducir_bloque_inodo(&inodo, i, 1);
            if(nbfisico == -1) {
                return FALLO;
            }

            if(bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) == -1) {
                return FALLO;
            }

            bytesEscritos += BLOCKSIZE;
        }

        //3. Último bloque lógico
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        if(nbfisico == -1) {
            return FALLO;
        }

        if(bread(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }

        memcpy(buf_bloque, buf_original + (nbytes - (desp2 - 1)), desp2 + 1);

        bytesEscritos += desp2 + 1;
    }

    if(bwrite(nbfisico, buf_bloque) == -1) {
        return FALLO;
    }

    //Comprobar si hemos escrito más allá del final del fichero
    if(inodo.tamEnBytesLog < (ultimoBL * BLOCKSIZE + desp2 + 1)) {
        inodo.tamEnBytesLog = ultimoBL * BLOCKSIZE + desp2 + 1;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == FALLO) {
        return FALLO;
    }

    return bytesEscritos;
}
 */

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

    // Primer y último bloque lógico
    int primerBL = offset / BLOCKSIZE;
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    // Buffer para almacenar el bloque
    unsigned char buf_bloque[BLOCKSIZE];

    // Desplazamiento donde cae el offset y los nbytes escritos a partir del offset
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // Revisar permisos
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    // Comprobar si el offset es mayor o igual que el tamaño tamEnBytesLog
    if (offset >= inodo.tamEnBytesLog)
    {
        return 0;
    }
    else if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        return inodo.tamEnBytesLog - offset;
    }

    // Contemplar el caso en que el fichero/directorio ocupe un único bloque lógico
    if (primerBL == ultimoBL)
    {

        unsigned char buf_bloque[BLOCKSIZE];

        int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1);
        if (bread(nbfisico, buf_bloque) == FALLO)
            return FALLO;
        memcpy(buf_bloque + desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
            return FALLO;
    }
    else
    {
    }
}

// Esto enteoria es un mi_read_f que funciona
/**
 * int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {

    //Declaraciones
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int bytesLeidos = 0;

    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];

    //Leemos inodo
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Compobamos permisos
    if((inodo.permisos & 4) != 4) {
        fprintf(stderr, "No hay permisos de lectura\n");
        return FALLO;
    }

    if(offset >= inodo.tamEnBytesLog) {
        return bytesLeidos;
    }

    if(offset + nbytes >= inodo.tamEnBytesLog) {  //pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        //leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtenemos el numero de bloque fisico
    nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
    if(nbfisico != -1) {
        if(bread(nbfisico, buf_bloque) == -1) {
            return FALLO;
        }
    }

    if(primerBL == ultimoBL) {
        if(nbfisico != -1) {
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        bytesLeidos = nbytes;

    } else {

        if(nbfisico != -1) {
            memcpy (buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytesLeidos = BLOCKSIZE - desp1;

        for(int i = primerBL + 1; i < ultimoBL; i++) {

            nbfisico = traducir_bloque_inodo(&inodo, i, 0);
            if(nbfisico != -1) {
                if (bread(nbfisico, buf_bloque) == -1) { // error
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }

            bytesLeidos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if(nbfisico != -1) {
            if(bread(nbfisico, buf_bloque) == -1) {
                return FALLO;
            }
            memcpy(buf_original + (nbytes - (desp2 + 1)), buf_bloque, desp2 + 1);
        }

        bytesLeidos += desp2 + 1;
    }

    inodo.atime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    return bytesLeidos;

}
 */

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