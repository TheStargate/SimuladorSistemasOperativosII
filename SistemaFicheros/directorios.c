/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación del programa directorios.c que permite crear un directorio y mostrar su contenido
 */
#include "directorios.h"

/**
 * Dada una cadena de caracteres *camino (que comience por '/'), separa su contenido en dos (*inicial y *final)
 * A *tipo se le asignara ‘d’ o ‘f’ en función de si en *inicial hay un nombre de directorio o un nombre de fichero.
 *
 * @param camino Cadena de caracteres que contiene el camino a separar
 * @param inicial Cadena de caracteres que contendrá el nombre del directorio o fichero
 * @param final Cadena de caracteres que contendrá el resto del camino
 * @param tipo Cadena de caracteres que contendrá el tipo de entrada ('d' o 'f')
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Verificar que el camino comienza con '/'
    if (camino == NULL || camino[0] != '/')
    {
        return FALLO;
    }

    // Encontrar la posición del segundo '/'
    const char *segundo_slash = strchr(camino + 1, '/');

    if (segundo_slash == NULL)
    { // No hay segundo '/', es un fichero
        *tipo = 'f';
        strcpy(inicial, camino + 1); // Copiamos todo después del primer '/'
        strcpy(final, "");           // Cadena vacía para final
    }
    else // Hay segundo '/', es un directorio
    {

        *tipo = 'd';

        // Copiar la parte inicial hasta el segundo '/'
        // Vamos iterando con los valores del puntero, como segundo_slash apunta
        // al segundo slash de camino, entonces al ir incrementando llegará un momento que tendrán el mismo valor
        // Ahí se detiene el bucle y se pone el elemento nulo.
        int i = 0;
        while (camino + 1 + i < segundo_slash)
        {
            inicial[i] = camino[1 + i];
            i++;
        }
        inicial[i] = '\0'; // Asegurar terminación nula

        // Copiar el resto (final) a partir del segundo '/'
        strcpy(final, segundo_slash);
    }

    return EXITO;
}

/**
 * Esta función nos buscará una determinada entrada (la parte *inicial del
 * *camino_parcial que nos devuelva extraer_camino()) entre las entradas del
 * inodo correspondiente a su directorio padre (identificado con *p_inodo_dir).
 *
 * @param camino_parcial Cadena de caracteres que contiene el camino parcial a buscar
 * @param p_inodo_dir Puntero al inodo del directorio padre
 * @param p_inodo Puntero al inodo que se busca
 * @param p_entrada Puntero a la entrada que se busca
 * @param reservar Indica si se debe reservar espacio para la entrada
 * @param permisos Permisos de acceso a la entrada
 * @return EXITO si se ha encontrado la entrada, FALLO si no se ha encontrado o ha habido un error.
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    // Definición de variables
    struct entrada entrada;
    struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;

    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Camino parcial es "/"
    if (camino_parcial[0] == '/' && strlen(camino_parcial) == 1) // es el directorio raiz
    {
        *p_inodo = SB.posInodoRaiz; // raiz siempre asociada al inodo 1
        *p_entrada = 0;
        return EXITO;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return FALLO;
    }

#if DEBUGN7
    printf(GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %u]\n" RESET, inicial, final, reservar);
#endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return FALLO;
    }

    // Inicializar el buffer de lectura con ceros
    memset(&entrada, 0, BLOCKSIZE / sizeof(struct entrada));

    // Calcular la cantidad de entradas que puede tener el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // numero de entrada inicial

    if (cant_entradas_inodo > 0)
    {

        // MEJORA BUFFER ENTRADAS

        // Leer el primer bloque de entradas
        if (mi_read_f(*p_inodo_dir, entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE) == FALLO)
        {
            mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
            return FALLO;
        }

        // Buscar la entrada en el bloque leído
        while (num_entrada_inodo < cant_entradas_inodo)
        {
            // Iteramos a través de las entradas del bloque actual
            for (int i = 0; i < BLOCKSIZE / sizeof(struct entrada); i++)
            {
                if (strcmp(entradas[i].nombre, inicial) == 0)
                {
                    entrada = entradas[i];
                    break;
                }
            }

            if (entrada.ninodo != 0) // Si hemos encontrado la entrada
            {
                break;
            }

            // Si no encontramos la entrada, leemos el siguiente bloque
            num_entrada_inodo++;
            memset(entradas, 0, BLOCKSIZE);
            if (mi_read_f(*p_inodo_dir, entradas, num_entrada_inodo * sizeof(struct entrada), BLOCKSIZE) == FALLO)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
                return FALLO;
            }
        }
    }

    // Si la entrada no existe
    if (strcmp(entrada.nombre, inicial) != 0)
    {
        switch (reservar)
        {
        case 0: // modo consulta
            mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
            return FALLO;
        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
                return FALLO;
            }

            // Si es directorio comprobar que tiene permiso de escritura
            if ((inodo_dir.permisos & 2) != 2)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_ESCRITURA);
                return FALLO;
            }
            else
            {
                // Copiamos *inicial en el nombre de la entrada
                strcpy(entrada.nombre, inicial);

                if (tipo == 'd')
                {
                    if (final[0] == '/' && strlen(final) == 1)
                    {
                        // Reservar un inodo como un directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo(tipo, permisos);
                    }
                    else
                    { // No es el final de la ruta
                        mostrar_error_buscar_entrada(ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO);
                        return FALLO;
                    }
                }
                else
                { // Es un fichero
                    // Reservar un inodo como un fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo(tipo, permisos);
                }

#if DEBUGN7
                printf(GRAY "[buscar_entrada()→ reservado inodo %u tipo %c con permisos %u para %s]\n" RESET, entrada.ninodo, tipo, permisos, inicial);
#endif

                // Escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
                { // Error de escritura
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                    }
                    return FALLO;
                }
#if DEBUGN7
                printf(GRAY "[buscar_entrada()→ creada entrada: %s, %u]\n" RESET, inicial, entrada.ninodo);
#endif
            }
        }
    }
    // Si hemos llegado al final del camino
    if ((tipo == 'f' && strlen(final) == 0) || (tipo == 'd' && strlen(final) == 1))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // modo escritura y la entrada ya existe
            mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
            return FALLO;
        }

        // Cortamos la recursividad
        *p_inodo = entrada.ninodo;      // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_entrada = num_entrada_inodo; //  asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        return EXITO;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo; //  asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}

/**
 * Función para mostrar los errores de buscar_entrada()
 */
void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}

/**
 * Función de la capa de directorios que crea un fichero/directorio y su entrada de directorio.
 * Se basa, principalmente, en llamar a la función buscar_entrada() con reservar = 1.
 * (Realmente habría que leer el superbloque para pasarle la posición del inodo del directorio raíz,
 * aunque por simplicidad podríamos suponer directamente que p_inodo_dir es 0).
 *
 * @param camino Cadena de caracteres que contiene el camino a crear
 * @param permisos Permisos de acceso al fichero/directorio
 * @return EXITO si se ha creado correctamente, FALLO si ha habido algún error.
 */
int mi_creat(const char *camino, unsigned char permisos)
{

    unsigned int p_inodo;
    unsigned int p_entrada;

    if (buscar_entrada(camino, 0, &p_inodo, &p_entrada, 1, permisos) == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}

/**
 * Función de la capa de directorios que pone el contenido del directorio en un buffer de memoria
 * (el nombre de cada entrada puede venir separado por '|' o por un tabulador) y devuelve el número de entradas.
 * Implica leer de forma secuencial el contenido de un inodo de tipo directorio, con mi_read_f() leyendo sus entradas.
 * Buscamos la entrada correspondiente a *camino para comprobar que existe y leemos su inodo, comprobando que se trata
 * de un directorio y que tiene permisos de lectura. Para cada entrada concatenamos (mediante la función strcat())
 * su nombre al buffer con un separador.
 *
 * @param camino Cadena de caracteres que contiene el camino a mostrar
 * @param buffer Buffer de memoria donde se almacenará el contenido del directorio
 * @return Número de entradas leídas, o -1 si ha habido un error.
 */
// int mi_dir(const char *camino, char *buffer)
//{
// }

/**
 * Función de la capa de directorios que pone el contenido del directorio en un buffer de memoria
 * (el nombre de cada entrada puede venir separado por '|' o por un tabulador) y devuelve el número de entradas.
 * Implica leer de forma secuencial el contenido de un inodo de tipo directorio, con mi_read_f() leyendo sus entradas.
 * Buscamos la entrada correspondiente a *camino para comprobar que existe y leemos su inodo, comprobando que se trata
 * de un directorio y que tiene permisos de lectura. Para cada entrada concatenamos (mediante la función strcat())
 * su nombre al buffer con un separador.
 *
 * Si queremos ampliar la utilidad de mi_dir() para aplicarla también a ficheros, podemos añadir un parámetro que
 * indique el tipo y que nos lo pasará mi_ls.c, para luego poder comparar la sintaxis con el tipo real del inodo
 * que obtendremos al leer el inodo.
 *
 * @param camino Cadena de caracteres que contiene el camino a mostrar
 * @param buffer Buffer de memoria donde se almacenará el contenido del directorio
 * @param tipo Tipo de entrada ('d' para directorio, 'f' para fichero)
 * @return Número de entradas leídas, o -1 si ha habido un error.
 */
// int mi_dir(const char *camino, char *buffer, char tipo)
//{
// }

/**
 * Función de la capa de directorios que pone el contenido del directorio en un buffer de memoria
 * (el nombre de cada entrada puede venir separado por '|' o por un tabulador) y devuelve el número de entradas.
 * Implica leer de forma secuencial el contenido de un inodo de tipo directorio, con mi_read_f() leyendo sus entradas.
 * Buscamos la entrada correspondiente a *camino para comprobar que existe y leemos su inodo, comprobando que se trata
 * de un directorio y que tiene permisos de lectura. Para cada entrada concatenamos (mediante la función strcat())
 * su nombre al buffer con un separador.
 *
 * Si queremos ampliar la utilidad de mi_dir() para que muestre el listado simple o el extendido (long)
 * según si se usa la opción -l o no en el comando, podemos añadir un parámetro flag, que nos lo pasará
 * mi_ls.c, para luego construir/mostrar un tipo u otro de listado.
 *
 * @param camino Cadena de caracteres que contiene el camino a mostrar
 * @param buffer Buffer de memoria donde se almacenará el contenido del directorio
 * @param tipo Tipo de entrada ('d' para directorio, 'f' para fichero)
 * @param flag Indica si se debe mostrar el listado simple o extendido
 * @return Número de entradas leídas, o -1 si ha habido un error.
 */
// int mi_dir(const char *camino, char *buffer, char flag)
//{
// }

/**
 * Función de la capa de directorios que pone el contenido del directorio en un buffer de memoria
 * (el nombre de cada entrada puede venir separado por '|' o por un tabulador) y devuelve el número de entradas.
 * Implica leer de forma secuencial el contenido de un inodo de tipo directorio, con mi_read_f() leyendo sus entradas.
 * Buscamos la entrada correspondiente a *camino para comprobar que existe y leemos su inodo, comprobando que se trata
 * de un directorio y que tiene permisos de lectura. Para cada entrada concatenamos (mediante la función strcat())
 * su nombre al buffer con un separador.
 * @param camino Cadena de caracteres que contiene el camino a mostrar
 * @param buffer Buffer de memoria donde se almacenará el contenido del directorio
 * @param tipo Tipo de entrada ('d' para directorio, 'f' para fichero)
 * @param flag Indica si se debe mostrar el listado simple o extendido
 * @return Número de entradas leídas, o -1 si ha habido un error.
 */
int mi_dir(const char *camino, char *buffer, char tipo, char flag)
{
    // Nos pasan un camino.
    // Con buscar entrada obtenemos le valor del inodo
    // Luego con mi_read_f, obtenemos info de un inodo pasado por parametro y lo guardamos en un buffer.
    unsigned int p_inodo;
    unsigned int p_entrada;
    unsigned char permisos;
    char tmp[64];

    if (buscar_entrada(camino, 0, &p_inodo, &p_entrada, 0, permisos) == FALLO)
    {
        return FALLO;
    }
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }
    if (!(inodo.permisos & 4) || inodo.tipo != 'd')
    {
        return FALLO;
    }
    if (tipo != inodo.tipo)
    {
        fprintf(stderr, "Error: la sintaxis no concuerda con el tipo\n");
        return FALLO;
    }
    struct entrada buffer_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buffer_entradas, 0, BLOCKSIZE / sizeof(struct entrada));
    int nbytes_leidos = mi_read_f(p_inodo, buffer_entradas, 0, inodo.tamEnBytesLog);
    if (nbytes_leidos == FALLO)
        return FALLO;
    int n = nbytes_leidos / sizeof(struct entrada);
    memset(buffer, 0, sizeof(buffer));
    for (int i = 0; i < n; i++)
    {

        if (leer_inodo(buffer_entradas[i].ninodo, &inodo) == FALLO)
            return FALLO;
        strcat(buffer, inodo.tipo);
        strcat(buffer, "|");
        if (inodo.permisos & 4)
            strcat(buffer, "r");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 2)
            strcat(buffer, "w");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 1)
            strcat(buffer, "x");
        else
            strcat(buffer, "-");
        strcat(buffer, "|");
        struct tm *tm; // ver info: struct tm
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "|");
        strcat(buffer, inodo.tamEnBytesLog);
        strcat(buffer, "|");
        strcat(buffer, buffer_entradas[i].nombre);
        strcat(buffer, "|");
    }
    return n;
}

/**
 * Buscar la entrada *camino con buscar_entrada() para obtener el nº de inodo (p_inodo).
 * Si la entrada existe llamamos a la función mi_chmod_f() de ficheros.c pasándole el p_inodo.
 *
 * @param camino Cadena de caracteres que contiene el camino a modificar
 * @param permisos Permisos de acceso al fichero/directorio
 * @return EXITO si se ha modificado correctamente, FALLO si ha habido algún error.
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
}

/**
 * Buscar la entrada *camino con buscar_entrada() para obtener el p_inodo.
 * Si la entrada existe llamamos a la función mi_stat_f() de ficheros.c pasándole el p_inodo
 *
 * @param camino Cadena de caracteres que contiene el camino a modificar
 * @param p_stat Puntero a la estructura de estado del inodo
 * @return EXITO si se ha modificado correctamente, FALLO si ha habido algún error.
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
}