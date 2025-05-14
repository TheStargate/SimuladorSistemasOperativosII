/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Muestra todo el contenido de un fichero
 */

#include "directorios.h"
#include <stdlib.h>
#include <string.h>

#define tambuffer BLOCKSIZE * 4

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "Número de argumentos incorrecto. Sintaxis: mi_cat <disco> </ruta_fichero>\n");
        return FALLO;
    }

    char *camino = argv[2];
    char buffer_texto[tambuffer];
    int offset = 0;
    int totalBytesLeidos = 0;

    if (camino[strlen(camino) - 1] == '/')
    {
        fprintf(stderr, RED "Error. El camino debe corresponder a un fichero\n" RESET);
        return FALLO;
    }

    bmount(argv[1]);

    // Limpiamos el buffer
    memset(buffer_texto, 0, tambuffer);
    int leidos = mi_read(camino, buffer_texto, offset, tambuffer);

    // Leemos mientras quede contenido
    while (leidos > 0)
    {
        write(1, buffer_texto, leidos);
        totalBytesLeidos += leidos;
        offset += tambuffer;
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read(camino, buffer_texto, offset, tambuffer);
    }

    fprintf(stderr, "\n\nTotal_leidos " BLUE "%d" RESET, totalBytesLeidos);

    bumount();

}