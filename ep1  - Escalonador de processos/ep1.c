#include "ep1.h"

struct timeval inicio;

#define MAX_PROCESSOS 100000
int num_processos;
Processo processos[MAX_PROCESSOS];
int num_cores;

int contador_FCFS = 0;
int terminou =0;
int escalonador;
const char *arquivo_entrada;
const char *arquivo_saida;
pthread_mutex_t mutex_contador_FCFS = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_arquivo_saida = PTHREAD_MUTEX_INITIALIZER;
void *status;

//SRTN

pthread_mutex_t mutex_processos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_contador_SRTN = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_prontos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_contador_terminados = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_num_preempcoes = PTHREAD_MUTEX_INITIALIZER;
int contador_processos_terminados;
int contador_SRTN = 0;
int num_preempcoes = 0;
int tamanho_prontos = 0;
Processo prontos[MAX_PROCESSOS];
Processo chegou[MAX_PROCESSOS];

//Escalonamento com prioridade

int contador_prioridade=0;

int num_cumpridos=0;
pthread_mutex_t mutex_num_cumpridos = PTHREAD_MUTEX_INITIALIZER;

int verifica_num_cores() {
    return sysconf(_SC_NPROCESSORS_ONLN);  // Número de núcleos disponíveis
}

int comparar_por_chegada(const void *a, const void *b) {
    const Processo *p1 = (const Processo *)a;
    const Processo *p2 = (const Processo *)b;
    return p1->t0 - p2->t0;
}


int ler_processos(Processo processos[]) {
    FILE *arquivo = fopen(arquivo_entrada, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    int num_processos = 0;
    while (fscanf(arquivo, "%s %d %d %d",
                  processos[num_processos].nome,
                  &processos[num_processos].t0,
                  &processos[num_processos].dt,
                  &processos[num_processos].deadline) == 4) {
        processos[num_processos].tempo_restante = (double) processos[num_processos].dt;
        processos[num_processos].threads_vistas =0;
        num_processos++;
    }
    fclose(arquivo);
    return num_processos;
}



void* FCFS(void *arg){
    struct timeval agora;
    struct timeval inicio_processo;
    while (1) {
        pthread_mutex_lock(&mutex_contador_FCFS);
        if (contador_FCFS >= num_processos) {
            pthread_mutex_unlock(&mutex_contador_FCFS);
            break;
        }
        Processo processo_atual = processos[contador_FCFS];
        contador_FCFS++;
        pthread_mutex_unlock(&mutex_contador_FCFS);
        
        gettimeofday(&agora, NULL);
        double tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;

        while (processo_atual.t0 > tempo_atual){
            usleep(1000);
            gettimeofday(&agora, NULL);
            tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
        }

        gettimeofday(&inicio_processo, NULL);
        double tempo_inicio_processo = (inicio_processo.tv_sec - inicio.tv_sec) + (inicio_processo.tv_usec - inicio.tv_usec) / 1000000.0;

        while (tempo_atual <= tempo_inicio_processo + processo_atual.dt){
            gettimeofday(&agora, NULL);
            tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
        }
        
        
        gettimeofday(&agora, NULL);
        tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
        double tf = tempo_atual;
    
        double tr = tf - processo_atual.t0;

        int cumpriu = (tf <= processo_atual.deadline);
        if (cumpriu){
            pthread_mutex_lock(&mutex_num_cumpridos);
            num_cumpridos++;
            pthread_mutex_unlock(&mutex_num_cumpridos);


        }
        pthread_mutex_lock(&mutex_arquivo_saida);
        FILE *arquivo_s = fopen(arquivo_saida, "a");
        if (arquivo_s) {
            fprintf(arquivo_s, "%s %3f %3f %d\n", processo_atual.nome, tr, tf, cumpriu);
            fclose(arquivo_s);
        }
        pthread_mutex_unlock(&mutex_arquivo_saida);
    }
    return NULL;
}

//SRTN


void tira_processo(Processo array_processos[], int *tamanho){
    for (int i = 0; i< *tamanho-1 ; i++){
        array_processos[i] = array_processos[i+1];
    }
    (*tamanho)--;
}

void inserir_ordenado_por_dt(Processo array_processos[],int *tamanho,Processo novo) {
    int i = *tamanho;

    while (i > 0 && array_processos[i - 1].dt > novo.dt) {
        array_processos[i] = array_processos[i - 1];
        i--;
    }

    array_processos[i] = novo;
    (*tamanho)++;
}

void* SRTN(void *arg){
    struct timeval agora;
    struct timeval inicio_processo;
    int tenho_processo; 
    Processo processo_atual;
    double tempo_atual;
    while (1){
        tenho_processo =0;
        pthread_mutex_lock(&mutex_prontos);
        if (tamanho_prontos>=1){
            processo_atual = prontos[0];
            tira_processo(prontos, &tamanho_prontos);
            pthread_mutex_unlock(&mutex_prontos);
            tenho_processo=1;
        }
        else{
            pthread_mutex_unlock(&mutex_prontos);
            pthread_mutex_lock(&mutex_processos);
            if (contador_SRTN<num_processos){
                gettimeofday(&agora, NULL);
                tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                if (processos[contador_SRTN].t0 <= tempo_atual ){
                    processo_atual = processos[contador_SRTN];
                    contador_SRTN++;                              
                    tenho_processo=1;
                }
            }
            pthread_mutex_unlock(&mutex_processos); 
        }
        if (tenho_processo){
            while (1){
                gettimeofday(&inicio_processo, NULL);
                double tempo_inicio_processo = (inicio_processo.tv_sec - inicio.tv_sec) + (inicio_processo.tv_usec - inicio.tv_usec) / 1000000.0;

                while (tempo_atual <= tempo_inicio_processo + 0.1){
                    gettimeofday(&agora, NULL);
                    tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                }

               
                processo_atual.tempo_restante -=0.1;
                if (processo_atual.tempo_restante<=0+0.00001){
                    gettimeofday(&agora, NULL);
                    tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                    double tf = tempo_atual;
    
                    double tr = tf - processo_atual.t0;

                    int cumpriu = (tf <= processo_atual.deadline);
                    if (cumpriu){
                        pthread_mutex_lock(&mutex_num_cumpridos);
                        num_cumpridos++;
                        pthread_mutex_unlock(&mutex_num_cumpridos);


                    }
                    pthread_mutex_lock(&mutex_arquivo_saida);
                    FILE *arquivo_s = fopen(arquivo_saida, "a");
                    if (arquivo_s) {
                        fprintf(arquivo_s, "%s %3f %3f %d\n", processo_atual.nome, tr, tf, cumpriu);
                        fclose(arquivo_s);
                        
                    }
                    pthread_mutex_unlock(&mutex_arquivo_saida);

                    pthread_mutex_lock(&mutex_contador_terminados);
                    contador_processos_terminados++;
                    pthread_mutex_unlock(&mutex_contador_terminados);
                    tenho_processo=0;
                    break;
                }
                else{
                    while (1){
                        pthread_mutex_lock(&mutex_processos);
                        if (contador_SRTN<num_processos){
                            gettimeofday(&agora, NULL);
                            tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                            if (processos[contador_SRTN].t0 <= tempo_atual ){

                                Processo chegou = processos[contador_SRTN];

                                if (chegou.dt < processo_atual.tempo_restante){

                                    pthread_mutex_lock(&mutex_prontos);
                                    inserir_ordenado_por_dt(prontos, &tamanho_prontos, processo_atual);
                                    pthread_mutex_unlock(&mutex_prontos);

                                    processo_atual = chegou;
                                    contador_SRTN++;     
                                    
                                    pthread_mutex_lock(&mutex_num_preempcoes);
                                    num_preempcoes++;
                                    pthread_mutex_unlock(&mutex_num_preempcoes);
                                    
                                    
                                }
                                else{
                                    processos[contador_SRTN].threads_vistas++;
                                    if (processos[contador_SRTN].threads_vistas == num_cores) {
                                        
                                        pthread_mutex_lock(&mutex_prontos);
                                        inserir_ordenado_por_dt(prontos, &tamanho_prontos, chegou);
                                        pthread_mutex_unlock(&mutex_prontos);
                                        contador_SRTN++;
                                    }
                                }
                                pthread_mutex_unlock(&mutex_processos);     
                            }
                            else{
                                pthread_mutex_unlock(&mutex_processos);     
                                break;
                            }
                        }
                        else{
                            pthread_mutex_unlock(&mutex_processos);     
                            break;
                        }
                    }
                }
            }
        }
        else{
            if (contador_processos_terminados==num_processos) break;
            usleep(1000);
        }   
    }
    return NULL;
}

//Escalonamento com prioridade 

void inserir_novo_processo(Processo array_processos[],int *tamanho,Processo novo) {
    int i = *tamanho;
    array_processos[i] = novo;
    (*tamanho)++;
}

void* adiciona_processos(){
    double tempo_atual;
    struct timeval agora;
    while(1){
        gettimeofday(&agora, NULL);
        tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
        if (contador_prioridade<num_processos){
            if (processos[contador_prioridade].t0 <= tempo_atual){
                pthread_mutex_lock(&mutex_prontos);
                inserir_novo_processo(prontos, &tamanho_prontos, processos[contador_prioridade]);
                pthread_mutex_unlock(&mutex_prontos);
                contador_prioridade++;
            }
            else 
            {
                if(terminou) break;
                usleep(10000);
            }
        }
        else{
            break;
        }
    }
    return NULL;
}

int calcula_prioridade(Processo p){
    double tempo_atual;
    int prioridade;
    struct timeval agora;
    gettimeofday(&agora, NULL);
    tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;

    double tempo_ate_deadline = p.deadline - tempo_atual;
    double tempo_pode_desperdicar = tempo_ate_deadline - p.tempo_restante;

    if (tempo_pode_desperdicar <= 0) prioridade=1;
    else{
        double prioridade_real = p.tempo_restante / fmax(0.01, tempo_pode_desperdicar/200);
        prioridade = (int)(fmax(1.0, prioridade_real) + 0.5);  
    }
    return prioridade;
}

void* escalonamento_com_prioridade(){
    struct timeval agora;
    struct timeval inicio_processo;
    int tenho_processo; 
    Processo processo_atual;
    double tempo_atual;

    while(1){
        tenho_processo=0;
        pthread_mutex_lock(&mutex_prontos);
        if (tamanho_prontos>=1){
            processo_atual = prontos[0];
            tira_processo(prontos, &tamanho_prontos);
            tenho_processo=1;
        }
        pthread_mutex_unlock(&mutex_prontos);

        if (tenho_processo){
            while(1){
                int prioridade = calcula_prioridade(processo_atual);
                
                
                for (int i =0; i< prioridade;i++){
                    gettimeofday(&inicio_processo, NULL);
                    double tempo_inicio_processo = (inicio_processo.tv_sec - inicio.tv_sec) + (inicio_processo.tv_usec - inicio.tv_usec) / 1000000.0;

                    while (tempo_atual <= tempo_inicio_processo + 0.01){
                        gettimeofday(&agora, NULL);
                        tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                    }
                    processo_atual.tempo_restante -=(0.01);
                    if (processo_atual.tempo_restante<=0+0.00001){
                        gettimeofday(&agora, NULL);
                        tempo_atual = (agora.tv_sec - inicio.tv_sec) + (agora.tv_usec - inicio.tv_usec) / 1000000.0;
                        double tf = tempo_atual;
        
                        double tr = tf - processo_atual.t0;

                        int cumpriu = (tf <= processo_atual.deadline);

                        if (cumpriu){
                            pthread_mutex_lock(&mutex_num_cumpridos);
                            num_cumpridos++;
                            pthread_mutex_unlock(&mutex_num_cumpridos);


                        }
                        pthread_mutex_lock(&mutex_arquivo_saida);
                        FILE *arquivo_s = fopen(arquivo_saida, "a");
                        if (arquivo_s) {
                            fprintf(arquivo_s, "%s %3f %3f %d\n", processo_atual.nome, tr, tf, cumpriu);
                            fclose(arquivo_s);
                            
                        }
                        pthread_mutex_unlock(&mutex_arquivo_saida);

                        pthread_mutex_lock(&mutex_contador_terminados);
                        contador_processos_terminados++;
                        pthread_mutex_unlock(&mutex_contador_terminados);
                        tenho_processo=0;
                        break;
                    }  
                }
                if(tenho_processo){
                   
                    pthread_mutex_lock(&mutex_num_preempcoes);
                    num_preempcoes++;
                    pthread_mutex_unlock(&mutex_num_preempcoes);

                    pthread_mutex_lock(&mutex_prontos);
                    inserir_novo_processo(prontos, &tamanho_prontos, processo_atual);
                    if (tamanho_prontos>=1){
                        processo_atual = prontos[0];
                        tira_processo(prontos, &tamanho_prontos);
                    }
                    pthread_mutex_unlock(&mutex_prontos); 
                }
                else{
                    break;
                }
            }   
        }
        else{
            if (contador_processos_terminados==num_processos) {
                terminou=1;
                break;
            }
            usleep(1000);
        }   
    }
    return NULL;
}





void init(int argc, char *argv[]){
   
    sscanf(argv[1], "%d", &escalonador);  
    arquivo_entrada = argv[2];  
    arquivo_saida = argv[3];     
};

int main(int argc, char *argv[]){
    
    init(argc, argv);
    num_processos = ler_processos(processos);

    qsort(processos, num_processos, sizeof(Processo), comparar_por_chegada);

    num_cores = verifica_num_cores();
   
    if (escalonador == 1) {
        pthread_t threads[num_cores];

        
        gettimeofday(&inicio, NULL);
        for (int i = 0; i<num_cores; i++){
            pthread_create(&threads[i], NULL, FCFS, NULL); 
        }

        for(int i=0;i<num_cores;i++) {
            pthread_join(threads[i], &status);
        }
        terminou =1;
        FILE *arquivo_s = fopen(arquivo_saida, "a");
        fprintf(arquivo_s, "0\n");
        fclose(arquivo_s);
        //printf("%d\n",num_cumpridos);
        pthread_exit(NULL); 
    }

    if (escalonador == 2) {
        pthread_t threads[num_cores];

        
        gettimeofday(&inicio, NULL);
      
        for (int i = 0; i<num_cores; i++){
            pthread_create(&threads[i], NULL, SRTN, NULL); 
        }

        for(int i=0;i<num_cores;i++) {
            pthread_join(threads[i], &status);
        }
        
        FILE *arquivo_s = fopen(arquivo_saida, "a");
        fprintf(arquivo_s, "%d\n", num_preempcoes);
        fclose(arquivo_s);
        //printf("%d\n",num_cumpridos);
        pthread_exit(NULL); 
    }

    if (escalonador==3){
        pthread_t threads[num_cores+1];

        gettimeofday(&inicio, NULL);
        pthread_create(&threads[0], NULL, adiciona_processos, NULL);
        for (int i = 1; i<num_cores+1; i++){
            pthread_create(&threads[i], NULL, escalonamento_com_prioridade, NULL);
        }

        for(int i=0;i<num_cores+1;i++) {
            pthread_join(threads[i], &status);
        }
        
        FILE *arquivo_s = fopen(arquivo_saida, "a");
        fprintf(arquivo_s, "%d\n", num_preempcoes);
        fclose(arquivo_s);
        //printf("%d\n",num_cumpridos);
        pthread_exit(NULL); 
    }

    

    return 0;
}

