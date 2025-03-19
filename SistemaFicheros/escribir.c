// Programa de pruebas para mi_write_f

#include "ficheros.h"

#define numOffsets 5 // Cantidad de offsets a probar

int main(int argc, char **argv)
{
    // Comprobar si el número de argumentos es correcto y si no mostrar sintaxis
    if (argc != 4)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>" RESET);
        return FALLO;
    }

    int diferentes_inodos = atoi(argv[3]);
    int offset[] = {9000, 209000, 30725000, 409605000, 480000000};
    int ninodo[numOffsets];
    int tambuffer = strlen(argv[2]) * sizeof(char);
    char *buffer_texto = argv[2];
    struct STAT stat[5];

    bmount(argv[1]);

    printf("longitud texto: %lu\n", strlen(argv[2]));

    if (diferentes_inodos == 0)
    { // Un inodo para todos los offset
        ninodo[0] = reservar_inodo('f', 6);
        for (int i = 1; i < numOffsets; i++)
        {
            ninodo[i] = ninodo[0];
        }
    }
    else
    { // Un inodo diferente para cada offset
        for (int i = 0; i < numOffsets; i++)
        {
            ninodo[i] = reservar_inodo('f', 6);
        }
    }

    for (int i = 0; i < numOffsets; i++)
    {
        printf("Nº inodo reservado: %d", ninodo[i]);
        printf("offset: %d", offset[i]);
        // Escribimos el fichero
        int bytesEscritos = mi_write_f(ninodo[i], buffer_texto, offset[i], tambuffer);
        mi_stat_f(ninodo[i], &stat[i]);
        printf("Bytes escritos: %d", bytesEscritos);
        printf("stat.tamEnBytesLog=%d", stat->tamEnBytesLog);
        printf("stat.numBloquesOcupados=%d", stat->numBloquesOcupados);

        // (ELIMINAR DESPUÉS DE TESTEAR)
        memset(buffer_texto, 0, tambuffer);
        int bytesLeidos = mi_read_f(ninodo[i], buffer_texto, offset[i], tambuffer);
        write(1, buffer_texto, tambuffer);
    }

    bumount();
}