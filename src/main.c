#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    
#include <sys/wait.h>  
#include <sys/types.h> 

typedef struct{
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

void executar_tarefa(Task *t){
    pid_t pid = fork();
            if (pid < 0) {
                perror("Erro ao executar fork");
            } 
            else if (pid == 0) {
                execvp(t->programa, t->argumentos);
                perror("Erro ao executar programa com execvp");
                exit(EXIT_FAILURE);
            } 
            else{
                int status;
                waitpid(pid, &status, 0);
            }
}

int main(){
    char linha[256];
    Task tarefas[16];
    int qnt_task = 0;
    while(1){
        printf("processflow> ");
        fflush(stdout);
        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            break;
        }
        char *token = strtok(linha, " \t\n");
         if (token == NULL){
            printf("Nenhum comando digitado.\n");
            continue;
        }
        if (strcmp(token, "exit") == 0){
            break;
        }

        if (strcmp(token, "task") == 0){
            if (qnt_task >= 16) {
                printf("Erro: Limite máximo de tarefas atingido 16.\n");
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

            if (nome_token != NULL) {
            strncpy(t->nome, nome_token, 63);
            }
            t->nome[63] = '\0';

            if (prog_token != NULL) {
            strncpy(t->programa, prog_token, 127);
            }
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
            printf("\n");
        } else if (strcmp(token, "run") == 0) {
            
            char *token_temp = strtok(NULL, " \t\n");
            
            if (token_temp == NULL) {
                printf("Erro: Nome da tarefa não especificado. Uso: run <nome> OU run sequential <t1> <t2> ...\n");
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
                if (erro_validacao) {
                        continue;
                    }
                for (int i = 0; i < count_run; i++) {
                        executar_tarefa(tarefas_para_executar[i]);
                    }
            } else {
                char *nome_tarefa = token_temp;
                int i = buscar_tarefa(tarefas, qnt_task, nome_tarefa);
                if (i == -1) {
                    printf("Erro: Tarefa '%s' não encontrada.\n", nome_tarefa);
                    continue;
                }
                executar_tarefa(&tarefas[i]);
                } 
            }
            else {
                printf("Comando desconhecido: %s\n", token);
            }
    }
    return 0;
}