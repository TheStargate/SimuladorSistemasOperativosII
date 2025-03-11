// Programa de pruebas para el sistema de ficheros

#include "ficheros_basico.h"
#define DEBUGN2 0
#define DEBUGN3 1

int main(int argc, char **argv)
{
    bmount(argv[1]);

    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

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

#if DEBUGN2

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
            { // hemos llegado al último inodo.
                printf(BLUE "-1\n" RESET);
                break;
            }
        }
    }
#endif

#if DEBUGN3

    // Reservar y liberar bloque
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");

    int numBloque = reservar_bloque();
    printf("Se ha reservado el bloque físico nº%u que era el 1º libre indicado por el MB:\n", numBloque);
    printf("SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);

    liberar_bloque(numBloque);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);

    // Mostrar el MB
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS");
    // printf(GRAY "[leer_bit(0)→ posbyte:%u, posbyte (ajustado): %u, posbit:%u, nbloqueMB:%u, nbloqueabs:%u)]" RESET);
    // printf("posSB: %u → leer_bit(0) = %u", posSB, leer_bit(0));

    // Mostrar el inodo del directorio raíz
    printf("\nDATOS DEL DIRECTORIO RAIZ\n");

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];

    struct inodo inodo;
    leer_inodo(SB.posInodoRaiz, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(ctime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("Tipo: %c\nPermisos: %d\natime: %s\nmtime: %s\nctime: %s\nbtime: %s\nnlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n", inodo.tipo, inodo.permisos, atime, mtime, ctime, btime, inodo.nlinks, inodo.tamEnBytesLog, inodo.numBloquesOcupados);

#endif

    bumount();
}