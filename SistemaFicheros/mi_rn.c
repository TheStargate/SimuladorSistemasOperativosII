/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que renombra un fichero o directorio
 */

#include <stdio.h>
#include "directorios.h"

int main(int argc, char **argv)
{

    if (argc != 4) {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n" RESET);
        return FALLO;
    }
     char * disco = argv[1];
     char * caminoAntiguo = argv[2];
     char * nombreNuevo = argv[3];

    bmount(disco);

     if (mi_rename(caminoAntiguo, nombreNuevo) == FALLO) {
        bumount();
        return FALLO;
     }
     bumount();
}
