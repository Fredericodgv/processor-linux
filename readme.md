# PROC - Gerenciador Simples de Processos

Este programa implementa funcionalidades básicas de gerenciamento de processos e threads no Linux. Ele permite listar processos, criar novos processos e threads, e enviar sinais para processos. O código utiliza chamadas de sistema (`syscalls`) diretamente para realizar operações de baixo nível, como leitura do diretório `/proc` e manipulação de processos.

## Funcionalidades

- **Listar processos**: Lista todos os processos em execução no sistema, exibindo o PID e o nome do comando associado.
- **Criar processo**: Cria um novo processo utilizando `fork()`.
- **Criar thread**: Cria uma nova thread utilizando `clone()`.
- **Matar processos**: Envia sinais `SIGTERM` ou `SIGKILL` para um processo específico com base no PID.

### Comandos disponíveis

Após executar o programa, você pode usar os seguintes comandos:

- **`list`**: Lista todos os processos em execução.
- **`create -p`**: Cria um novo processo.
- **`create -t`**: Cria uma nova thread.
- **`kill <pid>`**: Envia o sinal `SIGTERM` para o processo especificado.
- **`kill --force <pid>`**: Envia o sinal `SIGKILL` (força a terminação) para o processo especificado.
- **`exit`**: Sai do programa.

## Estrutura do Código

### Funções principais

- **`list_processes()`**: Lista todos os processos em execução lendo o diretório `/proc`. Utiliza a chamada de sistema `SYS_getdents64` para percorrer as entradas do diretório e ler o nome do comando de cada processo a partir do arquivo `/proc/[PID]/comm`.
  
- **`create_process()`**: Cria um novo processo utilizando a chamada de sistema `SYS_fork`. O processo pai continua a execução, enquanto o processo filho entra em um loop infinito.
  
- **`create_thread()`**: Cria uma nova thread utilizando a chamada de sistema `SYS_clone`. A função `clone()` permite criar threads com a mesma memória e recursos compartilhados.

- **`kill_process(const char *pid_str, int force)`**: Envia um sinal para um processo com base no PID. Usa a chamada de sistema `SYS_kill` para enviar `SIGTERM` (padrão) ou `SIGKILL` (se o argumento `force` for 1).

### Compilação

O código utiliza chamadas de sistema diretamente, portanto deve ser compilado com permissões adequadas e com uma versão recente do compilador GCC.

Para compilar o programa, execute o seguinte comando:

```bash
gcc -o proc proc.c
