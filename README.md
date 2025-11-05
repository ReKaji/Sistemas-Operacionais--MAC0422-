# Descrição
Sistemas Operacionais — MAC0422

Este repositório reúne exercícios implementados durante a disciplina de Sistemas Operacionais (MAC0422). Cada subprojeto foca em tópicos centrais de engenharia de software e sistemas: programação em C com chamadas de sistema POSIX, concorrência com threads, simulações de políticas de alocação/escalonamento, scripts de avaliação de desempenho e práticas de build reprodutíveis com `Makefile`.

O objetivo das descrições abaixo é explicar de forma objetiva o que cada exercício implementa, quais tecnologias e conceitos são explorados e como executar as versões de referência. Cada seção usa uma entrada curta com subtópicos para facilitar a leitura.

## Sumário

- `ep1  - Escalonador de processos/` — implementação de um shell e exercícios de controle de processos.
- `ep2- Programação paralela com multithreads/` — exercícios com `pthread`, sincronização e paralelismo.
- `ep3 - Algoritmos para Alocação de Memória/` — simulador com métricas de escalonamento e traces de alocação.
- `ep4- bash script e avaliação de servidores de echo concorrentes/` — scripts de benchmark e avaliação de servidores concorrentes.

---

## `ep1  - Escalonador de processos/`

### O que faz
Implementa um shell simples em C e exercícios complementares ligados ao escalonamento e controle de processos. O trabalho foca em parsing de linha de comando, criação de processos com `fork`/`exec`, uso de `wait`/`waitpid`, tratamento de sinais (por exemplo `SIGCHLD`) e modelos básicos de escalonamento/execução de tarefas.

### Tecnologias e conceitos
- Linguagem: C (compilação com `Makefile`)
- POSIX: `fork`, `exec`, `wait`, sinais e handlers
- Práticas: organização em `.c`/`.h`, entradas de teste em `entrada-esperado.txt` e `entrada-inesperado.txt`

### Por que é relevante
Trabalhos de baixo nível com processos e sinais demonstram domínio de chamadas de sistema, gerenciamento de recursos e robustez — competências úteis para engenharia de sistemas, ferramentas e infraestruturas.

### Como compilar/rodar (exemplo)
Na pasta do exercício:

```bash
cd "ep1  - Escalonador de processos"
make
# executável (nome pode variar: ver Makefile) — exemplo:
./uspsh
```

---

## `ep2- Programação paralela com multithreads/`

### O que faz
Exercícios que exploram programação paralela usando threads POSIX. Tipicamente incluem problemas de sincronização (mutual exclusion, condition variables), divisão de trabalho entre threads e medidas simples de desempenho para observar escalabilidade e contenção.

### Tecnologias e conceitos
- Linguagem: C com `pthread`
- Sincronização: `pthread_mutex_t`, `pthread_cond_t` e barreiras
- Estratégias: partição de trabalho, redução de resultados e mitigação de condições de corrida

### Por que é relevante
Demonstrar controle seguro de concorrência, raciocínio sobre memória compartilhada e otimizações simples de paralelismo é diretamente aplicável ao desenvolvimento de sistemas e serviços de alta performance.

### Como compilar/rodar (exemplo)

```bash
cd "ep2- Programação paralela com multithreads"
make
./ep2    # argumentos dependem do exercício; consulte o README local
```

---

## `ep3 - Algoritmos para Alocação de Memória/`

### O que faz
Simula políticas e ambientes relacionados à gestão de recursos (escalonamento e comportamento de alocação). Inclui arquivos de trace usados como entradas de teste, por exemplo `trace-bestfit`, `trace-firstfit`, `trace-nextfit` e `trace-worstfit`, que permitem avaliar diferentes estratégias e medir métricas como fragmentação, throughput e latência de alocação.

### Tecnologias e conceitos
- Linguagem: C
- Simulação determinística: processamento de traces, cálculos de métricas (tempo de espera, turnaround, fragmentação)
- Políticas: diferentes heurísticas/estratégias de alocação e escalonamento (configuráveis via parâmetros ou arquivos de trace)

### Por que é relevante
Simuladores exigem clareza na modelagem, validação experimental e geração/control de entradas — habilidades valiosas para design e avaliação de algoritmos e sistemas.

### Como compilar/rodar (exemplo)

```bash
cd "ep3 - Algoritmos para Alocação de Memória"
make
# executar simulação passando o trace como entrada
./ep3 trace-bestfit
```

---

## `ep4- bash script e avaliação de servidores de echo concorrentes/`

### O que faz
Conjunto de scripts em Bash para orquestrar testes de servidores de echo concorrentes. O script `ep4.sh` automatiza a geração de carga, coleta de métricas (latência/throughput) e comparação entre modelos concorrentes (por exemplo, processos vs threads vs event-driven), além de registros para análise posterior.

### Tecnologias e conceitos
- Shell scripting: Bash (automação de testes e pipelines simples)
- Ferramentas Unix usuais: `nc`/`telnet`/`ab`/`wrk` ou utilitários equivalentes para gerar carga
- Medição: amostragem de tempo, contagem de requisições, logs estruturados

### Por que é relevante
Automação de benchmarks e scripts de avaliação demonstram disciplina em engenharia (reprodutibilidade, coleta de métricas e comparação experimental), importante para desenvolvimento e validação de serviços de produção.

### Como executar (exemplo)

```bash
cd "ep4- bash script e avaliação de servidores de echo concorrentes"
bash ep4.sh    # revisar o topo do script para variáveis de configuração (porta, número de clientes, etc.)
```
