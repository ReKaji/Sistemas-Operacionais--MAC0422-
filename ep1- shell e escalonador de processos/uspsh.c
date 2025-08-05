#include "uspsh.h"

char nome_computador[256];
char diretorio_atual[4096];
char comando[256];
char *parametros[10];

int le_comando() {
    char prompt_text[5000];
    gethostname(nome_computador, sizeof(nome_computador));
    getcwd(diretorio_atual, sizeof(diretorio_atual));
    snprintf(prompt_text, sizeof(prompt_text), "[%s:%s]$ ", nome_computador, diretorio_atual);

    char *linha = readline(prompt_text);
    add_history(linha);  

    
    for (int i = 0; i < 10; i++) {
        parametros[i] = NULL;
    }

    char *par = strtok(linha, " "); 
    if (par != NULL) {
        strcpy(comando, par);  
        parametros[0] = comando;
    }

    int i = 1;
    while ((par = strtok(NULL, " ")) != NULL) {
        parametros[i++] = par;  
    }
    return 0;
}

int verifica_comando_embutido(){
    if (strcmp(comando, "cd") == 0) return 1;
    if (strcmp(comando, "whoami") == 0) return 1;
    if (strcmp(comando, "chmod") == 0) return 1;
    return 0;
}

int main(){
    while (1){
        le_comando();

        if (verifica_comando_embutido()){
            if (strcmp(comando, "cd") == 0) {
                if (chdir(parametros[1]) != 0) { 
                    perror("cd failed"); 
                }
            } 
            else if (strcmp(comando, "chmod") == 0) {
                mode_t permissao = strtol(parametros[1], NULL, 8); 
                if (chmod(parametros[2], permissao) != 0) {
                    perror("chmod failed");
                }
            }
            else if (strcmp(comando, "whoami") == 0) {
                for (int i =0; i< strlen(nome_computador);i++){
                    if (nome_computador[i]!='-') printf("%c", nome_computador[i]);
                    else {
                        printf("\n");
                        break;
                    }
                }
            }
        }
        else if (strcmp(comando, "exit")== 0){
            exit(0);
        }
        else {
            int status;
            if (fork()!= 0) {
                // Código do pai
                waitpid(-1, &status, 0);
            } else {
                // Código do filho
                execvp(comando, parametros);
            }
        }
    }
    return 0;
}
