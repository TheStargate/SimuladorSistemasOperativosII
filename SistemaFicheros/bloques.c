/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar los bloques de un sistema de ficheros
 */

#include "bloques.h"
#include "semaforo_mutex_posix.h"

static sem_t *mutex;
static unsigned int inside_sc = 0;

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

    // Abrimos el fichero con permisos de lectura y escritura
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        perror(RED "open() error");
        printf(RESET);
        return FALLO;
    }

    // Inicializamos el semáforo mutex
    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
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
    // Comprobamos si el descriptor es válido
    if (close(descriptor) == -1)
    {
        perror(RED "close() error");
        printf(RESET);
        return FALLO;
    }

    // Liberamos el semáforo mutex
    deleteSem();

    return EXITO;
}

/**
 * Escribe un bloque en el dispositivo virtual
 *
 * @param nbloque Especifica el bloque físico
 * @param buf Buffer que se volcará en la posición indicada con nbloque
 * @return El numero de bytes escritos (debería ser BLOCKSIZE) o FALLO si ha habido error
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
 * @param buf Buffer que obtendrá los datos de la posición indicada con nbloque
 * @return El numero de bytes leídos (debería ser BLOCKSIZE) o FALLO si ha habido error
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

void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
       waitSem(mutex);
   }
   inside_sc++;
}

void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}
