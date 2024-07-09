#include <stdio.h>

int main()
{
	int i, j;
	int two_dimen[2][2] = {
		{1, 2},
		{3, 4}
	};
	for(i = 0; i < 2; i++){
		for(j = 0; j < 2; j++)
			printf("two_dimen[%d][%d] = %d\n", i, j, two_dimen[i][j]);
	}
	return 0;
}
