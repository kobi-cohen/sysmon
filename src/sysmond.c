#include <stdio.h>
#include <unistd.h>


// MACRO FOR INTERVAL UPDATE .
#define INTERVAL_SECONDS 5

void print_file(const char *heading, const char *filename)
{
    printf("\n[%s]\n", heading);

    FILE *source = fopen(filename, "r");
    if (source == NULL) {
        perror(filename);
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), source) != NULL) {
        printf("%s", buffer);
    }
    fclose(source);
}

int main(void)
{
    while (1) {
        printf("\n========== SYSMON ==========\n");
        print_file("MEMORY", "/tmp/sysmon/mem.dat");
        print_file("CPU LOAD", "/tmp/sysmon/cpu.dat");
        print_file("PROCESSES", "/tmp/sysmon/proc.dat");
        printf("============================\n");
        fflush(stdout);
        sleep(INTERVAL_SECONDS);
    }
    return 0;
}
