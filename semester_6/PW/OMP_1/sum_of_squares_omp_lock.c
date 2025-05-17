#include <stdio.h>
#include <omp.h>

int main() {
    int n = 1000;
    int sum = 0;
    double start = omp_get_wtime();

    omp_lock_t lock;
    omp_init_lock(&lock);

    #pragma omp parallel for
    for (int i = 1; i <= n; i++) {
        int temp = i * i;
        omp_set_lock(&lock);
        sum += temp;
        omp_unset_lock(&lock);
    }

    omp_destroy_lock(&lock);
    double end = omp_get_wtime();

    printf("Time: %f seconds\n", end - start);
    printf("Sum of squares = %d\n", sum);
    return 0;
}
