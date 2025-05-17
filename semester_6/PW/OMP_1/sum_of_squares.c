#include <stdio.h>
#include <omp.h>

int main() {
    int n = 1000;
    int sum = 0;
    double start = omp_get_wtime();

    #pragma omp parallel for reduction(+:sum)
    for (int i = 1; i <= n; i++) {
        sum += i * i;
    }

    double end = omp_get_wtime();

    printf("Time: %f seconds\n", end - start);
    printf("Sum of squares = %d\n", sum);

    return 0;
}
