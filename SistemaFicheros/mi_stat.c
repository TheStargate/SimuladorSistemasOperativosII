/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que muestra la información acerca del inodo de un fichero o directorio
 */

#include "directorios.h"

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_stat <disco> </ruta>\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }

    struct STAT stat;

    mi_stat(argv[2], &stat);

    bumount();

    return EXITO;
}