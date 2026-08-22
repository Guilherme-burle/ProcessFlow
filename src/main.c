#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char comando[128];
    while(1){
        printf("processflow> ");
        fgets(comando, sizeof(comando), stdin);
        comando[strcspn(comando, "\n")] = '\0';
        if (strcmp(comando, "exit") == 0){
            break;
        }
    }
    return 0;
}