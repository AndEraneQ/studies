#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double leibniz_series(int start, int end) {
    double sum = 0.0;
    for (int i = start; i < end; i++) {
        if (i % 2 == 0)
            sum += 1.0 / (2 * i + 1);
        else
            sum -= 1.0 / (2 * i + 1);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int rank, size, n;
    double local_sum, total_sum, pi;
    int elements_per_process, remainder;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc != 2) {
            printf("Użycie: %s <liczba składników>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        n = atoi(argv[1]);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    elements_per_process = n / size;
    remainder = n % size;

    int start = rank * elements_per_process + (rank < remainder ? rank : remainder);
    int end = start + elements_per_process + (rank < remainder ? 1 : 0);

    local_sum = leibniz_series(start, end);

    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        pi = 4.0 * total_sum;
        printf("Wartość pi z szeregu Leibniza: %.15f\n", pi);
    }

    MPI_Finalize();
    return 0;
}
