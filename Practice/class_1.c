#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  printf("Processor %s, rank %d out of %d processors\n", processor_name,
         world_rank, world_size);

  if (world_rank == 0) {
    int j[world_size - 1];
    MPI_Request request[world_size - 1];
    MPI_Status status[world_size - 1];
    for (int k = 1; k < world_size; k++) {
      MPI_Irecv(&j[k - 1], 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                &request[k - 1]);
    }
    MPI_Waitall(world_size - 1, request, status);
    for (int k = 1; k < world_size; k++) {
      printf("Received from processor %d: %d\n", status[k - 1].MPI_SOURCE,
             j[k - 1]);
    }
  } else {
    MPI_Send(&world_rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}
