#include <stdio.h>
#include "directorios.h"

int main(int argc, char **argv)
{

    if (argc != 3)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: mi_rmdir <disco> </ruta>\n" RESET);
        return FALLO;
    }

    char *disco = argv[1];
    char *camino = argv[2];

    if (strcmp(camino, "/") == 0)
    {
        fprintf(stderr, RED "ERROR: No se puede borrar la raiz del sistema de ficheros\n" RESET);
        return FALLO;
    }

    if (camino[strlen(camino) - 1] != '/')
    {
        fprintf(stderr, RED "ERROR: no es un directorio" RESET);
    }

    bmount(disco);

    // Intenta borrar el archivo
    if (mi_unlink(camino) == FALLO)
    {
        // fprintf(stderr, "Error al borrar el archivo\n");
        bumount();
        return FALLO;
    }

    // printf("Archivo borrado correctamente\n");

    bumount();
}