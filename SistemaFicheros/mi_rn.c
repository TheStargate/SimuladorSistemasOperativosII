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

    bmount(disco);

     if (mi_rename(caminoAntiguo, nombreNuevo) == FALLO) {
        bumount();
        return FALLO;
     }
     bumount();
     return 0;
}
