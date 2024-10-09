#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  int rank, size;

  // Initialize the MPI environment
  MPI_Init(NULL, NULL);

  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Allocate memory for the array to store the gathered values
  int *procs = NULL;
  if (rank == 0) {
    procs = (int *)malloc(size * sizeof(int));
  }

  // Create a request object for the non-blocking gather
  MPI_Request request;

  // Non-blocking gather all the values to the root process
  MPI_Igather(&rank, 1, MPI_INT, procs, 1, MPI_INT, 0, MPI_COMM_WORLD,
              &request);

  // Do other work here if needed while the gather is in progress
  printf("weee - Rank %d is process %d\n", rank, rank);

  // Wait for the non-blocking gather to complete
  MPI_Wait(&request, MPI_STATUS_IGNORE);

  // Print the gathered values
  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      printf("Rank %d is process %d\n", i, procs[i]);
    }
    free(procs);
  }

  // Finalize the MPI environment
  MPI_Finalize();

  return 0;
}