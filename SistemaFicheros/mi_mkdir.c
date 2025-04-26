#include "directorios.h"

int main(int argc, char const *argv[])
{
    if (argc != 4) // Escriben más o menos cosas de las que deberían por consola.
    {
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }

    // Le pasamos por parámetro la ruta (*camino) y los permisos pasados a ints.
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7)
    {
        bumount();
        return FALLO; // Comprobamos rango adecuado.
    }
    if (mi_creat(argv[3], permisos) == FALLO)
    {
        bumount();
        return FALLO;
    }
    bumount();

    return EXITO;
}
