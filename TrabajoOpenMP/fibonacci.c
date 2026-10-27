// EJERCICIOS OPENMP - Fibonacci.
// Escribe un programa fibo.c que resuelva el problema de Fibonacci de forma recursiva usando tareas de OpenMP.
// Ejecuta la aplicación con distinto número de procesadores, compara el resultado y analiza el rendimiento.
// Realiza una gráfica que refleje los distintos tiempos para las distintas configuraciones.

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Versión secuencial de Fibonacci (referencia)
static long long fib_seq(int n)
{
    if (n < 2)
        return 1; // Convención del boletín/transparencias
    else
        return fib_seq(n - 1) + fib_seq(n - 2);
}

// Versión paralela de Fibonacci usando tareas de OpenMP
static long long fib_task(int n)
{
    long long x, y;
    if (n < 2)
        return 1;

    // Si n < 20, grano fino secuencial para evitar sobrecostes
    if (n < 20)
        return fib_seq(n);

#pragma omp task shared(x) firstprivate(n)
    x = fib_task(n - 1);

#pragma omp task shared(y) firstprivate(n)
    y = fib_task(n - 2);

#pragma omp taskwait // Esperamos a las dos tareas hijas
    return x + y;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Uso: %s <n> <num_hilos>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int num_hilos = atoi(argv[2]);
    long long result;

    // Establecer el número de hilos pasados por parámetro
    omp_set_num_threads(num_hilos);

    double t0 = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single
        {
            result = fib_task(n);
        }
    }

    double t = omp_get_wtime() - t0;

    printf("Fibonacci(%d) = %lld\n", n, result);
    printf("Hilos: %d | Tiempo: %.6f segundos\n", num_hilos, t);

    // Línea de salida CSV para capturar resultados en scripts
    printf("CSV,%d,%d,%.6f,%lld\n", n, num_hilos, t, result); // n= numero de Fibonacci, num_hilos= número de hilos, t= tiempo de ejecución, result= resultado de Fibonacci(n)

    return 0;
}