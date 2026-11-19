#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#define f(A) (4.0 / (1.0 + (A) * (A)))

#ifndef N
#define N 10000000
#endif
const int n = N;

int main(int argc, char **argv)
{
    double w, sum, pi;
    int i_outer;
    double sum_outer[100][64]; /* una fila (= una cache line distinta) por tarea */
    clock_t t1, t2;
    struct timeval tv1, tv2;
    struct timezone tz;
#ifdef _OPENMP
    double wt1, wt2;
#endif

#ifdef _OPENMP
#pragma omp parallel
    {
#pragma omp single
        printf("OpenMP-parallel with %1d threads\n", omp_get_num_threads());
    } /* end omp parallel */
#endif

    gettimeofday(&tv1, &tz);
#ifdef _OPENMP
    wt1 = omp_get_wtime();
#endif
    t1 = clock();

    /* calculate pi = integral [0..1] 4/(1+x**2) dx */
    w = 1.0 / n;
    sum = 0.0;
    for (i_outer = 0; i_outer < 100; i_outer++)
        sum_outer[i_outer][0] = 0.0;

/*
Un solo hilo genera las tareas; todo el equipo las ejecuta. taskloop reparte las 100 iteraciones de i_outer en tareas. grainsize(1) -> 1 iteracion externa por tarea (100 tareas de ~n/100 pasos cada una).
taskloop lleva un taskgroup implicito: no se sale de el hasta que todas
las tareas han terminado, asi que la suma posterior es segura.
*/
#pragma omp parallel
#pragma omp single
#pragma omp taskloop grainsize(1)
    for (i_outer = 0; i_outer < 100; i_outer++)
    {
        double local = 0.0; /* i, x y local son privadas de cada tarea */
        for (int i = i_outer + 1; i <= n; i += 100)
        {
            double x = w * ((double)i - 0.5);
            local += f(x);
        }
        sum_outer[i_outer][0] = local;
    }

    for (i_outer = 0; i_outer < 100; i_outer++)
        sum += sum_outer[i_outer][0];
    pi = w * sum;

    t2 = clock();
#ifdef _OPENMP
    wt2 = omp_get_wtime();
#endif
    gettimeofday(&tv2, &tz);
    printf("computed pi = %24.16g\n", pi);
    printf("CPU time (clock)                = %12.4g sec\n", (t2 - t1) / 1000000.0);
#ifdef _OPENMP
    printf("wall clock time (omp_get_wtime) = %12.4g sec\n", wt2 - wt1);
#endif
    printf("wall clock time (gettimeofday)  = %12.4g sec\n", (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec) * 1e-6);
    return 0;
}