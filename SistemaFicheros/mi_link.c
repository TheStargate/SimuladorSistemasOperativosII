/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que crea un enlace a un fichero
 */

#include <stdio.h>
#include "directorios.h"

int main(int argc, char **argv) {

	if (argc != 4){
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n" RESET);
	    return FALLO;
	}

	char *camino1 = argv[2];
	char *camino2 = argv[3];

	bmount(argv[1]);

    // Intentamos crear el enlace del archivo
	if (mi_link(camino1, camino2) == FALLO){
		bumount();
		return FALLO;
	}

	bumount();

}