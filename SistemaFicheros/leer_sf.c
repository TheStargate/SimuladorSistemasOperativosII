/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación del programa leer_sf.c para leer el superbloque y mostrar su contenido.
 */

#include "directorios.h"

int main(int argc, char **argv)
{
    bmount(argv[1]);

    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    printf("\nDATOS DEL SUPERBLOQUE:\n");
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

#if DEBUGN2

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

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    printf("Se ha reservado el bloque físico nº%u que era el 1º libre indicado por el MB:\n", numBloque);
    printf("SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);

    liberar_bloque(numBloque);

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %u\n", SB.cantBloquesLibres);

    // Mostrar el MB
    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS");
    printf("\nposSB: %u → leer_bit(%u) = %d", posSB, posSB, leer_bit(posSB));
    printf("\nSB.posPrimerBloqueMB: %u → leer_bit(%u) = %d", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    printf("\nSB.posUltimoBloqueMB: %u → leer_bit(%u) = %d", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf("\nSB.posPrimerBloqueAI: %u → leer_bit(%u) = %d", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf("\nSB.posUltimoBloqueAI: %u → leer_bit(%u) = %d", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf("\nSB.posPrimerBloqueDatos: %u → leer_bit(%u) = %d", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf("\nSB.posUltimoBloqueDatos: %u → leer_bit(%u) = %d", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

    // Mostrar el inodo del directorio raíz
    printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");

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
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("Tipo: %c\nPermisos: %d\natime: %s\nmtime: %s\nctime: %s\nbtime: %s\nnlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n", inodo.tipo, inodo.permisos, atime, mtime, ctime, btime, inodo.nlinks, inodo.tamEnBytesLog, inodo.numBloquesOcupados);

#endif

#if DEBUGN4

    // Traducción de bloques lógicos
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");

    int ninodo = reservar_inodo('f', 6);

    traducir_bloque_inodo(ninodo, 8, 1);
    printf("\n");
    traducir_bloque_inodo(ninodo, 204, 1);
    printf("\n");
    traducir_bloque_inodo(ninodo, 30004, 1);
    printf("\n");
    traducir_bloque_inodo(ninodo, 400004, 1);
    printf("\n");
    traducir_bloque_inodo(ninodo, 468750, 1);

    printf("\n\nDATOS DEL INODO RESERVADO\n");

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];

    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("Tipo: %c\nPermisos: %d\natime: %s\nmtime: %s\nctime: %s\nbtime: %s\nnlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n", inodo.tipo, inodo.permisos, atime, mtime, ctime, btime, inodo.nlinks, inodo.tamEnBytesLog, inodo.numBloquesOcupados);

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    printf(BLUE "\nSB.posPrimerInodoLibre: %u\n" RESET, SB.posPrimerInodoLibre);

#endif

#if DEBUGN7
    // Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1);           // ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0);          // ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1);     // ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1);          // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1);     // creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
    // ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1);          // ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); // consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1);    // creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); // creamos /pruebas/docs/doc2

#endif

    bumount();
}

void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}
