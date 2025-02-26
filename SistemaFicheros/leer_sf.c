// Programa de pruebas para el sistema de ficheros

#include "ficheros_basico.h"

int main(int argc, char **argv)
{

    bmount(argv[1]);

    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Imprimimos los datos
    printf("DATOS DEL SUPERBLOQUE:\n");
    printf("posPrimerBloqueMB: %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB: %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI: %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos: %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos: %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz: %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %u\n", SB.posPrimerInodoLibre);
    printf(BLUE "cantBloquesLibres: %u\n" RESET, SB.cantBloquesLibres);
    printf("cantInodosLibres: %u\n", SB.cantInodosLibres);
    printf("totBloques: %u\n", SB.totBloques);
    printf("totInodos: %u\n", SB.totInodos);

    printf("\nsizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));

    // Recorrido de la lista de inodos libres
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES:\n");

    int contInodos = SB.posPrimerInodoLibre + 1;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Leer el bloque de inodos desde el dispositio virtual
        if (bread(i, inodos) == FALLO)
            return FALLO;

        // Iterar sobre los inodos dentro del bloque
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            if (contInodos < SB.totInodos)
            {
                printf("%u ", inodos[j].punterosDirectos[0]);
                contInodos++;
            }
            else
            { // hemos llegado al último nodo.
                printf(BLUE "-1\n" RESET);
                break;
            }
        }
    }

    bumount();
}