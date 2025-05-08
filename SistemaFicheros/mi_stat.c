/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que muestra la información acerca del inodo de un fichero o directorio
 */

#include "directorios.h"

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_stat <disco> </ruta>\n" RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }

    struct STAT stat;

    mi_stat(argv[2], &stat);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];

    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("Tipo: %c\nPermisos: %d\natime: %s\nmtime: %s\nctime: %s\nbtime: %s\nnlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n", stat.tipo, stat.permisos, atime, mtime, ctime, btime, stat.nlinks, stat.tamEnBytesLog, stat.numBloquesOcupados);

    bumount();

    return EXITO;
}