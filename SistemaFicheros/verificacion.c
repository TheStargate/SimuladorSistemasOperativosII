/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa para verificar el funcionamiento del sistema de ficheros con múltiples procesos.
 */

#include "verificacion.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define TAMNOMBRE 60
#define BUFFER_SIZE 500
#define CANT_REGISTROS_BUFFER 256 // 256 * 24 = 6144 bytes = múltiplo de BLOCKSIZE (1024)
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulacion>\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
        return FALLO;

    struct STAT stat;
    if (mi_stat(argv[2], &stat) == FALLO)
    {
        bumount();
        return FALLO;
    }

    int nEntradas = stat.tamEnBytesLog / sizeof(struct entrada);
    if (nEntradas != NUMPROCESOS)
    {
        fprintf(stderr, RED "ERROR: El número de entradas no coincide con NUMPROCESOS\n" RESET);
        bumount();
        return FALLO;
    }

    // Mejora (leemos las entradas previamente al inicio del bucle)
    struct entrada entradas[NUMPROCESOS];
    if (mi_read(argv[2], entradas, 0, sizeof(entradas)) < 0)
    {
        fprintf(stderr, RED "Error leyendo entradas del directorio\n" RESET);
        bumount();
        return FALLO;
    }

    // Creamos el fichero informe.txt dentro del directorio de simulación
    char informe_path[200];
    sprintf(informe_path, "%sinforme.txt", argv[2]);

    if (mi_creat(informe_path, 6) == FALLO)
    {
        fprintf(stderr, RED "Error al crear informe.txt\n" RESET);
        bumount();
        return FALLO;
    }

    // Recorremos las entradas de los procesos
    for (int i = 0; i < NUMPROCESOS; i++)
    {
        struct INFORMACION info = {0};

        // Extraemos el PID a partir del nombre de la entrada
        char *pid_str = strchr(entradas[i].nombre, '_');
        if (!pid_str)
            continue;

        info.pid = atoi(pid_str + 1);

        // Generamos la ruta del fichero prueba.dat del proceso actual
        char path_prueba[200];
        sprintf(path_prueba, "%s%s/prueba.dat", argv[2], entradas[i].nombre);

        int offset = 0;
        int leidos;
        struct REGISTRO buffer_escrituras[CANT_REGISTROS_BUFFER];

        // Recorremos el fichero secuencialmente en bloques
        while ((leidos = mi_read(path_prueba, buffer_escrituras, offset, sizeof(buffer_escrituras))) > 0)
        {
            // Número de registros leidos
            int nregs = leidos / sizeof(struct REGISTRO);
            for (int j = 0; j < nregs; j++)
            {
                struct REGISTRO reg = buffer_escrituras[j];
                // Comprobamos si la escritura es válida mirando el pid
                if (reg.pid != info.pid)
                    continue;

                info.nEscrituras++;

                if (info.nEscrituras == 1)
                {
                    // Guardamos la información del primer registro
                    info.PrimeraEscritura = reg;
                    info.UltimaEscritura = reg;
                    info.MenorPosicion = reg;
                    info.MayorPosicion = reg;
                }
                else
                {
                    // Actualizamos la información según la escritura del registro actual
                    if (reg.nEscritura < info.PrimeraEscritura.nEscritura)
                        info.PrimeraEscritura = reg;

                    if (reg.nEscritura > info.UltimaEscritura.nEscritura)
                        info.UltimaEscritura = reg;

                    if (reg.nRegistro < info.MenorPosicion.nRegistro)
                        info.MenorPosicion = reg;

                    if (reg.nRegistro > info.MayorPosicion.nRegistro)
                        info.MayorPosicion = reg;
                }
            }
            offset += leidos;
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras)); // Limpieza
        }

        fprintf(stderr, GRAY "[%d) %d escrituras validadas en %s]\n" RESET, (i+1), info.nEscrituras, path_prueba);
        
        // Escribir resultados en informe
        char linea[500], fecha[100];

        // Obtenemos tamaño del fichero para empezar a escribir al final
        if (mi_stat(informe_path, &stat) == FALLO)
        {
            bumount();
            return FALLO;
        }
        offset = stat.tamEnBytesLog;

        // Formateamos y escribimos línea a línea actualizando offset
        sprintf(linea, "PID: %d\nNúmero de escrituras: %d\n", info.pid, info.nEscrituras);
        offset += mi_write(informe_path, linea, offset, strlen(linea));

        strftime(fecha, sizeof(fecha), "%a %b %d %T %Y", localtime(&info.PrimeraEscritura.fecha));
        sprintf(linea, "Primera Escritura\t%u\t%u\t%s\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, fecha);
        offset += mi_write(informe_path, linea, offset, strlen(linea));

        strftime(fecha, sizeof(fecha), "%a %b %d %T %Y", localtime(&info.UltimaEscritura.fecha));
        sprintf(linea, "Última Escritura\t%u\t%u\t%s\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, fecha);
        offset += mi_write(informe_path, linea, offset, strlen(linea));

        strftime(fecha, sizeof(fecha), "%a %b %d %T %Y", localtime(&info.MenorPosicion.fecha));
        sprintf(linea, "Menor Posición\t\t%u\t%u\t%s\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, fecha);
        offset += mi_write(informe_path, linea, offset, strlen(linea));

        strftime(fecha, sizeof(fecha), "%a %b %d %T %Y", localtime(&info.MayorPosicion.fecha));
        sprintf(linea, "Mayor Posición\t\t%u\t%u\t%s\n\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, fecha);
        offset += mi_write(informe_path, linea, offset, strlen(linea));
    }

    bumount();
    return EXITO;
}