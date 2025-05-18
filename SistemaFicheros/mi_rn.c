#include <stdio.h>
#include "directorios.h"




int main(int argc, char const *argv[])
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
    //Para evitar segment fault

    char caminoModif [TAMNOMBRE]; //Camino modificable
    strcpy (caminoModif, caminoAntiguo);
    const char * delimitador = "/";
    char *token = strtok(caminoModif, delimitador);
    char *ultimo;
    while (token != NULL) {
    ultimo = token;
    token = strtok(NULL, delimitador);
  }
    //En último queda almacenado el token que se correspondería al nombre del archivo antiguo.
    if (strcmp (nombreNuevo, ultimo) == 0) {
        fprintf(stderr, "Error: La entrada ya existe", RESET);
        return FALLO;
    }
    


    bmount(disco);

     if (mi_rename(caminoAntiguo, nombreNuevo) == FALLO) {
        bumount();
        return FALLO;
     }
     bumount();
     return 0;
}
