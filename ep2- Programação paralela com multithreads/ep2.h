#ifndef EP2_H
#define EP2_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_D 2505
#define MAX_K 12500

typedef struct {
    int id;
    int pos_i;
    int pos_j;
    int voltas;
    int velocidade;
    int velocidade_sorteada;
    int ativo;
    float distancia;
    int pista_original;
} Ciclista;

// Variáveis globais:

//variáveis da pista e ciclistas
extern int pista[MAX_D][10];
extern Ciclista ciclistas[MAX_K];
extern int num_ciclistas;

//variáveis barreiras de sincronização
extern int continua[MAX_K];
extern int arrived[MAX_K];

//variaǘeis de entrada
extern int k;
extern int d;
extern char abordagem;
extern int debug;

//mutexes
extern pthread_mutex_t mutexes[10];
extern pthread_mutex_t mutex_numero_ciclistas;
extern pthread_mutex_t mutex_volta_eliminatoria_atual;
extern pthread_mutex_t mutex_numero_ciclistas_volta_eliminatoria_atual;
extern pthread_mutex_t mutex_pista_unico;
extern pthread_mutex_t mutex_volta_atual;
extern pthread_t threads[MAX_K];

//variáveis que auxiliam na lógica do código e nos prints
extern int volta_eliminatoria_atual;
extern long long tempo_decorrido;
extern int contador_chegada_volta[MAX_K];
extern int matrix_chegada_volta[MAX_K][MAX_K];
extern int numero_elimacoes;
extern int classificacao[MAX_K];
extern int quebrou[MAX_K];
extern int volta_quebrou[MAX_K];
extern int volta_atual;
extern int classificacao_volta_atual[2 * MAX_K][MAX_K];
extern int contador_volta_atual[MAX_K];
extern int finalizou_volta;
extern int ultima_linha_chegada[MAX_K];

// Funções definidas:

//inicializa os  mutexes (ciclista eficiente)
void inicializa_mutexes();

//sorteios da velocidade nova do ciclista
int sorteia_velocidade(int velocidade_inicial);

//sorteio para ver se o ciclista vai quebrar em voltas múltiplas de 5
int sorteia_desistencia();

//função para ser executada quando a velocidade do ciclista for 30
void velocidade_ciclista_30(Ciclista* c);

//função para ser executada quando o ciclista não estiver em sua pista de origem (tenta voltar à pista de origem)
void ciclista_pista_nao_original(Ciclista* c);

//função que verifica a velocidade do ciclista à frente
int velocidade_ciclista_frente(Ciclista* c);

//função para ser executada quando o ciclista à frente também estiver com velocidade 60
void andar_frente_60 (Ciclista* c);

//função que tenta a ultrapassagem pela direita
void tenta_ultrapassagem(Ciclista* c);

//função que executa o ciclista eficiente
void* ciclista_eficiente(void* arg);

//função que inicializa a pista (todos os valores = -1)
void inicializa_pista();

//função que printa a classificação final, ao final da prova
void printa_classificacao_final();

// função que printa a colocação de cada competidor após o fim de cada volta
void printa_finalizacao_volta();

//função que printa a situação da pista na saída de erro, quando houver o parâmetro -debug
void printa_pista();

//função que inicializa a posição dos ciclistas nas pistas e seus atributos
int inicializa_pista_ciclistas();

//função que executa o ciclista ineficiente
void* ciclista_ineficiente(void* arg);

// função responsável pela entidade central
int entidade_central();

//função que lê a entrada
void init(int argc, char *argv[]);

//main
int main(int argc, char *argv[]);

#endif // EP2_H