#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysmon_ipc.h>

#define INTERVAL_SECONDS 1

int collect_cpu(CpuData *data)
{
    FILE *source = fopen("/proc/loadavg", "r");
    if (source == NULL) {
        perror("fopen loadavg");
        return 1;
    }
    int result = fscanf(source, "%lf %lf %lf", &data->load1,
                        &data->load5, &data->load15);
    fclose(source);
    return result == 3 ? 0 : 1;
}

int main(void)
{
    int fd = open(CPU_FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open cpu fifo");
        return 1;
    }
    while (1) {
        CpuData data = {0};
        if (collect_cpu(&data) != 0) break;
        if (write(fd, &data, sizeof(data)) != sizeof(data)) break;
        sleep(INTERVAL_SECONDS);
    }
    close(fd);
    return 1;
}
