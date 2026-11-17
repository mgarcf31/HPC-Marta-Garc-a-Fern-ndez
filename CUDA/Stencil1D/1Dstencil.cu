#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define RADIUS 3
#define BLOCK_SIZE 256
#define NUM_ELEMENTS (4096 * 2)

static_assert(NUM_ELEMENTS % BLOCK_SIZE == 0,
              "NUM_ELEMENTS debe ser multiplo de BLOCK_SIZE");

// CUDA API error checking macro
#define cudaCheck(call)                                           \
    do                                                            \
    {                                                             \
        cudaError_t err_ = (call);                                \
        if (err_ != cudaSuccess)                                  \
        {                                                         \
            printf("Fatal error: %s at %s:%d\n",                  \
                   cudaGetErrorString(err_), __FILE__, __LINE__); \
            exit(1);                                              \
        }                                                         \
    } while (0)

__global__ void stencil_1d(int *in, int *out)
{
    __shared__ int temp[BLOCK_SIZE + 2 * RADIUS];
    int gindex = blockIdx.x * BLOCK_SIZE + threadIdx.x + RADIUS;
    int lindex = threadIdx.x + RADIUS;

    // Read input elements into shared memory
    temp[lindex] = in[gindex];
    if (threadIdx.x < RADIUS)
    {
        temp[lindex - RADIUS] = in[gindex - RADIUS];
        temp[lindex + BLOCK_SIZE] = in[gindex + BLOCK_SIZE];
    }
    __syncthreads();

    // Apply the stencil
    int result = 0;
    for (int offset = -RADIUS; offset <= RADIUS; offset++)
        result += temp[lindex + offset];

    // Store the result
    out[gindex - RADIUS] = result;
}

int main()
{
    unsigned int i;
    int *h_in = (int *)malloc((NUM_ELEMENTS + 2 * RADIUS) * sizeof(int));
    int *h_out = (int *)malloc(NUM_ELEMENTS * sizeof(int));
    int *d_in, *d_out;

    if (h_in == NULL || h_out == NULL)
    {
        printf("Error: no se pudo reservar memoria en el host\n");
        return 1;
    }

    // Initialize host data
    for (i = 0; i < (NUM_ELEMENTS + 2 * RADIUS); ++i)
        h_in[i] = 1; // With a value of 1 and RADIUS of 3, all output values should be 7

    // Allocate space on the device
    cudaCheck(cudaMalloc((void **)&d_in,
                         (NUM_ELEMENTS + 2 * RADIUS) * sizeof(int)));
    cudaCheck(cudaMalloc((void **)&d_out, NUM_ELEMENTS * sizeof(int)));

    // Copy input data to device
    cudaCheck(cudaMemcpy(d_in, h_in,
                         (NUM_ELEMENTS + 2 * RADIUS) * sizeof(int),
                         cudaMemcpyHostToDevice));

    // Launch kernel and check for errors
    stencil_1d<<<NUM_ELEMENTS / BLOCK_SIZE, BLOCK_SIZE>>>(d_in, d_out);
    cudaCheck(cudaGetLastError());
    cudaCheck(cudaDeviceSynchronize());

    // Copy result back to host
    cudaCheck(cudaMemcpy(h_out, d_out, NUM_ELEMENTS * sizeof(int),
                         cudaMemcpyDeviceToHost));

    for (i = 0; i < 10; ++i)
        printf("h_out[%d] = %d\n", i, h_out[i]);
    // Verify every out value is 7
    for (i = 0; i < NUM_ELEMENTS; ++i)
        if (h_out[i] != 7)
        {
            printf("Element h_out[%d] == %d != 7\n", i, h_out[i]);
            break;
        }

    if (i == NUM_ELEMENTS)
        printf("SUCCESS!\n");

    // Free memory
    cudaFree(d_in);
    cudaFree(d_out);
    free(h_in);
    free(h_out);

    return 0;
}