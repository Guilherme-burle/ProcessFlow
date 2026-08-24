#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/types.h>  

typedef struct {
    char nome[64];
    char programa[128];
    char *argumentos[32];
    char guardar_args[31][64];
} Task;

int buscar_tarefa(Task tarefas[], int qnt_task, const char *nome) {
    for (int i = 0; i < qnt_task; i++) {
        if (strcmp(tarefas[i].nome, nome) == 0) {
            return i; 
        }
    }
    return -1; 
}

void executar_tarefa(Task *t) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Erro ao executar fork");
    } 
    else if (pid == 0) {
        execvp(t->programa, t->argumentos);
        perror("Erro ao executar programa com execvp");
        exit(EXIT_FAILURE);
    } 
    else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("Erro ao aguardar processo filho");
        }
    }
}

int main() {
    char linha[256];
    Task tarefas[16];
    int qnt_task = 0;

    while (1) {
        printf("processflow> ");
        fflush(stdout);
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }
        char *token = strtok(linha, " \t\n");
        if (token == NULL) {
            continue;
        }

        if (strcmp(token, "exit") == 0) {
            break;
        }

        if (strcmp(token, "task") == 0) {
            if (qnt_task >= 16) {
                printf("Erro: Limite máximo de tarefas atingido (16).\n");
                continue;
            }

            char *nome_token = strtok(NULL, " \t\n");
            if (nome_token == NULL) {
                printf("Erro: Formato inválido. Uso: task <nome> <programa> [argumentos...]\n");
                continue;
            }
            char *prog_token = strtok(NULL, " \t\n");
            if (prog_token == NULL) {
                printf("Erro: Formato inválido. Programa não informado. Uso: task <nome> <programa> [argumentos...]\n");
                continue;
            }

            Task *t = &tarefas[qnt_task];

            strncpy(t->nome, nome_token, 63);
            t->nome[63] = '\0';

            strncpy(t->programa, prog_token, 127);
            t->programa[127] = '\0';

            int arg_i = 0;
            strncpy(t->guardar_args[arg_i], t->programa, 63);
            t->guardar_args[arg_i][63] = '\0';
            t->argumentos[arg_i] = t->guardar_args[arg_i];
            arg_i++;

            char *arg_token = strtok(NULL, " \t\n");
            while (arg_token != NULL && arg_i < 31) {
                strncpy(t->guardar_args[arg_i], arg_token, 63);
                t->guardar_args[arg_i][63] = '\0';
                t->argumentos[arg_i] = t->guardar_args[arg_i];

                arg_i++;
                arg_token = strtok(NULL, " \t\n");
            }
            t->argumentos[arg_i] = NULL;
            qnt_task++;

            printf("\n--- Tarefa Cadastrada com Sucesso (Total: %d) ---\n", qnt_task);
            printf("Nome da tarefa : %s\n", t->nome);
            printf("Programa       : %s\n", t->programa);
            printf("Argumentos     : ");
            for (int i = 0; t->argumentos[i] != NULL; i++) {
                printf("[%s] ", t->argumentos[i]);
            }
            printf("\n\n");

        } else if (strcmp(token, "run") == 0) {
            
            char *token_temp = strtok(NULL, " \t\n");
            
            if (token_temp == NULL) {
                printf("Erro: Nome da tarefa não especificado. Uso: run <nome> OU run sequential/parallel/pipe <t1>...\n");
                continue;
            }
            if (strcmp(token_temp, "sequential") == 0) {
                Task *tarefas_para_executar[16];
                int count_run = 0;
                int erro_validacao = 0;
                char *nome_tarefa = strtok(NULL, " \t\n");
                if (nome_tarefa == NULL) {
                    printf("Erro: Nenhuma tarefa informada após 'run sequential'.\n");
                    continue;
                }
                while (nome_tarefa != NULL && count_run < 16) {
                    int j = buscar_tarefa(tarefas, qnt_task, nome_tarefa);
                    
                    if (j == -1) {
                        printf("Erro: Tarefa '%s' não foi encontrada. Cancelando execução sequencial.\n", nome_tarefa);
                        erro_validacao = 1;
                        break; 
                    }
                    tarefas_para_executar[count_run] = &tarefas[j];
                    count_run++;

                    nome_tarefa = strtok(NULL, " \t\n");
                }

                if (nome_tarefa != NULL && count_run == 16) {
                    printf("Erro: Limite máximo de 16 tarefas simultâneas excedido. Cancelando execução sequencial.\n");
                    erro_validacao = 1;
                }

                if (erro_validacao) {
                    continue;
                }

                for (int i = 0; i < count_run; i++) {
                    executar_tarefa(tarefas_para_executar[i]);
                }

            } 
            else if (strcmp(token_temp, "parallel") == 0) {
                Task *tarefas_para_executar[16];
                int count_run = 0;
                int erro_validacao = 0;

                char *nome_tarefa = strtok(NULL, " \t\n");
                if (nome_tarefa == NULL) {
                    printf("Erro: Nenhuma tarefa informada após 'run parallel'.\n");
                    continue;
                }

                while (nome_tarefa != NULL && count_run < 16) {
                    int j = buscar_tarefa(tarefas, qnt_task, nome_tarefa);
                    
                    if (j == -1) {
                        printf("Erro: Tarefa '%s' não foi encontrada. Cancelando execução paralela.\n", nome_tarefa);
                        erro_validacao = 1;
                        break; 
                    }

                    tarefas_para_executar[count_run] = &tarefas[j];
                    count_run++;

                    nome_tarefa = strtok(NULL, " \t\n");
                }

                if (nome_tarefa != NULL && count_run == 16) {
                    printf("Erro: Limite máximo de 16 tarefas simultâneas excedido. Cancelando execução paralela.\n");
                    erro_validacao = 1;
                }

                if (erro_validacao) {
                    continue;
                }

                pid_t pids[16];
                int processos_criados = 0;

                for (int i = 0; i < count_run; i++) {
                    pid_t pid = fork();

                    if (pid < 0) {
                        perror("Erro ao executar fork no modo paralelo");
                        break; 
                    } 
                    else if (pid == 0) {
                        Task *t = tarefas_para_executar[i];
                        execvp(t->programa, t->argumentos);
                        perror("Erro ao executar programa no modo paralelo");
                        exit(EXIT_FAILURE); 
                    } 
                    else {
                        pids[processos_criados] = pid;
                        processos_criados++;
                    }
                }

                for (int i = 0; i < processos_criados; i++) {
                    int status;
                    if (waitpid(pids[i], &status, 0) == -1) {
                        perror("Erro ao aguardar processo filho no modo paralelo");
                    }
                }

            } 
            else if (strcmp(token_temp, "pipe") == 0) {
                Task *tarefas_para_executar[16];
                int count_run = 0;
                int erro_validacao = 0;

                char *nome_tarefa = strtok(NULL, " \t\n");
                if (nome_tarefa == NULL) {
                    printf("Erro: Nenhuma tarefa informada após 'run pipe'.\n");
                    continue;
                }

                while (nome_tarefa != NULL && count_run < 16) {
                    int j = buscar_tarefa(tarefas, qnt_task, nome_tarefa);
                    
                    if (j == -1) {
                        printf("Erro: Tarefa '%s' não foi encontrada. Cancelando execução do pipe.\n", nome_tarefa);
                        erro_validacao = 1;
                        break; 
                    }

                    tarefas_para_executar[count_run] = &tarefas[j];
                    count_run++;

                    nome_tarefa = strtok(NULL, " \t\n");
                }

                if (nome_tarefa != NULL && count_run == 16) {
                    printf("Erro: Limite máximo de 16 tarefas simultâneas excedido. Cancelando execução do pipe.\n");
                    erro_validacao = 1;
                }

                if (erro_validacao) {
                    continue;
                }

                int pipes[15][2];
                int pipes_criados = 0;

                for (int i = 0; i < count_run - 1; i++) {
                    if (pipe(pipes[i]) < 0) {
                        perror("Erro ao criar pipe");
                        erro_validacao = 1;
                        break;
                    }
                    pipes_criados++;
                }

                if (erro_validacao) {
                    for (int k = 0; k < pipes_criados; k++) {
                        close(pipes[k][0]);
                        close(pipes[k][1]);
                    }
                    continue;
                }

                pid_t pids[16];
                int processos_criados = 0;
                int erro_fork = 0;

                for (int i = 0; i < count_run; i++) {
                    pid_t pid = fork();

                    if (pid < 0) {
                        perror("Erro ao executar fork no modo pipe");
                        erro_fork = 1;
                        break;
                    } 
                    else if (pid == 0) { 
                        if (i > 0) {
                            if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                                perror("Erro ao redirecionar STDIN");
                                exit(EXIT_FAILURE);
                            }
                        }

                        if (i < count_run - 1) {
                            if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                                perror("Erro ao redirecionar STDOUT");
                                exit(EXIT_FAILURE);
                            }
                        }

                        for (int j = 0; j < pipes_criados; j++) {
                            close(pipes[j][0]);
                            close(pipes[j][1]);
                        }

                        Task *t = tarefas_para_executar[i];
                        execvp(t->programa, t->argumentos);
                        perror("Erro ao executar programa no modo pipe");
                        exit(EXIT_FAILURE);
                    } 
                    else { 
                        pids[processos_criados] = pid;
                        processos_criados++;
                    }
                }

                for (int i = 0; i < pipes_criados; i++) {
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }

                for (int i = 0; i < processos_criados; i++) {
                    int status;
                    if (waitpid(pids[i], &status, 0) == -1) {
                        perror("Erro ao aguardar processo filho no modo pipe");
                    }
                }

            } 
            else {
                char *nome_tarefa = token_temp;
                int i = buscar_tarefa(tarefas, qnt_task, nome_tarefa);
                if (i == -1) {
                    printf("Erro: Tarefa '%s' não encontrada.\n", nome_tarefa);
                    continue;
                }
                executar_tarefa(&tarefas[i]);
            } 
        } else {
            printf("Comando desconhecido: %s\n", token);
        }
    }
    return 0;
}