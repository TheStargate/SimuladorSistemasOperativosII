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

    if (nbytes < 0)
    {
        liberar_inodo(ninodo);
    }
    else
    {
        mi_truncar_f(ninodo, nbytes);
    }

    bumount();

    return EXITO;
}