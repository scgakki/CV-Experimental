#ifndef  __TEST_KERNEL_CU_
#define  __TEST_KERNEL_CU_
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <time.h>
#include <iostream>
#define datasize 100

inline void checkCudaErrors(cudaError err) //cuda error handle function
{
	if (cudaSuccess != err)
	{
		fprintf(stderr, "CUDA Runtime API error:%s.\n", cudaGetErrorString(err));
		return;
	}
}

__global__ void add(int *c, int *a, int *b)
{
	int i = threadIdx.x;
	c[i] = a[i] + b[i];
}

extern "C" void add_host(int *host_a, int *host_b, int *host_c)
{
	int *dev_a, *dev_b, *dev_c;
	checkCudaErrors(cudaMalloc((void**)&dev_a, sizeof(int)* datasize));
	checkCudaErrors(cudaMalloc((void**)&dev_b, sizeof(int)* datasize));
	checkCudaErrors(cudaMalloc((void**)&dev_c, sizeof(int)* datasize));

	checkCudaErrors(cudaMemcpy(dev_a, host_a, sizeof(int)* datasize, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(dev_b, host_b, sizeof(int)* datasize, cudaMemcpyHostToDevice));

	add<< <1, datasize >> >(dev_c, dev_a, dev_b);

	checkCudaErrors(cudaMemcpy(host_c, dev_c, sizeof(int)* datasize, cudaMemcpyDeviceToHost));
	cudaFree(dev_a);//«Â¿Ìœ‘ø®ƒ⁄¥Ê  
	cudaFree(dev_b);
	cudaFree(dev_c);
	
}

#endif // ! __TEST_KERNEL_CU_