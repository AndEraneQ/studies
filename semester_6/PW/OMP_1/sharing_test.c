#include <stdio.h>
#include <omp.h>

int main() {
    int a = 7;

    printf("Before parallel: a = %d\n", a);

    #pragma omp parallel num_threads(7) private(a)
    {
        for (int i = 0; i < 10; i++) {
            printf("Thread %d a = %d\n", omp_get_thread_num(), a);
            a++;
        }
    }

    printf("After parallel: a = %d\n", a);

    return 0;
}
