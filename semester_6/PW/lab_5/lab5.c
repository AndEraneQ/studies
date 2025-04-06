#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void sieve_of_eratosthenes(int n) {
    bool *numbers = malloc((n + 1) * sizeof(bool));
    if (numbers == NULL) {
        printf("Memory allocation error.\n");
        return;
    }

    for (int i = 2; i <= n; i++) {
        numbers[i] = true;
    }

    for (int i = 2; i * i <= n; i++) {
        if (numbers[i]) {
            for (int j = i * i; j <= n; j += i) {
                numbers[j] = false;
            }
        }
    }

    printf("Prime numbers up to %d: ", n);
    for (int i = 2; i <= n; i++) {
        if (numbers[i]) {
            printf("%d ", i);
        }
    }
    printf("\n");

    free(numbers);
}

int main() {
    int n;
    printf("Enter the upper limit for prime numbers: ");
    scanf("%d", &n);

    if (n < 2) {
        printf("No prime numbers in the given range.\n");
        return 1;
    }

    sieve_of_eratosthenes(n);
    return 0;
}
