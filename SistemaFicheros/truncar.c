/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa para truncar un fichero/directorio a un tamaño específico
 */

#include "ficheros.h"

int main(int argc, char **argv)
{
    // Comprobar si el número de argumentos es correcto y si no mostrar sintaxis
    if (argc != 4)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    bmount(argv[1]);

    if (nbytes == 0)
    {
        liberar_inodo(ninodo);
    }
    else
    {
        mi_truncar_f(ninodo, nbytes);
    }

    struct STAT stat;
    mi_stat_f(ninodo, &stat);

    fprintf(stderr, "\nDATOS INODO 1:\n");

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