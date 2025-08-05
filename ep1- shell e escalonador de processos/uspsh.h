#ifndef USPSH_H
#define USPSH_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

// Tamanhos máximos
#define TAM_HOST 256
#define TAM_DIR 4096
#define TAM_COMANDO 256
#define MAX_PARAMETROS 10

// Variáveis globais
extern char nome_computador[TAM_HOST];
extern char diretorio_atual[TAM_DIR];
extern char comando[TAM_COMANDO];
extern char *parametros[MAX_PARAMETROS];

// Funções
int le_comando();
int verifica_comando_embutido();

#endif 
