/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar los bloques de un sistema de ficheros
 */

#include "bloques.h"

static int descriptor = 0; // Descriptor del fichero

/**
 * Función para montar el dispositivo virtual (abrir el fichero)
 *
 * @param camino Ruta / nombre del fichero
 * @return El descriptor del archivo o FALLO si ha habido error al abrirlo
 */
int bmount(const char *camino)
{

    umask(000); // Permisos rw para todos los usuarios
    
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        perror(RED "open() error");
        printf(RESET);
        return FALLO;
    }
    return descriptor;
}

/**
 * Función para desmontar el dispositivo virtual (liberar el descriptor del fichero)
 *
 * @return EXITO si ha ido bien o FALLO si ha habido error
 */
int bumount()
{

    if (close(descriptor) == -1)
    {
        perror(RED "close() error");
        printf(RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Escribe un bloque en el dispositivo virtual
 *
 * @param nbloque Especifica el bloque físico
 * @param buf Buffer que se volcará en la posición indicada con nbloque
 * @return El numbero de bytes escritos (debería ser BLOCKSIZE) o FALLO si ha habido error
 */
int bwrite(unsigned int nbloque, const void *buf)
{

    if (lseek(descriptor, (nbloque * BLOCKSIZE), SEEK_SET) == -1)
    {
        perror(RED "lseek() error");
        printf(RESET);
        return FALLO;
    }

    int nbytes = write(descriptor, buf, BLOCKSIZE);

    if (nbytes == -1)
    {
        perror(RED "write() error");
        printf(RESET);
        return FALLO;
    }

    return nbytes;
}

/**
 * Lee un bloque en el dispositivo virtual
 *
 * @param nbloque Especifica el bloque físico
 * @param buf Buffer que se volcará en la posición indicada con nbloque
 * @return El numbero de bytes leídos (debería ser BLOCKSIZE) o FALLO si ha habido error
 */
int bread(unsigned int nbloque, void *buf)
{

    if (lseek(descriptor, (nbloque * BLOCKSIZE), SEEK_SET) == -1)
    {
        perror(RED "lseek() error");
        printf(RESET);
        return FALLO;
    }

    int nbytes = read(descriptor, buf, BLOCKSIZE);

    if (nbytes == -1)
    {
        perror(RED "read() error");
        printf(RESET);
        return FALLO;
    }

    return nbytes;
}