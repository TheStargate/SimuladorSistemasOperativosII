// Programa de pruebas para cambiar permisos

#include "ficheros.h"

int main(int argc, char **argv)
{
    // Comprobar si el número de argumentos es correcto y si no mostrar sintaxis
    if (argc != 4)
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n" RESET);
        return FALLO;
    }

    bmount(argv[1]);

    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);
    mi_chmod_f(ninodo, permisos);

    bumount();
}