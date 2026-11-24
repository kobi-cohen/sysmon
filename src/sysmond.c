#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <sysmon_ipc.h>

extern char **environ;

volatile sig_atomic_t stop = 0;

void on_signal(int sig)
{
    (void)sig;
    stop = 1;
}

// Keep reading until one complete struct arrives.
int read_data(int fd, void *data, size_t size)
{
    size_t received = 0;
    while (received < size && !stop) {
        ssize_t n = read(fd, (char *)data + received, size - received);
        if (n == 0) return 1;
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("read");
            return 1;
        }
        received += (size_t)n;
    }
    return stop ? 1 : 0;
}

// Launch one sensor: child replaces itself with the sensor executable.
pid_t start_sensor(const char *path, int close_fd)
{
    pid_t pid = fork();
    if (pid == 0) {
        close(close_fd); // Child does not read the memory pipe.
        char *args[] = {(char *)path, NULL};
        execve(path, args, environ);
        perror("execve");
        _exit(1);
    }
    return pid;
}

int main(void)
{
    pid_t mem_pid = -1, cpu_pid = -1, proc_pid = -1;
    int pipefd[2] = {-1, -1};
    int cpu_fd = -1, proc_fd = -1;
    int cpu_created = 0, proc_created = 0;

    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // 1. Create named pipes (FIFO files).
    if (mkfifo(CPU_FIFO_PATH, 0600) == -1) {
        perror("mkfifo cpu");
        return 1;
    }
    cpu_created = 1;
    if (mkfifo(PROC_FIFO_PATH, 0600) == -1) {
        perror("mkfifo proc");
        goto cleanup;
    }
    proc_created = 1;

    // 2. Create anonymous pipe for mem_sensor.
    if (pipe(pipefd) == -1) {
        perror("pipe");
        goto cleanup;
    }
    mem_pid = fork();
    if (mem_pid == 0) {
        close(pipefd[0]); // Child writes only.
        char fd_text[20];
        snprintf(fd_text, sizeof(fd_text), "%d", pipefd[1]);
        char *args[] = {"./bin/mem_sensor", fd_text, NULL};
        execve(args[0], args, environ);
        perror("execve mem_sensor");
        _exit(1);
    }
    if (mem_pid < 0) {
        perror("fork mem");
        goto cleanup;
    }
    close(pipefd[1]); // Parent reads only.
    pipefd[1] = -1;

    // 3. Launch FIFO writers.
    cpu_pid = start_sensor("./bin/cpu_sensor", pipefd[0]);
    if (cpu_pid < 0) {
        perror("fork cpu");
        goto cleanup;
    }
    proc_pid = start_sensor("./bin/proc_sensor", pipefd[0]);
    if (proc_pid < 0) {
        perror("fork proc");
        goto cleanup;
    }

    // Opening a FIFO for reading waits for its writer to connect.
    cpu_fd = open(CPU_FIFO_PATH, O_RDONLY);
    if (cpu_fd == -1) {
        perror("open cpu fifo");
        goto cleanup;
    }
    proc_fd = open(PROC_FIFO_PATH, O_RDONLY);
    if (proc_fd == -1) {
        perror("open proc fifo");
        goto cleanup;
    }

    // 4. Receive binary structs and print the dashboard.
    while (!stop) {
        MemData mem;
        CpuData cpu;
        ProcData proc;
        if (read_data(pipefd[0], &mem, sizeof(mem)) != 0) break;
        if (read_data(cpu_fd, &cpu, sizeof(cpu)) != 0) break;
        if (read_data(proc_fd, &proc, sizeof(proc)) != 0) break;

        printf("\n========== SYSMON PHASE 3 ==========\n");
        printf("Memory: total=%ld free=%ld available=%ld kB\n",
               mem.mem_total, mem.mem_free, mem.mem_avail);
        printf("Swap: total=%ld free=%ld kB\n", mem.swap_total, mem.swap_free);
        printf("CPU load: %.2f  %.2f  %.2f\n", cpu.load1, cpu.load5, cpu.load15);
        printf("Processes: %d\n", proc.total_procs);
        fflush(stdout);
    }

cleanup:
    // 5. Stop children, reap them, close descriptors and remove FIFOs.
    if (mem_pid > 0) kill(mem_pid, SIGTERM);
    if (cpu_pid > 0) kill(cpu_pid, SIGTERM);
    if (proc_pid > 0) kill(proc_pid, SIGTERM);

    if (mem_pid > 0) waitpid(mem_pid, NULL, 0);
    if (cpu_pid > 0) waitpid(cpu_pid, NULL, 0);
    if (proc_pid > 0) waitpid(proc_pid, NULL, 0);

    if (pipefd[0] != -1) close(pipefd[0]);
    if (pipefd[1] != -1) close(pipefd[1]);
    if (cpu_fd != -1) close(cpu_fd);
    if (proc_fd != -1) close(proc_fd);
    if (cpu_created) unlink(CPU_FIFO_PATH);
    if (proc_created) unlink(PROC_FIFO_PATH);
    printf("sysmond stopped.\n");
    return 0;
}
