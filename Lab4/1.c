#include <assert.h>
#include <stdlib.h>
int sum(int *arr, int len)
{
	int i, res=0;
	for(i = 0; i < len ; i++)
	{
		res += arr[i];
	}
	return res;
}
void start(int n)
{
	if( n < 0 ||  n >10){
		printf("Wrong n!\n");
		exit(-1);
	}
	assert(n <= 10 );
	int arr[n];
	int i;
	for(i = 0; i < n; i++)
	{
		arr[i] = i+1;
	}
	int result = sum(arr, n);
	assert(result >= 0);
	assert(result == (n*(n+1))/2);
}
