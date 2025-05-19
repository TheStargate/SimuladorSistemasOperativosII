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

    leer_inodo(*p_inodo_dir, &inodo_dir);
    if (inodo_dir.permisos < 4)
    {
#if DEBUGN7
        printf(GRAY "[buscar_entrada()→ El inodo %u no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
#endif
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
        int bloque_actual = 0;
        int entrada_encontrada = 0;

        // Buscar la entrada en el bloque leído
        while (num_entrada_inodo < cant_entradas_inodo && !entrada_encontrada)
        {
            // Leemos el bloque actual
            if (mi_read_f(*p_inodo_dir, entradas, bloque_actual * BLOCKSIZE, BLOCKSIZE) == FALLO)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
                return FALLO;
            }

            // Iteramos a través de las entradas del bloque actual
            for (int i = 0; i < BLOCKSIZE / sizeof(struct entrada) && num_entrada_inodo < cant_entradas_inodo; i++)
            {
                if (strcmp(entradas[i].nombre, inicial) == 0)
                {
                    entrada = entradas[i];
                    entrada_encontrada = 1;
                    break;
                }
                num_entrada_inodo++;
            }

            // Si no encontramos la entrada, pasamos al siguiente bloque
            bloque_actual++;
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

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    return buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
}

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
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    unsigned char permisos = 4;
    char tmp[64];

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos) == FALLO)
    {
        return FALLO;
    }
    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }
    if (!(inodo.permisos & 4))
    {
        return FALLO;
    }
    if (tipo != inodo.tipo)
    {
        fprintf(stderr, "Error: la sintaxis no concuerda con el tipo\n");
        return FALLO;
    }

    struct entrada buffer_entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(buffer_entradas, 0, sizeof(buffer_entradas));
    int n;

    if (inodo.tipo == 'f')
    {
        // Simular una sola entrada si es un fichero
        if (mi_read_f(p_inodo_dir, buffer_entradas, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            return FALLO;
        n = 1;
    }
    else
    {
        int nbytes_leidos = mi_read_f(p_inodo, buffer_entradas, 0, inodo.tamEnBytesLog);
        if (nbytes_leidos == FALLO)
            return FALLO;
        n = nbytes_leidos / sizeof(struct entrada);
    }

    for (int i = 0; i < n; i++)
    {
        if (leer_inodo(buffer_entradas[i].ninodo, &inodo) == FALLO)
            return FALLO;
        char color[20];
        if (inodo.tipo == 'd')
        {                              // Directorio: azul
            strcpy(color, "\033[34m"); // Azul
        }
        else if (inodo.tipo == 'f') // Archivo: verde
        {
            strcpy(color, "\033[32m"); // Verde
        }
        sprintf(tmp, "%c", inodo.tipo);
        strcat(buffer, tmp);
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

        struct tm *tm;
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "|");

        sprintf(tmp, "%u", inodo.tamEnBytesLog);
        strcat(buffer, tmp);
        strcat(buffer, "|");
        strcat(buffer, color); // Añadir color
        strcat(buffer, buffer_entradas[i].nombre);
        strcat(buffer, "\033[0m"); // Resetear color
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

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos) == FALLO)
    {
        return FALLO;
    }
    return mi_chmod_f(p_inodo, permisos);
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
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos) == FALLO)
    {
        return FALLO;
    }

    fprintf(stderr, BLUE "\nNº de inodo: %d\n" RESET, p_inodo);

    return mi_stat_f(p_inodo, p_stat);
}

// Guarda las últimas entradas para escritura (caché LRU)
static struct UltimaEntrada UltimasEntradasE[CACHE_SIZE];

// Guarda las últimas entradas para lectura (caché LRU)
static struct UltimaEntrada UltimasEntradasL[CACHE_SIZE];

/**
 * Función de directorios.c para escribir contenido en un fichero. Buscaremos
 * la entrada camino con buscar_entrada() para obtener el p_inodo. Si la entrada
 * existe llamamos a la función correspondiente de ficheros.c pasándole el p_inodo
 *
 * @param camino Cadena de caracteres que contiene el camino a modificar
 * @param buf Buffer de memoria donde se almacenará el contenido a escribir
 * @param offset Desplazamiento para escritura
 * @param nbytes Número de bytes a escribir
 * @return Número de bytes escritos
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int bytesEscritos;
    int lru_index = 0;
    int entradaEnCache = FALLO;

    // Comprobamos si encontramos el camino actual en la caché
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (strcmp(UltimasEntradasE[i].camino, camino) == 0)
        {
            // Si coincide, usamos el inodo almacenado en la caché
            p_inodo = UltimasEntradasE[i].p_inodo;

            // Actualizar tiempo de acceso
            gettimeofday(&UltimasEntradasE[i].ultima_consulta, NULL);

            lru_index = i;
            entradaEnCache = EXITO;
        }
    }

    if (entradaEnCache == FALLO)
    {
        // Si no está en la caché, buscamos la entrada del camino a modificar
        if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
        {
            return FALLO;
        }

        // Actualizamos la caché LRU con la nueva entrada encontrada
        struct timeval tiempoActual;
        gettimeofday(&tiempoActual, NULL);

        // Buscar la entrada usada hace más tiempo para substituirla
        for (int i = 0; i < CACHE_SIZE; i++)
        {
            if (timercmp(&UltimasEntradasE[i].ultima_consulta, &UltimasEntradasE[lru_index].ultima_consulta, <))
            {
                lru_index = i;
            }
        }

#if DEBUGN9
        fprintf(stderr, ORANGE "[mi_write() → Reemplazamos cache[%d]: %s]\n" RESET, lru_index, camino);
#endif

        strcpy(UltimasEntradasE[lru_index].camino, camino);
        UltimasEntradasE[lru_index].p_inodo = p_inodo;
        UltimasEntradasE[lru_index].ultima_consulta = tiempoActual;
    }
    else
    {
#if DEBUGN9
        fprintf(stderr, BLUE "\n[mi_write() → Utilizamos cache[%d]: %s]\n" RESET, lru_index, camino);
#endif
    }

    // Escribimos los datos en el fichero
    if ((bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes)) == FALLO)
    {
        return FALLO;
    }

    return bytesEscritos;
}

/**
 * Función de directorios.c para leer los nbytes del fichero indicado por camino,
 * a partir del offset pasado por parámetro y copiarlos en el buffer buf. Buscaremos
 * la entrada camino con buscar_entrada() para obtener el p_inodo. Si la entrada existe
 * llamamos a la función correspondiente de ficheros.c pasándole el p_inodo
 *
 * @param camino Cadena de caracteres que contiene el camino a modificar
 * @param buf Buffer de memoria donde se almacenará el contenido a leer
 * @param offset Desplazamiento para lectura
 * @param nbytes Número de bytes a leer
 * @return Número de bytes leídos.
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int bytesLeidos;
    int lru_index = 0;
    int entradaEnCache = FALLO;

    // Comprobamos si encontramos el camino actual en la caché
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if (strcmp(UltimasEntradasL[i].camino, camino) == 0)
        {
            // Si coincide, usamos el inodo almacenado en la caché
            p_inodo = UltimasEntradasL[i].p_inodo;

            // Actualizar tiempo de acceso
            gettimeofday(&UltimasEntradasL[i].ultima_consulta, NULL);

            lru_index = i;
            entradaEnCache = EXITO;
        }
    }

    if (entradaEnCache == FALLO)
    {
        // Si no está en la caché, buscamos la entrada del camino a modificar
        if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
        {
            return FALLO;
        }

// Actualizamos la caché LRU con la nueva entrada encontrada
#if DEBUGN9
        fprintf(stderr, ORANGE "[mi_read() → Reemplazamos cache[%d]: %s]\n" RESET, lru_index, camino);
#endif

        struct timeval tiempoActual;
        gettimeofday(&tiempoActual, NULL);

        // Buscar la entrada usada hace más tiempo para substituirla
        for (int i = 0; i < CACHE_SIZE; i++)
        {
            if (timercmp(&UltimasEntradasL[i].ultima_consulta, &UltimasEntradasL[lru_index].ultima_consulta, <))
            {
                lru_index = i;
            }
        }

        strcpy(UltimasEntradasL[lru_index].camino, camino);
        UltimasEntradasL[lru_index].p_inodo = p_inodo;
        UltimasEntradasL[lru_index].ultima_consulta = tiempoActual;
    }
    else
    {
#if DEBUGN9
        fprintf(stderr, BLUE "\n[mi_read() → Utilizamos cache[%d]: %s]\n" RESET, lru_index, camino);
#endif
    }

    // Escribimos los datos en el fichero
    if ((bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes)) == FALLO)
    {
        return FALLO;
    }

    return bytesLeidos;
}

/**
 * Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1.
 *
 * @param camino1 Cadena de caracteres que contiene el camino del inodo
 * @param camino2 Cadena de caracteres que contiene el camino del enlace
 * @return EXITO si se ha creado correctamente, FALLO si ha habido algún error.
 */
int mi_link(const char *camino1, const char *camino2)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;

    struct inodo inodo1;
    struct entrada entrada;

    if (buscar_entrada(camino1, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
    {
        return FALLO;
    }

    leer_inodo(p_inodo, &inodo1);

    // Comprobamos que tenemos permisos de lectura
    if (inodo1.permisos < 4)
    {
        return FALLO;
    }

    // Comprobamos que el camino sea a un fichero
    if (inodo1.tipo != 'f')
    {
        return FALLO;
    }

    // Creamos la entrada de camino2
    if (buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6) == FALLO)
    {
        return FALLO;
    }

    // Leemos la entrada de camino2
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    // Creamos el enlace
    entrada.ninodo = p_inodo;
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    // Liberamos el inodo asociado a la entrada creada
    liberar_inodo(p_inodo2);

    inodo1.nlinks++;
    inodo1.ctime = time(NULL);

    escribir_inodo(p_inodo, &inodo1);

    return EXITO;
}

/**
 * Función de la capa de directorios que borra la entrada de directorio especificada
 * (no hay que olvidar actualizar la cantidad de enlaces en el inodo) y, en caso de que
 * fuera el último enlace existente, borrar el propio fichero/directorio. Es decir tanto
 * para borrar un enlace a un fichero como para eliminar un fichero o directorio que no
 * contenga enlaces.
 *
 * @param camino Cadena de caracteres que contiene el camino a eliminar
 * @return EXITO si se ha eliminado correctamente, FALLO si ha habido algún error.
 */
int mi_unlink(const char *camino)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    struct inodo inodo;
    struct inodo inodo_dir;

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
    {
        return FALLO;
    }

    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)
    {

#if DEBUGN10
        fprintf(stderr, RED "Error: el directorio %s no está vacío\n" RESET, camino);
#endif

        return FALLO; // No se puede borrar porque no está vacío.
    }
    else
    {
        if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
        {
            return FALLO;
        }
        unsigned int n_Entradas_Dir = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

        if (p_entrada == n_Entradas_Dir - 1)
        { // Le restamos el tamaño de una entrada para posicionarnos justo encima de la últime entrada
            if (mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada)) == FALLO)
            {
                return FALLO;
            }
        }
        else
        {
            struct entrada ultimaEntrada;
            // Leemos la última entrada ya que es la que borraremos con mi_truncar_f pero los datos de la última entrada no son los que queremos borrar.
            if (mi_read_f(p_inodo_dir, &ultimaEntrada, inodo_dir.tamEnBytesLog - sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            {
                return FALLO;
            }
            // Escribimos en la entrada que queríamos eliminar la ultima entrada y luego eliminamos la última entrada.
            if (mi_write_f(p_inodo_dir, &ultimaEntrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            {
                return FALLO;
            }

            if (mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada)) == FALLO)
            {
                return FALLO;
            }
        }
        inodo.nlinks--;
        if (inodo.nlinks == 0)
        {
            if (liberar_inodo(p_inodo) == FALLO)
            {
                return FALLO;
            }
        }
        else
        {
            inodo.ctime = time(NULL);
            if (escribir_inodo(p_inodo, &inodo) == FALLO)
                return FALLO;
        }
    }
    return EXITO;
}

// Extras

int mi_rename(const char *camino, const char *nombreNuevo)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    struct inodo inodo_dir;
    struct entrada entrada;

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
    {
        return FALLO;
    }
    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
    {
        return FALLO;
    }
    // Comprueba si existe el nombre ya
    if (comprob_nuevoNombre(p_inodo_dir, inodo_dir, nombreNuevo) == FALLO)
    {
#if DEBUGNEXT
        fprintf(stderr, RED "Error: La entrada ya existe\n" RESET);
#endif
        return FALLO;
    }

    memset(&entrada, 0, BLOCKSIZE / sizeof(struct entrada));

    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }

    strcpy(entrada.nombre, nombreNuevo);
    if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}

int comprob_nuevoNombre(int ninodo, struct inodo inodo, const char *nombreNuevo)
{

    struct entrada entrada;
    int nEntrada = 0;
    memset(&entrada, 0, BLOCKSIZE / sizeof(struct entrada));
    while (nEntrada < inodo.tamEnBytesLog / sizeof(struct entrada))
    {
        if (mi_read_f(ninodo, &entrada, nEntrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
        {
            fprintf(stderr, "[DEBUG] mi_read_f falló en la entrada %d\n", nEntrada);
            return FALLO;
        }
        

        fprintf(stderr, "NombreNuevo: %s\n", entrada.nombre);
        fprintf(stderr, "NombreFicherantiguo: %s\n", nombreNuevo);
        if (strcmp(entrada.nombre, nombreNuevo) == 0)
        {   
            
            return FALLO;
        }
        nEntrada++;
    }
    return EXITO;
}

int mi_move(const char *camino, const char *caminoNuevo)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    unsigned int p_inodo_dir_dest = 0;
    unsigned int p_inodo_dest = 0;
    unsigned int p_entrada_dest = 0;

    struct inodo inodo_dir;
    struct inodo inodo_dest;
    struct entrada entrada;


    //Leemos directorio actual  

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
    {
        fprintf(stderr,"FALLO1");
        return FALLO;
    }
    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr,"FALLO2");
        return FALLO;
    }
    memset(&entrada, 0, BLOCKSIZE / sizeof(struct entrada));
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        fprintf(stderr,"FALLO3");
        return FALLO;
    } //Ahora obtenemos nombre del archivo a mover.


    // Leemos directorio destino
    if (buscar_entrada(caminoNuevo, &p_inodo_dir_dest, &p_inodo_dest, &p_entrada_dest, 0, 7) == FALLO)
    {
        fprintf(stderr,"FALLO4");
        return FALLO;
    }
    if (leer_inodo(p_inodo_dest, &inodo_dest) == FALLO)
    {
        fprintf(stderr,"FALLO5");
        return FALLO;
    }

    fprintf(stderr, "%s\n", entrada.nombre);
    char * nombreArchivo = entrada.nombre;
    if (comprob_nuevoNombre(p_inodo_dest, inodo_dest, nombreArchivo) == FALLO)
    {
        
#if DEBUGNEXT
        fprintf(stderr, RED "Error: La entrada ya existe\n" RESET);
#endif
        return FALLO;
    }
    if (mi_unlink(camino) == FALLO) {
        fprintf(stderr,"FALLO6");
        return FALLO;
    }

   /* if (mi_creat(caminoNuevo, 7) == FALLO) {
        fprintf(stderr,"FALLO7");
        return FALLO;
    } */
    if (buscar_entrada(caminoNuevo, &p_inodo_dir_dest, &p_inodo_dest, &p_entrada_dest, 0, 7) == FALLO)
    {
        fprintf(stderr,"FALLO8");
        return FALLO;
    }
    if (leer_inodo(p_inodo_dest, &inodo_dest) == FALLO)
    {
        fprintf(stderr,"FALLO9");
        return FALLO;
    }
    
    if (mi_write_f(p_inodo_dest, &entrada, p_entrada_dest * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        fprintf(stderr,"FALLO10");
        return FALLO;
    } 

    

return EXITO;
    // Leemos directorio destino
}

int mi_rm_r(const char *camino) {
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    struct inodo inodo;

    // Obtener inodo de 'camino'
    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO) {
        return FALLO;
    }
    if (leer_inodo(p_inodo, &inodo) == FALLO) {
        return FALLO;
    }

    // Si es directorio, recorrer entradas
    if (inodo.tipo == 'd') {
        int nentradas = inodo.tamEnBytesLog / sizeof(struct entrada);
        struct entrada ent;
        for (int i = 0; i < nentradas; i++) {
            if (mi_read_f(p_inodo, &ent, i * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
                return FALLO;
            }
            // Saltar "." y ".."
            if (strcmp(ent.nombre, ".") != 0 && strcmp(ent.nombre, "..") != 0) {
                // Construir nuevo camino
                char nuevo_camino[strlen(camino) + 1 + strlen(ent.nombre) + 1];
                sprintf(nuevo_camino, "%s/%s", camino, ent.nombre);
                // Recursión
                if (mi_rm_r(nuevo_camino) == FALLO) {
                    return FALLO;
                }
            }
        }
    }

    // Borrar el fichero o directorio (ahora vacío)
    if (mi_unlink(camino) == FALLO) {
        return FALLO;
    }

    return EXITO;
}
