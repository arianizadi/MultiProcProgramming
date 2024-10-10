#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void printTime(int rank, struct timeval t1, struct timeval t2) {
  long long l;
  long millis;
  l = (t2.tv_sec * 1000000 + t2.tv_usec) - (t1.tv_sec * 1000000 + t1.tv_usec);
  millis = l / 1000;
  printf("%d: (%ld:%d -> %ld:%d (%ld ms)\n", rank, t1.tv_sec, t1.tv_usec,
         t2.tv_sec, t2.tv_usec, millis);
}

int main(int argc, char *argv[]) {
  clock_t start_time, end_time;
  double cpu_time_used;

  start_time = clock();
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);

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

  disp_width = atoi(argv[1]);
  disp_height = atoi(argv[2]);

  real_min = atof(argv[3]);
  real_max = atof(argv[4]);
  imag_min = atof(argv[5]);
  imag_max = atof(argv[6]);

  f = fopen(argv[7], "r");
  for (i = 0; i < 257; i++) {
    fgets(str, 1000, f);
    sscanf(str, "%d %d %d", &(map[0][i]), &(map[1][i]), &(map[2][i]));
  }
  fclose(f);

  scale_real = (real_max - real_min) / disp_width;
  scale_imag = (imag_max - imag_min) / disp_height;

  max = 256;

  MPI_Init(&argc, &argv);
  int proc_rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  printf("Processor %d of %d\n", proc_rank, world_size);

  unsigned char *allocation = malloc(3 * disp_width * sizeof(unsigned char));
  unsigned char *image = NULL;
  if (proc_rank == 0) {
    image = malloc(3 * disp_width * disp_height * sizeof(unsigned char));
  }

  for (y = proc_rank; y < disp_height; y += world_size) {
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
      allocation[3 * x + 0] = map[0][count];
      allocation[3 * x + 1] = map[1][count];
      allocation[3 * x + 2] = map[2][count];
    }

    if (proc_rank == 0) {
      memcpy(image + 3 * disp_width * y, allocation, 3 * disp_width);
      for (int src = 1; src < world_size; src++) {
        MPI_Recv(image + 3 * disp_width * (y + src), 3 * disp_width, MPI_CHAR,
                 src, 0, MPI_COMM_WORLD, 0);
      }
    } else {
      MPI_Send(allocation, 3 * disp_width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
  }

  printf("Processor %d done\n", proc_rank);

  if (proc_rank == 0) {
    f = fopen(argv[8], "wb");
    fprintf(f, "P6\n%d %d\n255\n", disp_width, disp_height);
    fwrite(image, 3, disp_width * disp_height, f);
    fclose(f);
    free(image);
  }

  free(allocation);
  MPI_Finalize();

  end_time = clock();
  cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  if (proc_rank == 0) {
    printf("Time taken: %f seconds\n", cpu_time_used);
  }

  gettimeofday(&t2, NULL);
  printTime(proc_rank, t1, t2);

  return 0;
}
