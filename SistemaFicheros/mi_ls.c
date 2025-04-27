
#include <directorios.h>
#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000)

int main(int argc, char const *argv[])
{
    const char *camino;
    char buffer[TAMBUFFER];
    char flag, tipo;
    unsigned int total_entradas;

    if (argc == 3)
    {
        camino = argv[2]; // Simple
        flag = 's';
    }
    else if (argc == 4)
    {
        camino = argv[3]; // Long
        flag = 'l';
    }
    else
    {
        return FALLO;
    }

    if (camino[strlen(camino) - 1] != '/')
    { // Fichero
        tipo = 'f';
        total_entradas = mi_dir(camino, buffer, tipo, flag);
        
    }
    else
    {
        tipo = 'd';
        total_entradas = mi_dir(camino, buffer, tipo, flag);
    }
    char *token;
    char separador[] = "|";
    if (total_entradas > 0)
    {
        printf("Tipo\tPermisos\tmTime\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------------------\n");
        if (tipo == 'd')
        {
            printf("Total: %d\n", total_entradas);
        }

        token = strtok(buffer, separador);
        while (token != NULL)
        {
            printf("%c\t", token[0]); // Primer campo: tipo
            token = strtok(NULL, separador);

            printf("%s\t", token); // Segundo campo: permisos
            token = strtok(NULL, separador);

            printf("%s\t", token); // Tercer campo: mtime
            token = strtok(NULL, separador);

            printf("%s\t", token); // Cuarto campo: tamaño
            token = strtok(NULL, separador);

            printf("%s\n", token);           // Quinto campo: nombre
            token = strtok(NULL, separador); // Avanzar a la siguiente entrada
        }
    }

    return 0;
}
