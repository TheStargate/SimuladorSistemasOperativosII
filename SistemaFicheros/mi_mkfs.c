#include "bloques.h"
#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    int nbloques;

    bmount(argv[1]);
    nbloques = atoi(argv[2]); // Usamos función atoi para convertir string de consola a entero
    unsigned char array[BLOCKSIZE];
    for (int i = 0; i < nbloques; i++)
    {
        memset(array, 0, BLOCKSIZE);
        bwrite(i, array);
    }
    bumount();
}