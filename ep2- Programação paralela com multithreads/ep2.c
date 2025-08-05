#include "ep2.h"



#define MAX_D 2505
#define MAX_K 12500




//variáveis da pista e ciclistas
int pista[MAX_D][10];
Ciclista ciclistas[MAX_K];
int num_ciclistas;

//variáveis barreiras de sincronização
int continua[MAX_K];
int arrived[MAX_K];

//variáveis de entrada
int k;
int d;
char abordagem;
int debug = 0;

//mutexes
pthread_mutex_t mutexes[10];
pthread_mutex_t mutex_numero_ciclistas = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_volta_eliminatoria_atual = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_numero_ciclistas_volta_eliminatoria_atual= PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_pista_unico = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_volta_atual = PTHREAD_MUTEX_INITIALIZER;
pthread_t threads[MAX_K]; 


//outras variáveis importantes
int volta_eliminatoria_atual = -1;
long long tempo_decorrido = 0;
int contador_chegada_volta[MAX_K] = {0};
int matrix_chegada_volta[MAX_K][MAX_K] = {0};
int numero_elimacoes=-1;



//variáveis usadas para impressão
int classificacao[MAX_K] = {0};
int quebrou[MAX_K] = {0};
int volta_quebrou[MAX_K] = {0};
int volta_atual = -1;
int classificacao_volta_atual[2 * MAX_K][MAX_K] = {0};
int contador_volta_atual[MAX_K] = {0};
int finalizou_volta = 0;
int ultima_linha_chegada[MAX_K] = {0};

void inicializa_mutexes() {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
}

int sorteia_velocidade(int velocidade_inicial){
    int velocidade_final;
    int numero_sorteado = rand();
    if (velocidade_inicial == 30) {
       numero_sorteado = numero_sorteado % 4;
       if (numero_sorteado <=2){
            velocidade_final = 60;
       }
       else{
            velocidade_final = 30;
       }
    }
    else{
        numero_sorteado = numero_sorteado % 20;
        if (numero_sorteado <= 10){
            velocidade_final = 30;
        }
        else{
            velocidade_final = 60;
        }
    }
    return velocidade_final;
}

int sorteia_desistencia(){
    int numero_sorteado = rand() %10;
    if (numero_sorteado == 0){
        return 0;
    }
    else{
        return 1;
    } 
}

void velocidade_ciclista_30(Ciclista* c){
    c->distancia+=0.5;    
    if (c->distancia>=c->pos_i+1){ 
        pthread_mutex_lock(&mutexes[c->pos_j]);
        if (c->pos_i < MAX_D -1 && pista[c->pos_i + 1][c->pos_j] == -1){
            pista[c->pos_i + 1][c->pos_j] = c->id;
            pista[c->pos_i][c->pos_j] = -1;
            c->pos_i += 1;
        }
        pthread_mutex_unlock(&mutexes[c->pos_j]);
    }
}

void ciclista_pista_nao_original(Ciclista* c){
    if (c->pos_j>0){
        int maior = c->pos_j;
        int menor = c->pos_j-1;
        pthread_mutex_lock(&mutexes[menor]);
        pthread_mutex_lock(&mutexes[maior]);
        if (c->pos_i <MAX_D-1 && c->pos_j >0){
            if (pista[c->pos_i+1][c->pos_j-1] == -1){
                c->distancia+=1;
                c->velocidade = 60;
                

              
                pista[c->pos_i][c->pos_j] = -1;
                
                pista[c->pos_i+1][c->pos_j-1] = c->id;
                c->pos_j -= 1;
                c->pos_i += 1;
            }
        }
        pthread_mutex_unlock(&mutexes[maior]);
        pthread_mutex_unlock(&mutexes[menor]);
    }
}

int velocidade_ciclista_frente(Ciclista* c){
    int velocidade_frente = 60;
    if (c->pos_i <MAX_D-1){
        pthread_mutex_lock(&mutexes[c->pos_j]);
        int id_ciclista_frente = pista[c->pos_i + 1][c->pos_j];
        pthread_mutex_unlock(&mutexes[c->pos_j]);

        

        if (id_ciclista_frente != -1){
            velocidade_frente = ciclistas[id_ciclista_frente].velocidade;
        }
        else{
            velocidade_frente = 60;
        }
    }
    return velocidade_frente;

}

void andar_frente_60 (Ciclista* c){
    c->distancia+=1;
    c->velocidade = 60;
    pthread_mutex_lock(&mutexes[c->pos_j]);
    if (c->pos_i < MAX_D -1) {
        pista[c->pos_i + 1][c->pos_j] = c->id;
        pista[c->pos_i][c->pos_j] = -1;
        c->pos_i += 1;
    }
    pthread_mutex_unlock(&mutexes[c->pos_j]);
}
void tenta_ultrapassagem(Ciclista* c){
    int maior = c->pos_j+1;
    int menor = c->pos_j;
    pthread_mutex_lock(&mutexes[menor]);
    pthread_mutex_lock(&mutexes[maior]);
    if (pista[c->pos_i + 1][c->pos_j+1] == -1){

        c->distancia+=1;
        c->velocidade = 60;
        

        
        pista[c->pos_i][c->pos_j] = -1;
       
        

        pista[c->pos_i + 1][c->pos_j + 1] = c->id;
        c->pos_i += 1;
        c->pos_j += 1;
        
    }
    else{
      
        c->distancia+=0.5;
        c->velocidade = 30;
        if (c->distancia>=c->pos_i+1){ 
            
            if (c->pos_i < MAX_D -1 && pista[c->pos_i + 1][c->pos_j] == -1){
                pista[c->pos_i + 1][c->pos_j] = c->id;
                pista[c->pos_i][c->pos_j] = -1;
                c->pos_i += 1;
            }
           
        }
    }
    pthread_mutex_unlock(&mutexes[maior]);
    pthread_mutex_unlock(&mutexes[menor]);
}

void* ciclista_eficiente(void* arg) {
    Ciclista* c = (Ciclista*) arg;
    
    while (c->ativo) {
        pthread_mutex_lock(&mutex_numero_ciclistas);
        if (num_ciclistas == 1){
            classificacao[c->id]=1;
          
            arrived[c->id] = 1;
            
            c->ativo = 0;
            
            pthread_exit(NULL);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_numero_ciclistas);

        
        c->velocidade = 30;
        if (c->velocidade_sorteada == 30){
            velocidade_ciclista_30(c);
        }


        else{
            if (c->pos_j != c->pista_original){
                ciclista_pista_nao_original(c);
            }
            
            else{
                int velocidade_frente = velocidade_ciclista_frente(c);
                if (velocidade_frente == 60){
                    andar_frente_60(c);
                }
                else{
                    if (c->pos_j <9 && c->pos_i < MAX_D -1){
                        tenta_ultrapassagem(c);
                    

                    }
                }
            }
            
            
        }
        if (c->distancia >= d){
            c->voltas++;
            c->distancia = 0;
            ultima_linha_chegada[c->id] = tempo_decorrido;
            c->velocidade_sorteada = sorteia_velocidade(c->velocidade);

            pthread_mutex_lock(&mutexes[c->pos_j]);
            pista[c->pos_i][c->pos_j] = -1;
            c->pos_i = 0;
            pista[c->pos_i][c->pos_j] = c->id;
            pthread_mutex_unlock(&mutexes[c->pos_j]);

            pthread_mutex_lock(&mutex_volta_eliminatoria_atual);
            if (c->voltas % 2 == 0){
                int volta = (c->voltas/2)-1;
                if (volta > volta_eliminatoria_atual){
                    volta_eliminatoria_atual = volta;
                }
                contador_chegada_volta[volta]++;
                matrix_chegada_volta[volta][c->id] = contador_chegada_volta[volta];
            }
            pthread_mutex_unlock(&mutex_volta_eliminatoria_atual);

            




            if (c->voltas % 5 == 0){
                c->ativo = sorteia_desistencia();
                if (c->ativo == 0){
                    pthread_mutex_lock(&mutex_volta_eliminatoria_atual);
                    for (int i = 0; i < volta_eliminatoria_atual+1; i++) {
                        matrix_chegada_volta[i][c->id] =0;
                    }
                    pthread_mutex_unlock(&mutex_volta_eliminatoria_atual);


                    pthread_mutex_lock(&mutexes[c->pos_j]);
                    pista[c->pos_i][c->pos_j] = -1;
                    pthread_mutex_unlock(&mutexes[c->pos_j]);

                    pthread_mutex_lock(&mutex_numero_ciclistas);
                    classificacao[c->id] = num_ciclistas;
                    num_ciclistas--;
                    pthread_mutex_unlock(&mutex_numero_ciclistas);

                    quebrou[c->id] = 1;
                    volta_quebrou[c->id] = c->voltas;
                    printf("\n");
                    printf("Ciclista %d QUEBROU na volta %d\n", c->id, c->voltas);
                    printf("\n");
                    arrived[c->id] = 1;
                    pthread_exit(NULL);
                    return NULL;
                }
            }

            pthread_mutex_lock(&mutex_volta_atual);
            contador_volta_atual[c->voltas-1]++;
            classificacao_volta_atual[c->voltas-1][c->id] = contador_volta_atual[c->voltas-1];
            if (contador_volta_atual[c->voltas -1] ==num_ciclistas){
                finalizou_volta = 1;
                volta_atual++;
            }
            pthread_mutex_unlock(&mutex_volta_atual);
        }

        
        arrived[c->id] = 1;
        while (continua[c->id] == 0) {
            usleep(1);
        }
        continua[c->id] = 0;
       

    }
    
    pthread_exit(NULL);
    return NULL;
}

void inicializa_pista() {
    for (int i = 0; i < MAX_D; i++) {
        for (int j = 0; j < 10; j++) {
            pista[i][j] = -1; // Inicializa cada posição da pista com -1
        }
    }
}

void printa_classificacao_final(){
    printf("\n");
    printf("FINAL DE PROVA:\n");
    printf("\n");
    for (int i =0; i<k; i++){
        if (quebrou[i] == 0)printf("Ciclista %d - posição: %d - última vez que passou na linha de chegada: %dms\n", i, classificacao[i], ultima_linha_chegada[i]);
        else{
            printf("Ciclista %d QUEBROU na volta %d - última vez que passou na linha de chegada: %dms\n", i, volta_quebrou[i], ultima_linha_chegada[i]);
        }
    }
}

void printa_finalizacao_volta(){
    printf("\n");
    printf("Volta %d finalizada:\n", volta_atual+1);
    printf("\n");
    for (int i = 0; i < k; i++) {
        if (ciclistas[i].ativo){
            printf("Ciclista %d terminou a volta %d na posição %d\n", ciclistas[i].id, volta_atual+1, classificacao_volta_atual[volta_atual][ciclistas[i].id]);
        }
    }
    finalizou_volta = 0;
}

void printa_pista(){
    fprintf(stderr, "\n");
    for (int i = d-1; i>=0 ; i--){
        for (int j = 0; j<10;j++){
            if (pista[i][j] == -1){
                fprintf(stderr, ". ");
               
            }
            else{
                fprintf(stderr, "%d ", pista[i][j]);
                
            }
        }
        fprintf(stderr, "\n");
        

    }
    return;
}
int inicializa_pista_ciclistas(){
    for (int i = 0; i < k; i++) {
        ciclistas[i].id = i;
        ciclistas[i].pos_i = d-1-(i/5); 
        ciclistas[i].pos_j = i % 5; 
        ciclistas[i].voltas = 0;
        ciclistas[i].velocidade_sorteada = 30; 
        ciclistas[i].velocidade = 30; 
        ciclistas[i].ativo = 1;
        ciclistas[i].distancia = -i/5; 
        ciclistas[i].pista_original = i%5; 
        pista[ciclistas[i].pos_i][ciclistas[i].pos_j] = ciclistas[i].id;
    }
    num_ciclistas = k;
    return 0;
}

void* ciclista_ineficiente(void* arg) {
    Ciclista* c = (Ciclista*) arg;
    
    while (c->ativo) {
        pthread_mutex_lock(&mutex_numero_ciclistas);
        if (num_ciclistas == 1){
            classificacao[c->id]=1;
            
            arrived[c->id] = 1;
            c->ativo = 0;
            pthread_exit(NULL);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_numero_ciclistas);

        
        c->velocidade = 30;
        if (c->velocidade_sorteada == 30){
            c->distancia+=0.5;    
            if (c->distancia>=c->pos_i+1){ 
                pthread_mutex_lock(&mutex_pista_unico);
                if (c->pos_i < MAX_D -1 && pista[c->pos_i + 1][c->pos_j] == -1){
                    pista[c->pos_i + 1][c->pos_j] = c->id;
                    pista[c->pos_i][c->pos_j] = -1;
                    c->pos_i += 1;
                }
                pthread_mutex_unlock(&mutex_pista_unico);
            }
        }


        else{
            if (c->pos_j != c->pista_original){
                if (c->pos_j>0){
                    int maior = c->pos_j;
                    int menor = c->pos_j-1;
                    pthread_mutex_lock(&mutex_pista_unico);
                    
                    if (c->pos_i <MAX_D-1 && c->pos_j >0){
                        if (pista[c->pos_i+1][c->pos_j-1] == -1){
                            c->distancia+=1;
                            c->velocidade = 60;
      
            
                          
                            pista[c->pos_i][c->pos_j] = -1;
                            
                            pista[c->pos_i+1][c->pos_j-1] = c->id;
                            c->pos_j -= 1;
                            c->pos_i += 1;
                        }
                    }
                    pthread_mutex_unlock(&mutex_pista_unico);
                   
                }
            }
            
            else{
                int velocidade_frente = velocidade_ciclista_frente(c);
                if (velocidade_frente == 60){
                    c->distancia+=1;
                    c->velocidade = 60;
                    pthread_mutex_lock(&mutex_pista_unico);
                    if (c->pos_i < MAX_D -1) {
                        pista[c->pos_i + 1][c->pos_j] = c->id;
                        pista[c->pos_i][c->pos_j] = -1;
                        c->pos_i += 1;
                    }
                    pthread_mutex_unlock(&mutex_pista_unico);
                }
                else{
                    if (c->pos_j <9 && c->pos_i < MAX_D -1){
                        int maior = c->pos_j+1;
                    int menor = c->pos_j;
                    pthread_mutex_lock(&mutex_pista_unico);
                    
                    if (pista[c->pos_i + 1][c->pos_j+1] == -1){

                        c->distancia+=1;
                        c->velocidade = 60;
          

                        
                        pista[c->pos_i][c->pos_j] = -1;
                    
                        

                        pista[c->pos_i + 1][c->pos_j + 1] = c->id;
                        c->pos_i += 1;
                        c->pos_j += 1;
                        
                    }
                    else{
                    
                        c->distancia+=0.5;
                        c->velocidade = 30;
                        if (c->distancia>=c->pos_i+1){ 
                            
                            if (c->pos_i < MAX_D -1 && pista[c->pos_i + 1][c->pos_j] == -1){
                                pista[c->pos_i + 1][c->pos_j] = c->id;
                                pista[c->pos_i][c->pos_j] = -1;
                                c->pos_i += 1;
                            }
                        
                        }
                    }
                    pthread_mutex_unlock(&mutex_pista_unico);
                    
                    

                    }
                }
            }
            
            
        }
        if (c->distancia >= d){
            c->voltas++;
            c->distancia = 0;
            ultima_linha_chegada[c->id] = tempo_decorrido;
            c->velocidade_sorteada = sorteia_velocidade(c->velocidade);

            pthread_mutex_lock(&mutex_pista_unico);
            pista[c->pos_i][c->pos_j] = -1;
            c->pos_i = 0;
            pista[c->pos_i][c->pos_j] = c->id;
            pthread_mutex_unlock(&mutex_pista_unico);

            pthread_mutex_lock(&mutex_volta_eliminatoria_atual);
            if (c->voltas % 2 == 0){
                int volta = (c->voltas/2)-1;
                if (volta > volta_eliminatoria_atual){
                    volta_eliminatoria_atual = volta;
                }
                contador_chegada_volta[volta]++;
                matrix_chegada_volta[volta][c->id] = contador_chegada_volta[volta];
            }
            pthread_mutex_unlock(&mutex_volta_eliminatoria_atual);

            if (c->voltas % 5 == 0){
                c->ativo = sorteia_desistencia();
                if (c->ativo == 0){
                    pthread_mutex_lock(&mutex_volta_eliminatoria_atual);
                    for (int i = 0; i < volta_eliminatoria_atual+1; i++) {
                        matrix_chegada_volta[i][c->id] =0;
                    }
                    pthread_mutex_unlock(&mutex_volta_eliminatoria_atual);


                    pthread_mutex_lock(&mutex_pista_unico);
                    pista[c->pos_i][c->pos_j] = -1;
                    pthread_mutex_unlock(&mutex_pista_unico);


                    pthread_mutex_lock(&mutex_numero_ciclistas);
                    classificacao[c->id] = num_ciclistas;
                    num_ciclistas--;
                    pthread_mutex_unlock(&mutex_numero_ciclistas);

                    quebrou[c->id] = 1;
                    volta_quebrou[c->id] = c->voltas;
                    printf("\n");
                    printf("Ciclista %d QUEBROU na volta %d\n", c->id, c->voltas);
                    printf("\n");
                    arrived[c->id] = 1;
                    pthread_exit(NULL);
                    return NULL;
                }
            }
            pthread_mutex_lock(&mutex_volta_atual);
            contador_volta_atual[c->voltas-1]++;
            classificacao_volta_atual[c->voltas-1][c->id] = contador_volta_atual[c->voltas-1];
            if (contador_volta_atual[c->voltas -1] ==num_ciclistas){
             
                finalizou_volta = 1;
                volta_atual++;
            }
            pthread_mutex_unlock(&mutex_volta_atual);
        }

        
        arrived[c->id] = 1;
        while (continua[c->id] == 0) {
            usleep(1);
        }
        continua[c->id] = 0;
       

    }
    
    pthread_exit(NULL);
    return NULL;
}

int entidade_central(){

    for (int i = 0; i < k; i++) {
        arrived[i] = 0;
        continua[i] = 0;
    }

    if (abordagem == 'e'){
        //printf("Abordagem eficiente\n");
        for (int i = 0; i < k; i++) {
            pthread_create(&threads[i], NULL, ciclista_eficiente, (void*)&ciclistas[i]);
        }
    }
    else if (abordagem == 'i'){
        //printf("Abordagem ineficiente\n");
        for (int i = 0; i < k; i++) {
            pthread_create(&threads[i], NULL, ciclista_ineficiente, (void*)&ciclistas[i]);
        }
    }
    else{
        printf("Abordagem inválida\n");
        return -1;
    }

    pthread_mutex_lock(&mutex_numero_ciclistas);
    int n = num_ciclistas;
    pthread_mutex_unlock(&mutex_numero_ciclistas);
    while (n>1){
        for (int i = 0; i < k; i++) {
            while(arrived[i] == 0){
                usleep(1);
            }
            if (ciclistas[i].ativo) arrived[i] = 0;
        }

        
        pthread_mutex_lock(&mutex_volta_eliminatoria_atual);
        pthread_mutex_lock(&mutex_numero_ciclistas);
        if (volta_eliminatoria_atual > numero_elimacoes){
            for (int i = 0; i < k; i++) {
                if (num_ciclistas>1){
                    if (matrix_chegada_volta[numero_elimacoes+1][i] >= num_ciclistas){

                    
                        
                        pthread_mutex_lock(&mutexes[ciclistas[i].pos_j]);
                        pista[ciclistas[i].pos_i][ciclistas[i].pos_j] = -1;
                        pthread_mutex_unlock(&mutexes[ciclistas[i].pos_j]);
                        
                        classificacao[ciclistas[i].id] = num_ciclistas;
                        num_ciclistas--;
                        for (int j= numero_elimacoes+2; j< volta_eliminatoria_atual+1; j++){
                            matrix_chegada_volta[j][ciclistas[i].id] = 0;
                        }
                        ciclistas[i].ativo = 0;
                        
                        arrived[ciclistas[i].id] = 1;
                        numero_elimacoes++;
                        
                        

                        break;
                    }
                }
            } 
        }
        pthread_mutex_unlock(&mutex_numero_ciclistas);
        pthread_mutex_unlock(&mutex_volta_eliminatoria_atual);

        tempo_decorrido += 60;
        pthread_mutex_lock(&mutex_volta_atual);
        if(finalizou_volta == 1){
            printa_finalizacao_volta();
        }
        pthread_mutex_unlock(&mutex_volta_atual);

        if (debug == 1){
            printa_pista();
        }

        for (int i = 0; i < k; i++) {
            continua[i] = 1;
        }
        pthread_mutex_lock(&mutex_numero_ciclistas);
        n = num_ciclistas;
        pthread_mutex_unlock(&mutex_numero_ciclistas);
        
    }
    
    
    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL);
    }

   


    //terminou:
    printa_classificacao_final();

    return 0;
}


void init(int argc, char *argv[]){
   
    sscanf(argv[1], "%d", &d);
    sscanf(argv[2], "%d", &k);
    sscanf(argv[3], "%c", &abordagem);
     
    if (argc > 4) {
        
        debug = 1;
    }
};

int main(int argc, char *argv[]) {
    srand(time(NULL));
    init(argc, argv);
    
    inicializa_mutexes();
    inicializa_pista();
    inicializa_pista_ciclistas();
    entidade_central();
   

    return 0;
}