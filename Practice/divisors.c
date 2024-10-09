#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void find_divisors(int num, int rank, int size, int *local_divisors,
                   int *local_count) {
  int start = (num / size) * rank + 1;
  int end = (num / size) * (rank + 1);
  if (rank == size - 1) {
    end = num;
  }

  printf("Processor %d finding divisors in range %d to %d\n", rank, start, end);
  *local_count = 0;
  for (int i = start; i <= end; i++) {
    if (num % i == 0) {
      local_divisors[*local_count] = i;
      (*local_count)++;
    }
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if (argc != 2) {
    if (world_rank == 0) {
      fprintf(stderr, "Usage: %s <number>\n", argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  int num = atoi(argv[1]);
  int local_divisors[num];
  int local_count;

  find_divisors(num, world_rank, world_size, local_divisors, &local_count);

  int *all_divisors = NULL;
  int *recv_counts = NULL;
  int *displs = NULL;

  if (world_rank != 0) {
    MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(local_divisors, local_count, MPI_INT, 0, 0, MPI_COMM_WORLD);
  } else {
    all_divisors = (int *)malloc(num * sizeof(int));
    recv_counts = (int *)malloc(world_size * sizeof(int));
    displs = (int *)malloc(world_size * sizeof(int));

    recv_counts[0] = local_count;
    displs[0] = 0;
    for (int i = 0; i < local_count; i++) {
      all_divisors[i] = local_divisors[i];
    }

    for (int i = 1; i < world_size; i++) {
      MPI_Recv(&recv_counts[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      displs[i] = displs[i - 1] + recv_counts[i - 1];
      MPI_Recv(all_divisors + displs[i], recv_counts[i], MPI_INT, i, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("All divisors of %d: ", num);
    for (int i = 0; i < displs[world_size - 1] + recv_counts[world_size - 1];
         i++) {
      printf("%d ", all_divisors[i]);
    }
    printf("\n");

    free(all_divisors);
    free(recv_counts);
    free(displs);
  }

  MPI_Finalize();
  return 0;
}
