/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que se encarga de inicializar el sistema de archivos
 */

#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    int nbloques;

    bmount(argv[1]);
    nbloques = atoi(argv[2]); // Usamos función atoi para convertir string de consola a entero
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    for (int i = 0; i < nbloques; i++)
    {
        bwrite(i, buffer);
    }

    int ninodos = nbloques / 4;
    initSB(nbloques, ninodos);
    initMB(nbloques, ninodos);
    initAI(nbloques, ninodos);

    reservar_inodo('d',7);

    bumount();
}