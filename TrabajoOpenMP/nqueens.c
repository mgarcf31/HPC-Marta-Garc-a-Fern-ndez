/*
 * Escribe un programa nqueens.c que resuelva el problema de las n
 * reinas usando tareas de OpenMP.
 *     - Ejecuta la aplicación con distinto número de procesadores,
 *       compara los resultados y evalúa el rendimiento.
 *     - Realiza una gráfica que refleje los distintos tiempos para las
 *       distintas configuraciones.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_N 32

long long solutions_parallel = 0;
int mysolutions = 0;

// Variable privada para cada hilo (crea una copia persistente por hilo)
#pragma omp threadprivate(mysolutions)

// Función para comprobar si una ubicación de reina es válida
static int ok(int j, const int *state)
{
    int r = j - 1;
    for (int k = 0; k < r; k++)
    {
        int d = state[r] - state[k];
        if (d == 0 || d == r - k || d == k - r)
        {
            return 0;
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// VERSIÓN SECUENCIAL (Algoritmo base de las transparencias)
// ---------------------------------------------------------------------------
static long nqueens_seq(int n, int j, int *state)
{
    if (j == n)
        return 1;

    long count = 0;
    for (int i = 0; i < n; i++)
    {
        state[j] = i;
        if (ok(j + 1, state))
        {
            count += nqueens_seq(n, j + 1, state);
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
// VERSIÓN PARALELA CON TAREAS OPENMP
// ---------------------------------------------------------------------------
void nqueens_parallel(int n, int j, int *state)
{
    if (n == j)
    {
        mysolutions++; // Incrementa el contador de soluciones local del hilo
        return;
    }

    for (int i = 0; i < n; i++)
    {
#pragma omp task
        {
            // Reserva y copia explícita de memoria en el heap (malloc)
            // para evitar que los datos de la pila queden fuera de ámbito
            int *new_state = (int *)malloc(n * sizeof(int));
            memcpy(new_state, state, n * sizeof(int));
            new_state[j] = i;

            if (ok(j + 1, new_state))
            {
                nqueens_parallel(n, j + 1, new_state);
            }
            free(new_state);
        }
    }

// Sincroniza y espera a que terminen las tareas hijas directas de esta llamada
#pragma omp taskwait
}

void start_search_parallel(int n)
{
#pragma omp parallel
    {
        mysolutions = 0; // Inicializa el contador privado para el hilo actual

#pragma omp single
        {
            int *initial_state = (int *)malloc(n * sizeof(int));
            nqueens_parallel(n, 0, initial_state);
            free(initial_state);
        }

// Suma atómica de los resultados de cada hilo al total global
#pragma omp atomic
        solutions_parallel += mysolutions;
    }
}

// ---------------------------------------------------------------------------
// FUNCIÓN PRINCIPAL
// ---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int n = 12; // Valor predeterminado de N

    if (argc > 1)
    {
        n = atoi(argv[1]);
    }

    if (n > MAX_N)
    {
        printf("N no puede ser mayor que %d\n", MAX_N);
        return 1;
    }

    printf("=== EVALUACIÓN DE RENDIMIENTO N-QUEENS (N = %d) ===\n\n", n);

    // 1. Medición de la versión Secuencial
    int *state_seq = (int *)malloc(n * sizeof(int));
    double t0_seq = omp_get_wtime();
    long long sol_seq = nqueens_seq(n, 0, state_seq);
    double t1_seq = omp_get_wtime();
    free(state_seq);

    double time_seq = t1_seq - t0_seq;
    printf("[SECUENCIAL]\n");
    printf("  Soluciones encontradas : %lld\n", sol_seq);
    printf("  Tiempo de ejecucion    : %f segundos\n\n", time_seq);

    // 2. Medición de la versión Paralela con Tareas
    double t0_par = omp_get_wtime();
    start_search_parallel(n);
    double t1_par = omp_get_wtime();

    double time_par = t1_par - t0_par;
    printf("[PARALELO - OpenMP Tasks]\n");
    printf("  Hilos (OMP_NUM_THREADS): %d\n", omp_get_max_threads());
    printf("  Soluciones encontradas : %lld\n", solutions_parallel);
    printf("  Tiempo de ejecucion    : %f segundos\n\n", time_par);

    // Cálculo y muestra del Speedup
    if (time_par > 0)
    {
        printf("Aceleración (Speedup S_p): %.2fx\n", time_seq / time_par);
    }

    return 0;
}