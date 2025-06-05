/* semaforo_mutex_posix.c */
#include "semaforo_mutex_posix.h"
#include <stdio.h>
#include <errno.h>

#define RED "\x1b[31m"
#define RESET "\x1b[0m"

#define EXITO 0
#define FALLO -1

static sem_t *g_sem = NULL;

/**
 * Inicializa un semáforo posix nombrado.
 *
 * Si ya existía un semáforo con el mismo nombre, lo elimina primero
 * para asegurar un estado limpio. Luego crea uno nuevo con permisos
 * de lectura/escritura/ejecución para el usuario y valor inicial definido.
 *
 * @return Puntero al semáforo inicializado en caso de éxito, FALLO en caso de error.
 */
sem_t *initSem()
{
    /* Si ya existía, lo eliminamos para garantizar un estado limpio */
    sem_unlink(SEM_NAME);

    /* Creamos el semáforo nombrado con permiso rwx para el usuario */
    g_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, S_IRWXU, SEM_INIT_VALUE);
    if (g_sem == SEM_FAILED)
    {
        fprintf(stderr, RED "Error al crear semáforo\n" RESET);
        return (sem_t *)FALLO;
    }
    return g_sem;
}

/**
 * Cierra y elimina el semáforo posix nombrado.
 *
 * Si el semáforo estaba abierto, lo cierra con sem_close().
 * Después elimina el nombre para que no quede residual en el sistema.
 */
void deleteSem()
{
    if (g_sem)
    {
        if (sem_close(g_sem) < 0)
        {
            fprintf(stderr, RED "Error cerrando semáforo\n" RESET);
        }
        g_sem = NULL;
    }
    
    /* Eliminamos el nombre para que no quede residual en el sistema */
    sem_unlink(SEM_NAME);
}

/**
 * Incrementa (señal) el semáforo indicado.
 *
 * Llama a sem_post() para liberar una unidad del semáforo.
 *
 * @param sem Puntero al semáforo que se desea señalizar.
 */
void signalSem(sem_t *sem)
{
    if (sem_post(sem) < 0)
    {
        fprintf(stderr, RED "Error en sem_post\n" RESET);
    }
}

/**
 * Decrementa (espera) el semáforo indicado.
 *
 * Llama a sem_wait() para decrementar el contador del semáforo,
 * bloqueando si el contador es cero hasta que otra hebra lo señale.
 *
 * @param sem Puntero al semáforo que se desea esperar.
 */
void waitSem(sem_t *sem)
{
    if (sem_wait(sem) < 0)
    {
        fprintf(stderr, RED "Error en sem_wait\n" RESET);
    }
}
