// Programa de pruebas para mi_read_f

#include "ficheros.h"

#define tambuffer 1500

int main(int argc, char **argv)
{
    // Comprobar si el número de argumentos es correcto y si no mostrar sintaxis
    if (argc != 3)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./leer <nombre_dispositivo> <ninodo>\n" RESET);
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    char buffer_texto[tambuffer];
    int offset = 0;
    int totalBytesLeidos = 0;
    struct STAT stat;

    bmount(argv[1]);

    memset(buffer_texto, 0, tambuffer);
    int leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    while (leidos > 0)
    {
        write(1, buffer_texto, leidos);
        totalBytesLeidos += leidos;

        offset += BLOCKSIZE; //offset += tambuffer;
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    }

    mi_stat_f(ninodo, &stat);

    fprintf(stderr, "\ntotal_leidos " BLUE "%d" RESET, totalBytesLeidos);
    fprintf(stderr, "\ntamEnBytesLog " BLUE "%d\n" RESET, stat.tamEnBytesLog);

    bumount();
}