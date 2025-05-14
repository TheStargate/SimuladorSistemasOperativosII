/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa para escribir texto en un fichero dentro de un sistema de ficheros personalizado
 */

#include "directorios.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

    // Verificamos que se hayan proporcionado exactamente 4 argumentos (más el nombre del programa)
    if (argc != 5)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    int offset = atoi(argv[4]); // Posición desde donde comenzar a escribir
    char *camino = argv[2];     // Ruta del fichero donde escribir
    int tambuffer = strlen(argv[3]) * sizeof(char);
    char *buffer_texto = argv[3]; // Texto a escribir en el fichero
    int bytesEscritos;

    if (camino[strlen(camino) - 1] == '/')
    {
      fprintf(stderr, RED "Error: la ruta se corresponde a un directorio.\n" RESET);
      exit(-1);
    }
    
    bmount(argv[1]);

    printf("longitud texto: %lu\n", strlen(argv[3]));

    // Intentamos escribir el texto en el fichero especificado
    if ((bytesEscritos = mi_write(camino, buffer_texto, offset, tambuffer)) == FALLO)
    {
        bytesEscritos = 0;
    }
    printf("Bytes escritos: %d\n", bytesEscritos);

    bumount();

}