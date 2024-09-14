#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define STACK_SIZE 1024 * 1024  // 1MB

#define BUF_SIZE 1024

struct linux_dirent64 {
    ino64_t        d_ino;
    off64_t        d_off;
    unsigned short d_reclen;
    unsigned char  d_type;
    char           d_name[];
};

void list_processes()
{
    int fd;                     // Descritor de arquivo para o diretório /proc
    char buf[BUF_SIZE];         // Buffer para armazenar as entradas de diretório
    struct linux_dirent64 *dir; // Ponteiro para a entrada de diretório
    int bufPos;                   // Posição atual no buffer

    // Abre o diretório /proc usando chamada de sistema
    fd = syscall(SYS_open, "/proc", O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("Erro ao abrir /proc");
        return;
    }

    printf("%-10s %-50s\n", "PID", "COMMAND");

    // Lê as entradas de diretório usando getdents64
    int nread;
    while ((nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE)) > 0) {
        for (bufPos = 0; bufPos < nread;) {
            dir = (struct linux_dirent64 *)(buf + bufPos);

            // Verifica se a entrada é um número (PID)
            if (isdigit(dir->d_name[0])) {
                // Construir o caminho para o arquivo comm
                char comm_path[256];
                snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", dir->d_name);

                // Abre o arquivo comm
                int comm_file = syscall(SYS_open, comm_path, O_RDONLY);
                if (comm_file != -1) {
                    char comm[256];
                    int comm_len = syscall(SYS_read, comm_file, comm, sizeof(comm) - 1);
                    if (comm_len > 0) {
                        comm[comm_len] = '\0';
                        // Remove a nova linha
                        comm[strcspn(comm, "\n")] = '\0';
                        printf("%-10s %-50s\n", dir->d_name, comm);
                    }
                    syscall(SYS_close, comm_file);  // Fecha o arquivo comm
                }
            }
            bufPos += dir->d_reclen;
        }
    }

    if (nread == -1) {
        perror("Erro ao ler diretório /proc");
    }

    syscall(SYS_close, fd);  // Fecha o diretório /proc
}

// void list_processes()
// {
//     int process_count = 0;
//     struct dirent *entry;
//     DIR *dir = opendir("/proc");

//     if (dir == NULL)
//     {
//         perror("Erro ao abrir /proc");
//         return;
//     }

//     printf("%-10s %-50s\n", "PID", "COMMAND");

//     while ((entry = readdir(dir)) != NULL)
//     {
//         if (isdigit(entry->d_name[0]))
//         { // Se o nome do diretório começar com um número, é um PID
//             process_count++;
//             char comm_path[256];
//             snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", entry->d_name);

//             FILE *comm_file = fopen(comm_path, "r");
//             if (comm_file)
//             {
//                 char comm[256];
//                 if (fgets(comm, sizeof(comm), comm_file) != NULL)
//                 {
//                     // Remove newline character, if any
//                     comm[strcspn(comm, "\n")] = '\0';
//                     printf("%-10s %-50s\n", entry->d_name, comm);
//                 }
//                 else
//                 {
//                     printf("%-10s %-50s\n", entry->d_name, "[erro ao ler comando]");
//                 }
//                 fclose(comm_file);
//             }
//             else
//             {
//                 printf("%-10s %-50s\n", entry->d_name, "[erro ao abrir comm]");
//             }
//         }
//     }

//     closedir(dir);

//     printf("\nTotal de processos listados: %d\n\n", process_count);
// }

void kill_process(const char *pid_str, int force)
{
    char *endptr;
    pid_t pid_num = strtol(pid_str, &endptr, 10);

    if (*endptr != '\0' || pid_num <= 0)
    {
        fprintf(stderr, "PID inválido: %s\n", pid_str);
        return;
    }

    int signal = force ? 9 : 15; // SIGKILL = 9, SIGTERM = 15

    // Chamada de sistema para enviar sinal
    long result = syscall(__NR_kill, pid_num, signal);

    if (result == -1)
    {
        perror("Erro ao enviar sinal para o processo");
    }
    else
    {
        printf("Sinal %s enviado para o processo %d\n", force ? "SIGKILL" : "SIGTERM", pid_num);
    }
}

void create_process()
{
    pid_t pid = syscall(SYS_fork);

    if (pid == -1)
    {
        perror("Erro ao criar processo");
    }
    else if (pid == 0)
    {
        // Código do processo filho
        while (1)
        {
            pause();
        }
    }
    else
    {
        // Código do processo pai
        printf("Processo pai criou o processo filho com PID %d, PID do pai é %d\n", pid, getpid());
    }
}

int thread_function(void *arg)
{
    printf("Thread criada com TID %ld\n", syscall(SYS_gettid));

    while (1)
    {
        pause();
    }

    return 0;
}

void create_thread()
{
    char *stack;  // Pilha para a nova thread
    char *stack_top;  // Topo da pilha

    // Aloca memória para a pilha
    stack = malloc(STACK_SIZE);
    if (stack == NULL)
    {
        perror("Erro ao alocar memória para a pilha");
        exit(EXIT_FAILURE);
    }

    stack_top = stack + STACK_SIZE;  // A pilha cresce para baixo

    // Cria a thread utilizando syscall clone
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
    pid_t tid = syscall(SYS_clone, flags, stack_top, NULL, NULL, NULL);

    if (tid == -1)
    {
        perror("Erro ao criar thread");
        free(stack);  // Libera a memória da pilha
    }
    else if (tid == 0)
    {
        // Código da nova thread
        thread_function(NULL);
    }
    else
    {
        // Código da thread principal
        printf("Thread principal criou uma nova thread com TID %d\n", tid);
    }
}


int main()
{
    printf("Comandos disponíveis:\n");
    printf(" - list : Lista todos os processos em execução.\n");
    printf(" - create -p : Cria um novo processo.\n");
    printf(" - create -t : Cria uma nova thread.\n");
    printf(" - kill <pid> : Envia um sinal SIGTERM para o processo com o PID especificado.\n");
    printf(" - kill --force <pid> : Envia um sinal SIGKILL para o processo com o PID especificado.\n");
    printf(" - exit : Sai do programa.\n");
    printf("\n");

    char command[256];

    while (1)
    {
        printf("PROC %d: Digite um comando: ", getpid());
        if (fgets(command, sizeof(command), stdin) != NULL)
        {
            // Remove newline character
            command[strcspn(command, "\n")] = '\0';

            if (strncmp(command, "list", 4) == 0)
            {
                list_processes();
            }
            else if (strncmp(command, "create -p", 9) == 0)
            {
                create_process();
                sleep(1);
            }
            else if (strncmp(command, "create -t", 9) == 0)
            {
                create_thread();
                sleep(1);
            }
            else if (strncmp(command, "kill --force ", 13) == 0)
            {
                kill_process(command + 13, 1);
            }
            else if (strncmp(command, "kill ", 5) == 0)
            {
                kill_process(command + 5, 0);
            }
            else if (strcmp(command, "exit") == 0)
            {
                printf("Saindo do programa.\n");
                break;
            }
            else
            {
                printf("Comando inválido\n");
            }
        }
    }

    return 0;
}
