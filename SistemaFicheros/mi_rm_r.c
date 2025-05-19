#include <stdio.h>
#include "directorios.h"

int main (int argc, char **argv) {
    
    // Verificamos que se han pasado los 5 argumentos exactamente
	if(argc != 3){
		fprintf(stderr, "Error. Sintaxis correcta: mi_rm <disco> </ruta>\n");
		return FALLO;
	}

    // Comprobamos que el disco y la ruta no son nulos
    char *disco = argv[1];  // Nombre del disco
	char *camino = argv[2]; // Ruta del archivo a borrar

    if (strcmp(camino,"/")== 0){
        fprintf(stderr, "Error: No se puede borrar la raiz del sistema de ficheros!!\n");
        bumount();
        return FALLO;
    }
    if (camino[strlen(camino)-1] == '/') {
        fprintf(stderr, "ERROR: no es un fichero");
    }

    // Montamos el disco
    bmount(disco);

    if (mi_rm_r(camino) == FALLO) {
        fprintf(stderr, "Error al borrar recursivamente %s\n", camino);
        bumount();
        return FALLO;
    }

    // Desmontamos el disco
    bumount();
    
}