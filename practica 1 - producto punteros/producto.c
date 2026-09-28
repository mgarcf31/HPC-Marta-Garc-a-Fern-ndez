#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

double wallclock(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + ((double)tv.tv_usec) / 1000000.0;
}

int main()
{
    int tamanos[] = {7, 20, 50, 100, 250, 500};
    int num_tamanos = sizeof(tamanos) / sizeof(tamanos[0]);

    srand(time(NULL));

    for (int k = 0; k < num_tamanos; k++)
    {
        int N = tamanos[k];
        double inicio;

        int *a = (int *)malloc(N * N * sizeof(int));
        int *b = (int *)malloc(N * N * sizeof(int));
        int *c_est_temp = (int *)malloc(N * N * sizeof(int));
        int *c_est_dir = (int *)malloc(N * N * sizeof(int));
        int *c_din_temp = (int *)malloc(N * N * sizeof(int));
        int *c_din_dir = (int *)malloc(N * N * sizeof(int));

        for (int i = 0; i < N * N; i++)
        {
            a[i] = rand() % 10;
            b[i] = rand() % 10;
        }

        // Estático con temp
        inicio = wallclock();
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                int temp = 0;
                for (int t = 0; t < N; t++)
                    temp += a[i * N + t] * b[t * N + j];
                c_est_temp[i * N + j] = temp;
            }
        }
        double t_est_temp = wallclock() - inicio;

        // Estático sin temp
        inicio = wallclock();
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                c_est_dir[i * N + j] = 0;
                for (int t = 0; t < N; t++)
                    c_est_dir[i * N + j] += a[i * N + t] * b[t * N + j];
            }
        }
        double t_est_dir = wallclock() - inicio;

        // Dinámico con temp
        inicio = wallclock();
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                int temp = 0;
                for (int t = 0; t < N; t++)
                    temp += a[i * N + t] * b[t * N + j];
                c_din_temp[i * N + j] = temp;
            }
        }
        double t_din_temp = wallclock() - inicio;

        // Dinámico sin temp
        inicio = wallclock();
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                c_din_dir[i * N + j] = 0;
                for (int t = 0; t < N; t++)
                    c_din_dir[i * N + j] += a[i * N + t] * b[t * N + j];
            }
        }
        double t_din_dir = wallclock() - inicio;

        printf("%dx%d: %.8f %.8f %.8f %.8f\n", N, N, t_est_temp, t_est_dir, t_din_temp, t_din_dir);

        free(a);
        free(b);
        free(c_est_temp);
        free(c_est_dir);
        free(c_din_temp);
        free(c_din_dir);
    }

    return 0;
}