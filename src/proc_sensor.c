#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysmon_ipc.h>

#define INTERVAL_SECONDS 1

int collect_processes(ProcData *data)
{
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir /proc");
        return 1;
    }
    data->total_procs = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        int is_pid = 1;
        for (int i = 0; entry->d_name[i] != '\0'; i++) {
            if (!isdigit((unsigned char)entry->d_name[i])) {
                is_pid = 0;
                break;
            }
        }
        if (is_pid) data->total_procs++;
    }
    closedir(dir);
    return 0;
}

int main(void)
{
    int fd = open(PROC_FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open proc fifo");
        return 1;
    }
    while (1) {
        ProcData data = {0};
        if (collect_processes(&data) != 0) break;
        if (write(fd, &data, sizeof(data)) != sizeof(data)) break;
        sleep(INTERVAL_SECONDS);
    }
    close(fd);
    return 1;
}
