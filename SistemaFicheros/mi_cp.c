/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que copia un fichero o directorio
 */

#include <stdio.h>
#include "directorios.h"



int main(int argc, char **argv)
{
    if (argc != 4)
        return FALLO;

    char *disco = argv[1];
    char *origen = argv[2];
    char *destino = argv[3];

    if (destino[strlen(destino) - 1] != '/')
        return FALLO;

    if (bmount(disco) == FALLO)
        return FALLO;

    int res = mi_cp(origen, destino);

    bumount();

    return res;
}
