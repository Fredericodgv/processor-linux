#include <stdio.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>

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

int main()
{
    printf("Comandos disponíveis:\n");
    printf(" - list : Lista todos os processos em execução.\n");
    printf(" - exit : Sai do programa.\n");
    printf("\n");

    char command[256];

    while (1)
    {
        printf("Digite um comando: ");
        if (fgets(command, sizeof(command), stdin) != NULL)
        {
            // Remove newline character
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "list") == 0)
            {
                list_processes();
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
