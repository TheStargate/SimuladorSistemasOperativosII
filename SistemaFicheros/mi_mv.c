#include <stdio.h>
#include "directorios.h"




int main(int argc, char **argv)
{
    if (argc != 4) {
        return FALLO;
    }
     char * disco = argv[1];
     char * caminoAntiguo = argv[2];
     char * caminoNuevo = argv[3];

    if (caminoNuevo[strlen(caminoNuevo)-1] != '/') {
        fprintf(stderr, RED "Error: el destino tiene que ser un directorio\n" RESET);
        return FALLO;
    }
    bmount(disco);

     if (mi_move(caminoAntiguo, caminoNuevo) == FALLO) {
        bumount();
        return FALLO;
     }
     bumount();
     return 0;
}
