#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>


void list_processes()
{
    int process_count = 0;
    struct dirent *entry;
    DIR *dir = opendir("/proc");

    if (dir == NULL)
    {
        perror("Erro ao abrir /proc");
        return;
    }

    printf("%-10s %-50s\n", "PID", "COMMAND");

    while ((entry = readdir(dir)) != NULL)
    {
        if (isdigit(entry->d_name[0]))
        { // Se o nome do diretório começar com um número, é um PID
            process_count++;
            char comm_path[256];
            snprintf(comm_path, sizeof(comm_path), "/proc/%s/comm", entry->d_name);

            FILE *comm_file = fopen(comm_path, "r");
            if (comm_file)
            {
                char comm[256];
                if (fgets(comm, sizeof(comm), comm_file) != NULL)
                {
                    // Remove newline character, if any
                    comm[strcspn(comm, "\n")] = '\0';
                    printf("%-10s %-50s\n", entry->d_name, comm);
                }
                else
                {
                    printf("%-10s %-50s\n", entry->d_name, "[erro ao ler comando]");
                }
                fclose(comm_file);
            }
            else
            {
                printf("%-10s %-50s\n", entry->d_name, "[erro ao abrir comm]");
            }
        }
    }

    closedir(dir);

    printf("\nTotal de processos listados: %d\n\n", process_count);
}


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
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("Erro ao criar processo");
    }
    else if (pid == 0)
    {
        // Código do processo filho
        printf("Processo filho criado com PID %d, PID do pai é %d\n", getpid(), getppid());
        while (1)
        {
            sleep(1);
        }
        
    }
    else
    {
        // Código do processo pai
        printf("Processo pai criou o processo filho com PID %d, PID do pai é %d\n", pid, getpid());
    }
}

int main()
{
    printf("Comandos disponíveis:\n");
    printf(" - list : Lista todos os processos em execução.\n");
    printf(" - create : Cria um novo processo.\n");
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
            else if (strncmp(command, "create", 6) == 0)
            {
                create_process();
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
