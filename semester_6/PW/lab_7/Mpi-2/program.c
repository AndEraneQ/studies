#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

struct fabric{
    int width;
    int height;
    char color[10];
    char material[10];
};

int main(int argc, char **argv)
{
    int i;
    int rank, ranksent, size, dest, tag, nextrank, prevrank;
    MPI_Aint extent, lower_bound;
    MPI_Datatype fabric_type;
    double start, stop;
    struct fabric f1, f2;

    MPI_Aint offsets[4];

    f1.width = 13;
    f1.height = 20;
    strcpy(f1.color, "blue");
    strcpy(f1.material, "cotton");

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == size - 1){
        prevrank = rank - 1;
        nextrank = 0;
    } else if(rank == 0){
        prevrank = size - 1;
        nextrank = rank + 1;
    } else {
        prevrank = rank - 1;
        nextrank = rank + 1;
    }

    offsets[0] = 0;

    MPI_Type_get_extent(MPI_INT, &lower_bound, &extent);
    offsets[1] = extent;

    MPI_Type_get_extent(MPI_INT, &lower_bound, &extent);
    offsets[2] = offsets[1] + extent;

    MPI_Type_get_extent(MPI_CHAR, &lower_bound, &extent);
    offsets[3] = offsets[2] + 10 * extent;

    int blockcounts[] = {1, 1, 10, 10};
    MPI_Datatype oldtypes[] = {MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};

    MPI_Type_create_struct(4, blockcounts, offsets, oldtypes, &fabric_type);
    MPI_Type_commit(&fabric_type);

    if(rank == 0){
        start = MPI_Wtime();

        MPI_Send(&f1, 1, fabric_type, nextrank, 0, MPI_COMM_WORLD);
        MPI_Recv(&f2, 1, fabric_type, prevrank, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        stop = MPI_Wtime();

        printf("rank = %d :\n\twidth = %d\n\theight = %d\n\tcolor = %s\n\tmaterial = %s\n",
               rank, f2.width, f2.height, f2.color, f2.material);
        printf("time = %f\n", stop - start);

        FILE* wfp = fopen("./dane_3.csv", "a+");
        fprintf(wfp, "%d,%lf\n", size, stop - start);
    } else {
        MPI_Recv(&f2, 1, fabric_type, prevrank, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Send(&f2, 1, fabric_type, nextrank, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
