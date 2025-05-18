#include <stdio.h>
#include "directorios.h"




int main(int argc, char **argv)
{
    if (argc != 4) {
        return FALLO;
    }
     char * disco = argv[1];
     char * caminoAntiguo = argv[2];
     char * nombreNuevo = argv[3];

    if ((caminoAntiguo[strlen(caminoAntiguo)-1] != '/' && nombreNuevo[strlen(nombreNuevo)-1] == '/') || (caminoAntiguo[strlen(caminoAntiguo)-1] == '/' && nombreNuevo[strlen(nombreNuevo)-1] != '/') ) {
        fprintf(stderr, "ERROR: los archivos deben ser del mismo tipo");
    }

    bmount(disco);

     if (mi_rename(caminoAntiguo, nombreNuevo) == FALLO) {
        bumount();
        return FALLO;
     }
     bumount();
     return 0;
}
