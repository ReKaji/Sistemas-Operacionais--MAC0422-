#ifndef EP1_H
#define EP1_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>

#define MAX_PROCESSOS 100000

//Struct que define os processos
typedef struct {
    char nome[32];
    int t0;
    int dt;
    int deadline;
    double tempo_restante; //variável usada no SRTN e no escalonamento com prioridades
    int threads_vistas; //variável utilizada no SRTN
} Processo;

// Variáveis globais
extern struct timeval inicio;

extern int num_processos;
extern Processo processos[MAX_PROCESSOS];
extern int num_cores;
extern int terminou;
extern int escalonador;
extern const char *arquivo_entrada;
extern const char *arquivo_saida;

extern int contador_FCFS;
extern int contador_SRTN;
extern int contador_prioridade;
extern int contador_processos_terminados;

extern int contador_processos_terminados;
extern int num_preempcoes;
extern int tamanho_prontos;
extern int num_cumpridos;

extern Processo prontos[MAX_PROCESSOS];
extern Processo chegou[MAX_PROCESSOS];

extern pthread_mutex_t mutex_contador_FCFS;
extern pthread_mutex_t mutex_arquivo_saida;
extern pthread_mutex_t mutex_processos;
extern pthread_mutex_t mutex_contador_SRTN;
extern pthread_mutex_t mutex_prontos;
extern pthread_mutex_t mutex_contador_terminados;
extern pthread_mutex_t mutex_num_preempcoes;
extern pthread_mutex_t mutex_num_cumpridos;

extern void *status;

// Funções auxiliares
int verifica_num_cores();
int comparar_por_chegada(const void *a, const void *b);
int ler_processos(Processo processos[]);

// Threads
void* FCFS(void *arg);
void* SRTN(void *arg);
void* adiciona_processos();
void* escalonamento_com_prioridade();

// Funções que ajudam a manipular a fila de prontos
void tira_processo(Processo array_processos[], int *tamanho);
void inserir_ordenado_por_dt(Processo array_processos[], int *tamanho, Processo novo);
void inserir_novo_processo(Processo array_processos[], int *tamanho, Processo novo);

//Função utilizada para calcular a prioridade de um processo, no escalonador com prioridade
int calcula_prioridade(Processo p);

#endif 
