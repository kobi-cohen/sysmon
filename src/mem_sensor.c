#include <stdio.h>
#include <string.h>
#include <unistd.h>


// MACROS FOR INITIAL VALUES
#define INTERVAL_SECONDS 5
#define OUTPUT_FILE "/tmp/sysmon/mem.dat"

int collect_memory(void)
{
    FILE *source = fopen("/proc/meminfo", "r");
    if (source == NULL) {
        perror("fopen /proc/meminfo");
        return 1;
    }

    FILE *destination = fopen(OUTPUT_FILE, "w");
    if (destination == NULL) {
        perror("fopen mem.dat");
        fclose(source);
        return 1;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), source) != NULL) {
        // Copy only the five fields.
        if (strncmp(buffer, "MemTotal:", 9) == 0 ||
            strncmp(buffer, "MemFree:", 8) == 0 ||
            strncmp(buffer, "MemAvailable:", 13) == 0 ||
            strncmp(buffer, "SwapTotal:", 10) == 0 ||
            strncmp(buffer, "SwapFree:", 9) == 0) {
            if (fputs(buffer, destination) == EOF) {
                perror("fputs mem.dat");
                fclose(source);
                fclose(destination);
                return 1;
            }
        }
    }

    if (ferror(source)) {
        perror("fgets /proc/meminfo");
        fclose(source);
        fclose(destination);
        return 1;
    }

    fclose(source);
    if (fclose(destination) == EOF) {
        perror("fclose mem.dat");
        return 1;
    }
    return 0;
}

int main(void)
{
    while (1) {
        if (collect_memory() != 0)
            return 1;
        sleep(INTERVAL_SECONDS);
    }
    return 0;
}
