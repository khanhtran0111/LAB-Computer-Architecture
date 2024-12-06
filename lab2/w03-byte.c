#include "iobench.h"
#include <stdio.h>

int main() {
    // Opens a file called `data` using the fopen function in write mode
    FILE *fp = fopen("data", "w");
    
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    // writes the character '6' to the file
    size_t size = 5120000;
    const char* buf = "6";
    double start = tstamp();

    size_t n = 0;
    while (n < size) {
        size_t r = fwrite(buf, 1, 1, fp);
        if (r != 1) {
            perror("fwrite");
            exit(1);
        }

        // with some frequency (defined in iobench.h), prints out
        // how long it takes to complete the write.
        n += r;
        if (n % PRINT_FREQUENCY == 0) {
            report(n, tstamp() - start);
        }
    }

    fclose(fp);
    report(n, tstamp() - start);
    fprintf(stderr, "\n");
}
