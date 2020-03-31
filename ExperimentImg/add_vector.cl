__kernel void test(__global int *pInOut)
{
	int index = get_global_id(0);
	pInOut[index] += 1;
}
