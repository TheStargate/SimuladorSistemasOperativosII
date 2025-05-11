/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que crea un directorio
 */

#include "directorios.h"

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_mkdir <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }

    // Le pasamos por parámetro la ruta (*camino) y los permisos pasados a ints.
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7)
    {
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        bumount();
        return FALLO; // Comprobamos rango adecuado.
    }
    if (mi_creat(argv[3], permisos) == FALLO)
    {
        bumount();
        return FALLO;
    }
    bumount();

    return EXITO;
}
