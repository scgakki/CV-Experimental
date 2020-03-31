#pragma OPENCL EXTENSION cl_nv_printf:enables
__kernel void medianFilterCL(__global int *In, __global int* Out, int Width, int Height)
{
	int window[9];
	int min;
	int x = get_global_id(0);
	int y = get_global_id(1);
	if (x < Width && x > 0 && y < Height && y > 0)
	{
		window[0] = (y == 0 || x == 0) ? 0 : In[(y - 1)* Width + x - 1];
		window[1] = (y == 0) ? 0 : In[(y - 1)* Width + x];
		window[2] = (y == 0 || x == Width - 1) ? 0 : In[(y - 1)* Width + x + 1];
		window[3] = (x == 0) ? 0 : In[y* Width + x - 1];
		window[4] = In[y* Width + x];
		window[5] = (x == Width - 1) ? 0 : In[y* Width + x + 1];
		window[6] = (y == Height - 1 || x == 0) ? 0 : In[(y + 1)* Width + x - 1];
		window[7] = (y == Height - 1) ? 0 : In[(y + 1)* Width + x];
		window[8] = (y == Height - 1 || x == Width - 1) ? 0 : In[(y + 1)* Width + x + 1];
		for (unsigned int j = 0; j < 5; j++)
		{
			min = j;
			for (unsigned int l = j + 1; l < 9; l++)
				if (window[l] < window[min])
					min = l;
			const float temp = window[j];
			window[j] = window[min];
			window[min] = temp;
		}
		Out[y* Width + x] = window[4];
	}
	//else
	//{
	//	Out[y* Width + x] = 6;
	//}
}