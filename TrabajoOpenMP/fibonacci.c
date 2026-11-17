#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifndef CANTIDAD
#define CANTIDAD 15 // cuántos números distintos genera el programa
#endif
#ifndef N_MIN
#define N_MIN 1 // valor mínimo (incluido)
#endif
#ifndef N_MAX
#define N_MAX 50 // valor máximo (incluido)
#endif
#ifndef UMBRAL
#define UMBRAL 30 // por encima de este n, fiboPar crea tareas
#endif
#ifndef COMA_DECIMAL
#define COMA_DECIMAL 1 // 1: decimales con coma (Excel en español); 0: con punto
#endif
#define FICHERO_CSV "resultadosFibo.csv"

// Números de hilos con los que se prueba la versión paralela
static const int HILOS[] = {2, 6, 10, 14, 18, 22, 26, 30};
#define N_HILOS ((int)(sizeof(HILOS) / sizeof(HILOS[0])))
// -------------------------------------------------------------------------------------------

double wallclock(void);
long long fibo(int n);
long long fiboPar(int n);
int generaNumeros(int *numeros, int cantidad, int min, int max);
static int comparaInt(const void *a, const void *b);
static void formatea(char *buf, size_t tam, double v);
static void etiquetaFila(char *buf, size_t tam, int fila);
static void imprimeTabla(const int *numeros, double tiempos[][CANTIDAD]);
static int guardaCSV(const char *fichero, const int *numeros, double tiempos[][CANTIDAD]);

int main()
{
    srand(time(NULL));
    omp_set_dynamic(0); // respeta exactamente el nº de hilos pedido

    int numeros[CANTIDAD];
    // tiempos[0] = secuencial; tiempos[1..N_HILOS] = paralelo con HILOS[0..N_HILOS-1]
    static double tiempos[N_HILOS + 1][CANTIDAD];
    long long resSec, resPar;
    double t0, t1;
    int errores = 0;

    if (generaNumeros(numeros, CANTIDAD, N_MIN, N_MAX) != 0)
    {
        fprintf(stderr, "Error: el rango [%d, %d] no tiene %d valores distintos\n", N_MIN, N_MAX, CANTIDAD);
        return 1;
    }
    qsort(numeros, CANTIDAD, sizeof(int), comparaInt);

    printf("Núcleos disponibles: %d | Umbral: n > %d | Números: %d\n\n",
           omp_get_num_procs(), UMBRAL, CANTIDAD);

    for (int i = 0; i < CANTIDAD; i++)
    {
        int n = numeros[i];

        // 1) Secuencial
        t0 = wallclock();
        resSec = fibo(n);
        t1 = wallclock();
        tiempos[0][i] = t1 - t0;

        // 2) Paralelo, con cada número de hilos
        for (int h = 0; h < N_HILOS; h++)
        {
            t0 = wallclock();
#pragma omp parallel num_threads(HILOS[h])
            {
#pragma omp single
                resPar = fiboPar(n);
            }
            t1 = wallclock();
            tiempos[h + 1][i] = t1 - t0;

            if (resPar != resSec)
            {
                errores++;
                fprintf(stderr, "AVISO: resultado distinto con n=%d y %d hilos\n", n, HILOS[h]);
            }
        }

        // Progreso por stderr (los cálculos grandes tardan)
        fprintf(stderr, "[%2d/%d] n=%d listo\n", i + 1, CANTIDAD, n);
    }

    printf("\nTiempos en segundos:\n\n");
    imprimeTabla(numeros, tiempos);

    if (guardaCSV(FICHERO_CSV, numeros, tiempos) == 0)
        printf("\nTabla guardada en %s (separador ';', listo para abrir en Excel)\n", FICHERO_CSV);
    else
        fprintf(stderr, "\nNo se pudo escribir %s\n", FICHERO_CSV);

    if (errores > 0)
        printf("ATENCIÓN: hubo %d resultados paralelos que no coinciden con el secuencial\n", errores);

    return 0;
}

// Convierte un double a texto con 6 decimales (con coma o punto según COMA_DECIMAL)
static void formatea(char *buf, size_t tam, double v)
{
    snprintf(buf, tam, "%.6f", v);
#if COMA_DECIMAL
    for (char *p = buf; *p; p++)
        if (*p == '.')
            *p = ',';
#endif
}

static void etiquetaFila(char *buf, size_t tam, int fila)
{
    if (fila == 0)
        snprintf(buf, tam, "Secuencial");
    else
        snprintf(buf, tam, "%dHilos", HILOS[fila - 1]);
}

// Tabla por pantalla: filas = secuencial/hilos, columnas = valores de n
static void imprimeTabla(const int *numeros, double tiempos[][CANTIDAD])
{
    char txt[32];

    printf("%-11s", "");
    for (int i = 0; i < CANTIDAD; i++)
    {
        snprintf(txt, sizeof(txt), "Valor%d", numeros[i]);
        printf(" %11s", txt);
    }
    printf("\n");

    for (int f = 0; f <= N_HILOS; f++)
    {
        etiquetaFila(txt, sizeof(txt), f);
        printf("%-11s", txt);
        for (int i = 0; i < CANTIDAD; i++)
        {
            char val[32];
            formatea(val, sizeof(val), tiempos[f][i]);
            printf(" %11s", val);
        }
        printf("\n");
    }
}

// Misma tabla en CSV (separador ';')
static int guardaCSV(const char *fichero, const int *numeros, double tiempos[][CANTIDAD])
{
    FILE *fp = fopen(fichero, "w");
    if (fp == NULL)
        return -1;
    char txt[32];

    fprintf(fp, "Configuracion");
    for (int i = 0; i < CANTIDAD; i++)
        fprintf(fp, ";Valor%d", numeros[i]);
    fprintf(fp, "\n");

    for (int f = 0; f <= N_HILOS; f++)
    {
        etiquetaFila(txt, sizeof(txt), f);
        fprintf(fp, "%s", txt);
        for (int i = 0; i < CANTIDAD; i++)
        {
            char val[32];
            formatea(val, sizeof(val), tiempos[f][i]);
            fprintf(fp, ";%s", val);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return 0;
}

static int comparaInt(const void *a, const void *b)
{
    return *(const int *)a - *(const int *)b;
}

// Genera 'cantidad' números distintos en [min, max] (Fisher-Yates parcial)
int generaNumeros(int *numeros, int cantidad, int min, int max)
{
    int rango = max - min + 1;
    if (cantidad > rango)
        return -1;

    int *pool = malloc(rango * sizeof(int));
    if (pool == NULL)
        return -1;

    for (int i = 0; i < rango; i++)
        pool[i] = min + i;

    for (int i = 0; i < cantidad; i++)
    {
        int j = i + rand() % (rango - i);
        int tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
        numeros[i] = pool[i];
    }

    free(pool);
    return 0;
}

long long fibo(int n)
{
    if (n == 0 || n == 1)
    {
        return 1;
    }
    else
    {
        return fibo(n - 1) + fibo(n - 2);
    }
}

long long fiboPar(int n)
{
    if (n == 0 || n == 1)
    {
        return 1;
    }
    else
    {
        if (n > UMBRAL)
        {
            long long a, b;
// Parte del n-1
#pragma omp task shared(a)
            a = fiboPar(n - 1);
// Parte del n-2
#pragma omp task shared(b)
            b = fiboPar(n - 2);
// Espero y finalizo
#pragma omp taskwait
            return a + b;
        }
        else
        {
            return fiboPar(n - 1) + fiboPar(n - 2);
        }
    }
}

double wallclock(void)
{
    struct timeval tv;
    double t;

    gettimeofday(&tv, NULL);

    t = (double)tv.tv_sec;
    t += ((double)tv.tv_usec) / 1000000.0;

    return t;
}