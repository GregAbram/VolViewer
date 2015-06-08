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
	std::cerr << f << "\n";

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

	FILE *fd = fopen("sphere.osp", "w");
	fprintf(fd, "<?xml version=\"1.0\"?>\n");
	fprintf(fd, "<volume name=\"volume\">\n");
 	fprintf(fd, "<dimensions> %d %d %d </dimensions>\n", s, s, s);
 	fprintf(fd, "<voxelType> float </voxelType>\n");
 	fprintf(fd, "<samplingRate> 1.0 </samplingRate>\n");
 	fprintf(fd, "<filename> sphere.raw </filename>\n");
	fprintf(fd, "</volume>\n");
	fclose(fd);

	fd = fopen("sphere.vol", "w");
	fprintf(fd, "%d %d %d float sphere.raw\n", s, s, s);
	fclose(fd);

	fd = fopen("sphere.state", "w");
	fprintf(fd, "sphere.vol\n");
	fprintf(fd, "pos %f %f %f\n", (s-1)/2.0, (s-1)/2.0, ((s-1)/2.0) + 4*s);
	fprintf(fd, "dir 0 0 %f\n", -4.0*s);
	fprintf(fd, "up 0.0 1.0 0.0\n");
	fprintf(fd, "aspect 0.962567\n");
	fprintf(fd, "fovy 45\n");
	fprintf(fd, "1\n");
	fprintf(fd, "1 -2 -1 1 1 1\n");
	fprintf(fd, "0.0187866\n");
	fprintf(fd, "24.1953\n");
	fprintf(fd, "0\n");
	fprintf(fd, "2\n");
	fprintf(fd, "0 0\n");
	fprintf(fd, "1 1\n");
	fprintf(fd, "/home/01249/gda/colormaps/CoolWarm.cmap\n");
	fprintf(fd, "0.0 0 0 0\n");
	fprintf(fd, "0.0 0 0 0\n");
	fprintf(fd, "0.0 0 0 0\n");
	fprintf(fd, "0 0\n");
	fprintf(fd, "0 0\n");
	fprintf(fd, "0 0\n");
	fclose(fd);
}

