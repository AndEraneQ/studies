#include <stdio.h>
#include <omp.h>

int main() {
    int n = 1000;
    int sum = 0;
    double start = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 1; i <= n; i++) {
        int temp = i * i;
        #pragma omp critical
        sum += temp;
    }

    double end = omp_get_wtime();

    printf("Time: %f seconds\n", end - start);
    printf("Sum of squares = %d\n", sum);

    return 0;
}
