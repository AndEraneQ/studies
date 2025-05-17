#include <stdio.h>
#include <omp.h>

void run_schedule_test(omp_sched_t sched_type, int chunk_size) {
    omp_set_schedule(sched_type, chunk_size);
    double start = omp_get_wtime();

    #pragma omp parallel for schedule(runtime) num_threads(4)
    for (int i = 0; i < 10000000; i++) {
        // printf("Thread %d executes iteration %d\n", omp_get_thread_num(), i);
    }

    double end = omp_get_wtime();

    printf("Time: %f seconds\n", end - start);
}

int main() {
    printf("Static, chunk=3\n");
    run_schedule_test(omp_sched_static, 3);

    printf("\nStatic, default chunk\n");
    run_schedule_test(omp_sched_static, 0);

    printf("\nDynamic, chunk=3\n");
    run_schedule_test(omp_sched_dynamic, 3);

    printf("\nDynamic, default chunk\n");
    run_schedule_test(omp_sched_dynamic, 0);

    return 0;
}
