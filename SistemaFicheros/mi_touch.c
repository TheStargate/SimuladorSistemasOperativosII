/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que crea un fichero
 */

#include "directorios.h"

int main(int argc, char const *argv[])
{

    unsigned int permisos = atoi(argv[2]);
    const char *camino = argv[3];
    if (argc != 4)
    {
        return FALLO;
    }

    if (permisos < 0 || permisos > 7)
    {
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }
    if (camino[strlen(camino) - 1] == '/')
    {
        return FALLO;
    }
    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    } 

    if (mi_creat(camino, permisos) == FALLO)
    {
        bumount();
        return FALLO;
    }

   return 0;
}
