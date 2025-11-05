#ifndef EP3_H
#define EP3_H

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_LINHA 63
#define MAX_LINHAS 4099
#define OFFSET_INICIAL 15

extern int n;
extern const char *arquivo_entrada;
extern const char *arquivo_saida;
extern const char *arquivo_trace;
extern int num_erros;
extern FILE *saida;
extern int offset_nf;
extern int linha_nf;
extern int indice_nf;

void copia_arquivo();
int first_fit(int num_ua);
int next_fit(int num_ua);
int best_fit(int num_ua);
int worst_fit(int num_ua);
void compactar_memoria();
int escolher_estrategia(int estrategia, int num_ua);
void init(int argc, char *argv[]);

#endif 
