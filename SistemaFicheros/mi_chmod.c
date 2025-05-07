/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Cambia los permisos de un fichero o directorio, llamando a la función mi_chmod() de la capa de directorios.
 */

#include "directorios.h"

int main(int argc, char **argv)
{

    if(argc != 4)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
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
        bumount();
        return FALLO; // Comprobamos rango adecuado.
    }
    
    int permisos = argv[2];
    const char *camino = argv[3];

    mi_chmod(camino, permisos);
    
    bumount();
}