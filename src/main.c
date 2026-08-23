#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char nome[64];
    char programa[128];
    char *argumentos[32];
} Task;


int main(){
    char linha[256];
    Task t;
    while(1){
        printf("processflow> ");
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
            token = strtok(NULL, " \t\n");
            if (token != NULL) {
            strncpy(t.nome, token, 64);
            }

            token = strtok(NULL, " \t\n");
            if (token != NULL) {
            strncpy(t.programa, token, 128);
            }

            token = strtok(NULL, " \t\n");
            int i = 0;
            while(token != NULL && i<32) {
            t.argumentos[i] = token;
            i++;
            token = strtok(NULL, " \t\n");
            }
            t.argumentos[i] = NULL;

            printf("\n--- Tarefa Processada ---\n");
            printf("Nome da tarefa : %s\n", t.nome);
            printf("Programa       : %s\n", t.programa);
            printf("Argumentos     : ");
            for (int i = 0; t.argumentos[i] != NULL; i++) {
                printf("[%s] ", t.argumentos[i]);
            }
            printf("\n");
        } else {
        printf("Comando desconhecido: %s\n", token);
        }
    }
    return 0;
}