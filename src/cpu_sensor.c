#include <stdio.h>
#include <unistd.h>



// MACROS FOR INITIAL VALUES

#define INTERVAL_SECONDS 5
#define OUTPUT_FILE "/tmp/sysmon/cpu.dat"

int collect_cpu(void)
{
    FILE *source = fopen("/proc/loadavg", "r");
    if (source == NULL) {
        perror("fopen /proc/loadavg");
        return 1;
    }

    double load1, load5, load15;
    if (fscanf(source, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
        fprintf(stderr, "Could not read CPU load values\n");
        fclose(source);
        return 1;
    }
    fclose(source);

    FILE *destination = fopen(OUTPUT_FILE, "w");
    if (destination == NULL) {
        perror("fopen cpu.dat");
        return 1;
    }

    fprintf(destination, "Load1: %.2f\n", load1);
    fprintf(destination, "Load5: %.2f\n", load5);
    fprintf(destination, "Load15: %.2f\n", load15);

    if (fclose(destination) == EOF) {
        perror("fclose cpu.dat");
        return 1;
    }
    return 0;
}

int main(void)
{
    while (1) {
        if (collect_cpu() != 0)
            return 1;
        sleep(INTERVAL_SECONDS);
    }
    return 0;
}
