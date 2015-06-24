#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>

int
main(int a, char **b)
{
	int s = (a == 2) ? atoi(b[1]) : 512;
	float *buf = new float[s*s];

	int f = open("sphere.raw", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	for (int i = 0; i < s; i++)
  {
		float x = -1.0 + 2.0 *(i / (s-1.0));

		float *b = buf;
		for (int j = 0; j < s; j++)
		{
			float y = -1.0 + 2.0 *(j / (s-1.0));

			for (int k = 0; k < s; k++)
			{
				float z = -1.0 + 2.0 *(k / (s-1.0));

				*b++ = sqrt(x*x + y*y + z*z);
			}
		}
	
		write(f, buf, s*s*sizeof(float));
	}

	delete[] buf;
	close(f);

	FILE *fd = fopen("sphere.vol", "w");
	fprintf(fd, "%d %d %d float sphere.raw\n", s, s, s);
	fclose(fd);
}

