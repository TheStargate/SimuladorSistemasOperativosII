#include <stdio.h>
#include "directorios.h"

int main(int argc, char **argv) {

    // Comprobamos que el número de argumentos es 4 exactamente
	if (argc != 4){
	    fprintf(stderr, "Error. Sintaxis correcta: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>");
	    return FALLO;
	}

    // Recuperamos los argumentos
	char *disco = argv[1];
	char *camino1 = argv[2];
	char *camino2 = argv[3];

    // Montamos el disco
	bmount(disco);

    // Intentamos crear el enlace del archivo
	if (mi_link(camino1, camino2) == FALLO){
		fprintf(stderr, "Error al crear el enlace\n");
		return -1;
	}

	printf("Enlace físico creado con éxito\n");

    // Desmontamos el disco
	bumount();

	return EXITO;

}