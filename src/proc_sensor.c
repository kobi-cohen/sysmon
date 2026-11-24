#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>


// MACROS FOR INITIAL VALUES

#define INTERVAL_SECONDS 5
#define OUTPUT_FILE "/tmp/sysmon/proc.dat"

int collect_processes(void)
{
    DIR *dir = opendir("/proc");
    if (dir == NULL) {
        perror("opendir /proc");
        return 1;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // PID directory names contain digits only.
        int is_pid = 1;
        for (int i = 0; entry->d_name[i] != '\0'; i++) {
            if (!isdigit((unsigned char)entry->d_name[i])) {
                is_pid = 0;
                break;
            }
        }
        if (is_pid)
            count++;
    }
    closedir(dir);

    FILE *destination = fopen(OUTPUT_FILE, "w");
    if (destination == NULL) {
        perror("fopen proc.dat");
        return 1;
    }

    fprintf(destination, "Processes: %d\n", count);
    if (fclose(destination) == EOF) {
        perror("fclose proc.dat");
        return 1;
    }
    return 0;
}

int main(void)
{
    while (1) {
        if (collect_processes() != 0)
            return 1;
        sleep(INTERVAL_SECONDS);
    }
    return 0;
}
