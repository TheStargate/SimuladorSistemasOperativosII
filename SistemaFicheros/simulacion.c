/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa para simular el funcionamiento del sistema de ficheros con múltiples procesos.
 */

#include "simulacion.h"
#include <sys/wait.h>
#include <signal.h>

int acabados = 0;
char simul_dir[100]; // directorio raíz de simulación

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./simulacion <disco>\n" RESET);
        return FALLO;
    }

    signal(SIGCHLD, reaper);

    if (bmount(argv[1]) == FALLO)
    {
        return FALLO;
    }

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", tm_info);
    sprintf(simul_dir, "/simul_%s/", timestamp);
    if (mi_creat(simul_dir, 6) == FALLO)
    {
        bumount();
        return FALLO;
    }

    fprintf(stderr, "*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, ESCRITURAS);

    for (int i = 0; i < NUMPROCESOS; i++)
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            perror(RED "fork() error");
            printf(RESET);
            return FALLO;
        }

        if (pid == 0)
        {
            if (bmount(argv[1]) == FALLO)
            {
                exit(1);
            }

            char dir_proceso[200];
            char fichero[250];
            pid_t mi_pid = getpid();

            sprintf(dir_proceso, "%sproceso_%d/", simul_dir, mi_pid);
            if (mi_creat(dir_proceso, 6) == FALLO)
            {
                fprintf(stderr, RED "Error al crear directorio del proceso" RESET);
                bumount();
                exit(1);
            }

            sprintf(fichero, "%sprueba.dat", dir_proceso);
            if (mi_creat(fichero, 6) < 0)
            {
                fprintf(stderr, RED "Error al crear prueba.dat" RESET);
                bumount();
                exit(1);
            }

            srand(time(NULL) + mi_pid);

            struct REGISTRO reg;
            for (int j = 1; j <= ESCRITURAS; j++)
            {
                gettimeofday(&reg.fecha, NULL);
                reg.pid = mi_pid;
                reg.nEscritura = j;
                reg.nRegistro = rand() % REGMAX;

                off_t offset = reg.nRegistro * sizeof(struct REGISTRO);

                // fprintf(stdout, "[simulación.c → Escritura %d en %s]\n", reg.nEscritura, fichero);

                if (mi_write(fichero, &reg, offset, sizeof(struct REGISTRO)) == FALLO)
                {
                    fprintf(stderr, RED "Error al escribir en prueba.dat" RESET);
                }

                usleep(50000); // 0,05 s
            }

            fprintf(stderr, GRAY "[Proceso %d: Completadas %d escrituras en %s]\n" RESET, i + 1, ESCRITURAS, fichero);
            bumount();
            exit(0);
        }
        usleep(150000); // 0,15 s
    }

    while (acabados < NUMPROCESOS)
    {
        pause(); // espera a que termine algún hijo
    }

    bumount();
    return 0;
}
