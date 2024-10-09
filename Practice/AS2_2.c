#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  clock_t start_time, end_time;
  double cpu_time_used;

  start_time = clock();

  int disp_width, disp_height;
  float real_min, real_max, imag_min, imag_max, scale_real, scale_imag;
  FILE *f;
  int x, y, i;
  char str[256];
  int count, max;
  float temp, temp2, lengthsq, zreal, zimag, creal, cimag;
  int map[3][257];

  if (argc != 9) {
    printf("Usage:\n Mandelbrot width height real-min real-max imag-min "
           "imag-max mapfile outfile\n");
    exit(1);
  }

  /* Decode arguments */
  disp_width = atoi(argv[1]);
  disp_height = atoi(argv[2]);

  real_min = atof(argv[3]);
  real_max = atof(argv[4]);
  imag_min = atof(argv[5]);
  imag_max = atof(argv[6]);

  /* Load the required colour map file */
  f = fopen(argv[7], "r");
  for (i = 0; i < 257; i++) {
    fgets(str, 1000, f);
    sscanf(str, "%d %d %d", &(map[0][i]), &(map[1][i]), &(map[2][i]));
  }
  fclose(f);

  /* Compute scaling factors */
  scale_real = (real_max - real_min) / disp_width;
  scale_imag = (imag_max - imag_min) / disp_height;

  max = 256;

  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  printf("Processor %d of %d\n", rank, size);

  unsigned char *strip =
      (unsigned char *)malloc(3 * disp_width * sizeof(unsigned char));
  unsigned char *image = NULL;
  if (rank == 0) {
    image = (unsigned char *)malloc(3 * disp_width * disp_height *
                                    sizeof(unsigned char));
  }

  for (y = rank; y < disp_height; y += size) {
    for (x = 0; x < disp_width; x++) {
      creal = real_min + ((float)x * scale_real);
      cimag = imag_min + ((float)y * scale_imag);

      zreal = 0;
      zimag = 0;
      count = 0;
      temp = 0;
      temp2 = 0;
      do {
        temp = temp - temp2 + creal;
        zimag = 2 * zreal * zimag + cimag;
        zreal = temp;
        temp = zreal * zreal;
        temp2 = zimag * zimag;
        lengthsq = temp + temp2;
        count++;
      } while ((lengthsq < 4.0) && (count < max));
      strip[3 * x + 0] = map[0][count];
      strip[3 * x + 1] = map[1][count];
      strip[3 * x + 2] = map[2][count];
    }
    MPI_Gather(strip, 3 * disp_width, MPI_UNSIGNED_CHAR,
               image + 3 * disp_width * y, 3 * disp_width, MPI_UNSIGNED_CHAR, 0,
               MPI_COMM_WORLD);
  }

  printf("Processor %d done\n", rank);

  if (rank == 0) {
    f = fopen(argv[8], "wb");
    fprintf(f, "P6\n%d %d\n255\n", disp_width, disp_height);
    fwrite(image, 3, disp_width * disp_height, f);
    fclose(f);
    free(image);
  }

  free(strip);
  MPI_Finalize();

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  if (rank == 0) {
    printf("Time taken: %f seconds\n", cpu_time_used);
  }

  return 0;
}
