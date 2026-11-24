#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sysmon_ipc.h>

#define INTERVAL_SECONDS 1

// Same /proc/meminfo source as Phase 1; now save values in a struct.
int collect_memory(MemData *data)
{
    FILE *source = fopen("/proc/meminfo", "r");
    if (source == NULL) {
        perror("fopen meminfo");
        return 1;
    }

    char buffer[256];
    int found = 0;
    while (fgets(buffer, sizeof(buffer), source) != NULL) {
        if (strncmp(buffer, "MemTotal:", 9) == 0)
            found += (sscanf(buffer, "MemTotal: %ld", &data->mem_total) == 1);
        else if (strncmp(buffer, "MemFree:", 8) == 0)
            found += (sscanf(buffer, "MemFree: %ld", &data->mem_free) == 1);
        else if (strncmp(buffer, "MemAvailable:", 13) == 0)
            found += (sscanf(buffer, "MemAvailable: %ld", &data->mem_avail) == 1);
        else if (strncmp(buffer, "SwapTotal:", 10) == 0)
            found += (sscanf(buffer, "SwapTotal: %ld", &data->swap_total) == 1);
        else if (strncmp(buffer, "SwapFree:", 9) == 0)
            found += (sscanf(buffer, "SwapFree: %ld", &data->swap_free) == 1);
    }
    fclose(source);
    return found == 5 ? 0 : 1;
}

int main(int argc, char *argv[])
{
    // Default: STDOUT; sysmond passes the pipe write FD as argv[1].
    int fd = STDOUT_FILENO;
    if (argc > 1)
        fd = atoi(argv[1]);

    while (1) {
        MemData data = {0};
        if (collect_memory(&data) != 0) return 1;
        if (write(fd, &data, sizeof(data)) != sizeof(data)) {
            perror("write mem pipe");
            return 1;
        }
        sleep(INTERVAL_SECONDS);
    }
}
