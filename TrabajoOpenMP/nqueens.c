#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define MAX_N 32

int solutions = 0;
int mysolutions = 0;

// Variable privada para cada hilo (crea una copia persistente por hilo)
#pragma omp threadprivate(mysolutions)

// Función para comprobar si una ubicación de reina es válida
static int valid_queen(int j, const int *state)
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

#define X 4 // profundidad hasta la que se crean tareas

void nqueens(int n, int j, int *state)
{
    if (n == j)
    {
        mysolutions++;
        return;
    }
    for (int i = 0; i < n; i++)
    {
        int *new_state = alloca(sizeof(int) * n);
        memcpy(new_state, state, sizeof(int) * n);
        new_state[j] = i;

        if (!valid_queen(j + 1, new_state))
            continue;

        if (j < X)
        {
#pragma omp task firstprivate(new_state, j, n)
            nqueens(n, j + 1, new_state); // en paralelo
        }
        else
        {
            nqueens(n, j + 1, new_state); // secuencial, sin crear tarea
        }
    }
#pragma omp taskwait
}

void search(int n)
{
#pragma omp parallel // Inicia la región paralela
    {
#pragma omp single // Solo un hilo ejecuta la siguiente sección
        {
            int initial_state[n];
            nqueens(n, 0, initial_state);
        }
#pragma omp atomic
        solutions += mysolutions;
    }
}

int main(int argc, char **argv)
{
    int n = (argc > 1) ? atoi(argv[1]) : 10;
    // int n = 8;
    double start_time = omp_get_wtime();
    double end_time;
    search(n);
    end_time = omp_get_wtime();

    printf("Número de soluciones para %d reinas: %d\n", n, solutions);
    printf("Tiempo de ejecución: %f segundos\n", end_time - start_time);

    return 0;
}