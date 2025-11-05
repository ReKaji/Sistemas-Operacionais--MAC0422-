#include "ep3.h"

#define TAMANHO_LINHA 63
#define MAX_LINHAS 4099
#define OFFSET_INICIAL 15
int n;
const char *arquivo_entrada;
const char *arquivo_saida;
const char *arquivo_trace;
int num_erros = 0;
FILE *saida;


void copia_arquivo() {
    FILE *entrada = fopen(arquivo_entrada, "r");
    if (entrada == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    saida = fopen(arquivo_saida, "w");
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        fclose(entrada);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    size_t bytes_lidos;

    while ((bytes_lidos = fread(buffer, 1, sizeof(buffer), entrada)) > 0) {
        fwrite(buffer, 1, bytes_lidos, saida);
    }

    fclose(entrada);
    fclose(saida);
    
}

int first_fit(int num_ua){
    if (num_ua ==0) return 0;
    saida = fopen(arquivo_saida, "r+"); 
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }
    long offset = 0;
    char buffer[128];
    int linha_atual = 0;

    while (linha_atual <  3){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        
        offset += strlen(buffer);
        linha_atual++;
    }
    
    int linha_comeco = 0;
    int tamanho_livre = 0;
    int primeiro_indice_livre = 0;
    while (linha_atual < MAX_LINHAS){

        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        

        for (int i = 0; i < TAMANHO_LINHA; i+=4) {
            if (buffer[i]=='2'){
                tamanho_livre++;
                if (tamanho_livre == 1) {
                    linha_comeco = linha_atual-3;
                    primeiro_indice_livre = i;
                }
                if (tamanho_livre == num_ua){
                    //printf("Linha %d, primeiro indice livre: %d\n", linha_comeco, primeiro_indice_livre);
                    for (int i = 0; i <linha_comeco; i++) {
                        offset+=64;
                    }

                    fseek(saida, offset, SEEK_SET);
                    if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                        fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                        fclose(saida);
                        exit(1);
                    }
                    fseek(saida, offset, SEEK_SET);
                    while (num_ua>0){
                        if (primeiro_indice_livre < TAMANHO_LINHA){
                            buffer[primeiro_indice_livre] = ' ';
                            buffer[primeiro_indice_livre+1] = ' ';
                            buffer[primeiro_indice_livre+2] = '0';
                            primeiro_indice_livre+=4;
                            num_ua--;
                        }
                        else{
                            fputs(buffer, saida);
                            primeiro_indice_livre = 0;
                            offset += 64;
                            fseek(saida, offset, SEEK_SET);
                            if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                                fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                                fclose(saida);
                                exit(1);
                            }
                            fseek(saida, offset, SEEK_SET);
                        }
                        if (num_ua == 0) {
                            fputs(buffer, saida);
                        }
                    }
                    fclose(saida);
                    return 0;
                }
            }
            else{
                tamanho_livre = 0;
            }
        }
        
        linha_atual++;
    }
    fclose(saida);
    return 1;
}

int worst_fit(int num_ua){
    saida = fopen(arquivo_saida, "r+"); 
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }
    long offset = 0;
    char buffer[128];
    int linha_atual = 0;

    while (linha_atual <  3){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
  
        offset += strlen(buffer);
        linha_atual++;
    }
    int linha_comeco = 0;
    int tamanho_livre = 0;
    int primeiro_indice_livre = 0;

    int maximo_tamanho_livre = 0;
    int linha_maximo_tamanho_livre = 0;
    int primeiro_indice_maximo_tamanho_livre = 0;

    while (linha_atual < MAX_LINHAS){

        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        for (int i = 0; i < TAMANHO_LINHA; i+=4) {
            if (buffer[i]=='2'){
                tamanho_livre++;
                if (tamanho_livre == 1) {
                    linha_comeco = linha_atual-3;
                    primeiro_indice_livre = i;
                }
                if (tamanho_livre > maximo_tamanho_livre){
                    maximo_tamanho_livre = tamanho_livre;
                    linha_maximo_tamanho_livre = linha_comeco;
                    primeiro_indice_maximo_tamanho_livre = primeiro_indice_livre;
                    //printf("Maximo tamanho livre atualizado: %d na linha %d, primeiro indice livre: %d\n", 
                      //     maximo_tamanho_livre, linha_maximo_tamanho_livre, primeiro_indice_maximo_tamanho_livre);
                }
            }
            else{
                tamanho_livre = 0;
            }
        }
        linha_atual++;
    }

    if (maximo_tamanho_livre < num_ua) {
        //printf("Memória insuficiente para alocar %d unidades de alocação\n", num_ua);
        fclose(saida);
        return 1; 
    }
    else{
        //printf("Linha %d, primeiro indice livre: %d\n", linha_maximo_tamanho_livre, primeiro_indice_maximo_tamanho_livre);
        //printf("Tamanho máximo livre: %d\n", maximo_tamanho_livre);
        for (int i = 0; i <linha_maximo_tamanho_livre; i++) {
            offset+=64;
        }

        fseek(saida, offset, SEEK_SET);
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        fseek(saida, offset, SEEK_SET);

        while (num_ua>0){
            //printf ("Alocando %d unidades de alocação\n", num_ua);
            if (primeiro_indice_maximo_tamanho_livre < TAMANHO_LINHA){
                buffer[primeiro_indice_maximo_tamanho_livre] = ' ';
                buffer[primeiro_indice_maximo_tamanho_livre+1] = ' ';
                buffer[primeiro_indice_maximo_tamanho_livre+2] = '0';
                primeiro_indice_maximo_tamanho_livre+=4;
                //printf("Alocando na linha %d, primeiro indice livre: %d\n", linha_maximo_tamanho_livre, primeiro_indice_maximo_tamanho_livre);
                //fputs(buffer, saida);
                num_ua--;
            }
            else{
                fputs(buffer, saida);
                primeiro_indice_maximo_tamanho_livre = 0;
                offset += 64;
                fseek(saida, offset, SEEK_SET);
                if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                    fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                    fclose(saida);
                    exit(1);
                }
                fseek(saida, offset, SEEK_SET);
            }
            if (num_ua == 0) {
                fputs(buffer, saida);
                
            }
        }
        fclose(saida);
        return 0; 
    }
}

int best_fit(int num_ua){
    saida = fopen(arquivo_saida, "r+"); 
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }
    long offset = 0;
    char buffer[128];
    int linha_atual = 0;

    while (linha_atual <  3){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
  
        offset += strlen(buffer);
        linha_atual++;
    }
    int linha_comeco = 0;
    int tamanho_livre = 0;
    int primeiro_indice_livre = 0;

    int minimo_tamanho_livre = 70000;
    int linha_minimo_tamanho_livre = 0;
    int primeiro_indice_minimo_tamanho_livre = 0;

    while (linha_atual < MAX_LINHAS){

        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        for (int i = 0; i < TAMANHO_LINHA; i+=4) {
            if (buffer[i]=='2'){
                tamanho_livre++;
                if (tamanho_livre == 1) {
                    linha_comeco = linha_atual-3;
                    primeiro_indice_livre = i;
                }
                if(linha_atual == 4098 && i == 60 && tamanho_livre < minimo_tamanho_livre && tamanho_livre >= num_ua){
                    minimo_tamanho_livre = tamanho_livre;
                    linha_minimo_tamanho_livre = linha_comeco;
                    primeiro_indice_minimo_tamanho_livre = primeiro_indice_livre;  
                }
            }
            else{
                if (tamanho_livre < minimo_tamanho_livre && tamanho_livre >= num_ua) {
                    minimo_tamanho_livre = tamanho_livre;
                    linha_minimo_tamanho_livre = linha_comeco;
                    primeiro_indice_minimo_tamanho_livre = primeiro_indice_livre;              
                }
                tamanho_livre = 0;
            }
        }
        
        linha_atual++;
    }

    if (minimo_tamanho_livre == 70000) {
        //printf("Memória insuficiente para alocar %d unidades de alocação\n", num_ua);
        fclose(saida);
        return 1; 
    }
    else{
        //printf("Linha %d, primeiro indice livre: %d\n", linha_minimo_tamanho_livre, primeiro_indice_minimo_tamanho_livre);
        //printf("Tamanho máximo livre: %d\n", minimo_tamanho_livre);
        for (int i = 0; i <linha_minimo_tamanho_livre; i++) {
            offset+=64;
        }

        fseek(saida, offset, SEEK_SET);
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        fseek(saida, offset, SEEK_SET);

        while (num_ua>0){
            //printf ("Alocando %d unidades de alocação\n", num_ua);
            if (primeiro_indice_minimo_tamanho_livre < TAMANHO_LINHA){
                buffer[primeiro_indice_minimo_tamanho_livre] = ' ';
                buffer[primeiro_indice_minimo_tamanho_livre+1] = ' ';
                buffer[primeiro_indice_minimo_tamanho_livre+2] = '0';
                primeiro_indice_minimo_tamanho_livre+=4;
                //printf("Alocando na linha %d, primeiro indice livre: %d\n", linha_minimo_tamanho_livre, primeiro_indice_minimo_tamanho_livre);
                //fputs(buffer, saida);
                num_ua--;
            }
            else{
                fputs(buffer, saida);
                primeiro_indice_minimo_tamanho_livre = 0;
                offset += 64;
                fseek(saida, offset, SEEK_SET);
                if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                    fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                    fclose(saida);
                    exit(1);
                }
                fseek(saida, offset, SEEK_SET);
            }
            if (num_ua == 0) {
                fputs(buffer, saida);
                
            }
        }
        fclose(saida);
        return 0; 
    }
}

int offset_nf = OFFSET_INICIAL; 
int linha_nf = 3;
int indice_nf = 0; 
int next_fit(int num_ua){
    if (num_ua ==0) return 0;

    //printf("Linha atual: %d, Indice atual: %d, Offset atual: %d\n", linha_nf, indice_nf, offset_nf);
    saida = fopen(arquivo_saida, "r+"); 
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }
    
    char buffer[128];
    int linha_atual = linha_nf;
    int offset = offset_nf;
    int indice_atual = indice_nf;
    int primeira_iteracao = 1;
    int linha_comeco = 0;
    int tamanho_livre = 0;
    int primeiro_indice_livre = 0;
    int i;
    fseek(saida, offset, SEEK_SET);
    while (linha_atual < MAX_LINHAS){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        
        if (primeira_iteracao) {
            i = indice_atual;
            primeira_iteracao = 0;}
        else {
            i = 0; 
        }
        while (i < TAMANHO_LINHA) {
            if (buffer[i]=='2'){
                tamanho_livre++;
                if (tamanho_livre == 1) {
                    linha_comeco = linha_atual-3;
                    primeiro_indice_livre = i;
                }
                if (tamanho_livre == num_ua){
                    //printf("Linha %d, primeiro indice livre: %d\n", linha_comeco, primeiro_indice_livre);
                    for (int i = linha_nf-3; i <linha_comeco; i++) {
                        offset+=64;
                    }

                    fseek(saida, offset, SEEK_SET);
                    if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                        fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                        fclose(saida);
                        exit(1);
                    }
                    fseek(saida, offset, SEEK_SET);
                    while (num_ua>0){
                        if (primeiro_indice_livre < TAMANHO_LINHA){
                            buffer[primeiro_indice_livre] = ' ';
                            buffer[primeiro_indice_livre+1] = ' ';
                            buffer[primeiro_indice_livre+2] = '0';
                            primeiro_indice_livre+=4;
                            //fputs(buffer, saida);
                            num_ua--;
                        }
                        else{
                            linha_comeco++;
                            fputs(buffer, saida);
                            primeiro_indice_livre = 0;
                            offset += 64;
                            fseek(saida, offset, SEEK_SET);
                            if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                                fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                                fclose(saida);
                                exit(1);
                            }
                            fseek(saida, offset, SEEK_SET);
                        }
                        if (num_ua == 0) {
                            fputs(buffer, saida);
                        }
                    }

                    offset_nf = offset; 
                    linha_nf = linha_comeco+3;
                    indice_nf = primeiro_indice_livre; 
                    if (indice_nf >= TAMANHO_LINHA) {
                        linha_nf++;
                        indice_nf = 0; 
                        offset_nf += 64; 
                    }
                    fclose(saida);
                    return 0;
                }
            }
            else{
                tamanho_livre = 0;
            }
            i += 4; 
        }
        
        linha_atual++;
    }

    offset = OFFSET_INICIAL;
    linha_atual = 3;
    i = 0;
    linha_comeco = 0;
    tamanho_livre = 0;
    primeiro_indice_livre = 0;
    fseek(saida, offset, SEEK_SET);
    while (linha_atual <= linha_nf){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        
        
        i = 0; 
        
        while (i < TAMANHO_LINHA) {
            if (buffer[i]=='2'){
                tamanho_livre++;
                if (tamanho_livre == 1) {
                    linha_comeco = linha_atual-3;
                    primeiro_indice_livre = i;
                }
                if (tamanho_livre == num_ua){
                    //printf("Linha %d, primeiro indice livre: %d\n", linha_comeco, primeiro_indice_livre);
                    for (int i = 0; i <linha_comeco; i++) {
                        offset+=64;
                    }

                    fseek(saida, offset, SEEK_SET);
                    if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                        fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                        fclose(saida);
                        exit(1);
                    }
                    fseek(saida, offset, SEEK_SET);
                    while (num_ua>0){
                        if (primeiro_indice_livre < TAMANHO_LINHA){
                            buffer[primeiro_indice_livre] = ' ';
                            buffer[primeiro_indice_livre+1] = ' ';
                            buffer[primeiro_indice_livre+2] = '0';
                            primeiro_indice_livre+=4;
                            //fputs(buffer, saida);
                            num_ua--;
                        }
                        else{
                            linha_comeco++;
                            fputs(buffer, saida);
                            primeiro_indice_livre = 0;
                            offset += 64;
                            fseek(saida, offset, SEEK_SET);
                            if (fgets(buffer, sizeof(buffer), saida) == NULL) {
                                fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
                                fclose(saida);
                                exit(1);
                            }
                            fseek(saida, offset, SEEK_SET);
                        }
                        if (num_ua == 0) {
                            fputs(buffer, saida);
                        }
                    }

                    offset_nf = offset; 
                    linha_nf = linha_comeco+3;
                    indice_nf = primeiro_indice_livre; 
                    if (indice_nf >= TAMANHO_LINHA) {
                        linha_nf++;
                        indice_nf = 0; 
                        offset_nf += 64; 
                    }
                    fclose(saida);
                    return 0;
                }
            }
            else{
                tamanho_livre = 0;
            }
            i += 4; 
        }
        
        linha_atual++;
    }

    //printf("Memória insuficiente para alocar %d unidades de alocação\n", num_ua);
    fclose(saida);
    return 1;
}

void compactar_memoria(){
    saida = fopen(arquivo_saida, "r+"); 
    if (saida == NULL) {
        perror("Erro ao abrir o arquivo de saída");
        exit(1);
    }
    long offset = 0;
    char buffer[128];
    int linha_atual = 0;
    int num_cheios = 0;
    
    while (linha_atual <  3){
        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
  
        offset += strlen(buffer);
        linha_atual++;
    }
    while (linha_atual < MAX_LINHAS){

        if (fgets(buffer, sizeof(buffer), saida) == NULL) {
            fprintf(stderr, "Arquivo tem menos linhas que o esperado\n");
            fclose(saida);
            exit(1);
        }
        for (int i = 0; i < TAMANHO_LINHA; i+=4) {
            if (buffer[i]==' '){
                num_cheios++;
            }
        }
        linha_atual++;
    }

    //printf("Número de linhas cheias: %d\n", num_cheios);

    
    linha_atual = 3;

    while (num_cheios>0){
        if (num_cheios>=16){
            for (int i=0; i<TAMANHO_LINHA; i+=4) {
                buffer[i] = ' ';
                buffer[i+1] = ' ';
                buffer[i+2] = '0';
            }
            fseek(saida, offset, SEEK_SET);
            fputs(buffer, saida);
            offset += 64;
            num_cheios -= 16;
            linha_atual++;
        }
        else{
            for (int i=0; i<TAMANHO_LINHA; i+=4) {
                buffer[i] = '2';
                buffer[i+1] = '5';
                buffer[i+2] = '5';
            }
            for (int i=0; i<num_cheios*4; i+=4) {
                buffer[i] = ' ';
                buffer[i+1] = ' ';
                buffer[i+2] = '0';
            }
            fseek(saida, offset, SEEK_SET);
            fputs(buffer, saida);
            offset += 64;
            num_cheios = 0;
            linha_atual++;
        }
    }

    for (int i=0; i<TAMANHO_LINHA; i+=4) {
        buffer[i] = '2';
        buffer[i+1] = '5';
        buffer[i+2] = '5';
    }
    while (linha_atual < MAX_LINHAS){
        fseek(saida, offset, SEEK_SET);
        fputs(buffer, saida);
        offset += 64;
        linha_atual++;
    }
    fclose(saida);
}

int escolher_estrategia(int estrategia, int num_ua) {
    switch (estrategia) {
        case 1:
            return first_fit(num_ua);
        case 2:
            return next_fit(num_ua);
        case 3:
            return best_fit(num_ua);
        case 4:
            return worst_fit(num_ua);
    }
}
void init(int argc, char *argv[]){
   
    sscanf(argv[1], "%d", &n);
    arquivo_entrada = argv[2];
    arquivo_trace = argv[3];
    arquivo_saida = argv[4];
};


int main(int argc, char *argv[]) {
    init(argc, argv);
    copia_arquivo();
    FILE *trace = fopen(arquivo_trace, "r");
    if (trace == NULL) {
        perror("Erro ao abrir o arquivo trace");
        exit(EXIT_FAILURE);
    }
    char buffer_trace[1024];
    char linha[1000];
    char ua[1000];
    int num_erros = 0;
    

    while (fgets(buffer_trace, sizeof(buffer_trace), trace) != NULL) {
        
        int i = 0;
        int j = 0;
        int compactar = 0;
        int num_ua;


        while (buffer_trace[i] != ' ' && buffer_trace[i] != '\n' && buffer_trace[i] != '\0') {
            linha[j++] = buffer_trace[i++];
        }
        linha[j] = '\0'; 
        int num_linha = atoi(linha); 

        i++; 
        j = 0;

    
        while (buffer_trace[i] != '\n' && buffer_trace[i] != '\0') {
            ua[j++] = buffer_trace[i++];
            if (buffer_trace[i] == 'C') {
                compactar = 1; 
                break;
            }
        }
        if (!compactar){
            ua[j] = '\0'; 
            num_ua = atoi(ua); 
            int erro = escolher_estrategia(n, num_ua);
            if (erro == 1) {
                printf("%s", buffer_trace);
                num_erros++;
            }
          
        }
        else{
            compactar_memoria();
        }

        //printf("Linha lida: %s\n", linha);
        //printf("Unidades de Alocação lidas: %s\n", ua); 
        //printf("Linha: %d, Unidades de Alocação: %d\n", num_linha, num_ua);
    }

    fclose(trace);
    printf("\n");
    printf("%d\n", num_erros);
    return 0;
}