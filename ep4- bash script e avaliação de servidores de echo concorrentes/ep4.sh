#!/bin/bash

echo "Compilando ep4-servidor-inet_processos"
gcc -o /tmp/servidor-inet_processos ep4-clientes+servidores/ep4-servidor-inet_processos.c

echo "Compilando ep4-servidor-inet_threads"
gcc -o /tmp/servidor-inet_threads ep4-clientes+servidores/ep4-servidor-inet_threads.c -lpthread

echo "Compilando ep4-servidor-inet_muxes"
gcc -o /tmp/servidor-inet_muxes ep4-clientes+servidores/ep4-servidor-inet_muxes.c

echo "Compilando ep4-servidor-unix_threads"
gcc -o /tmp/servidor-unix_threads ep4-clientes+servidores/ep4-servidor-unix_threads.c -lpthread

echo "Compilando ep4-cliente-inet"
gcc -o /tmp/cliente-inet ep4-clientes+servidores/ep4-cliente-inet.c

echo "Compilando ep4-cliente-unix"
gcc -o /tmp/cliente-unix ep4-clientes+servidores/ep4-cliente-unix.c

num_clientes="$1"

n=$#
for ((i=1; i<n; i++)); do
    j=$((i+1))
    tamanho_arq="${!j}"

    nome_arquivo="/tmp/arquivo_de_${tamanho_arq}MB.txt"
    tamanho_bytes=$((tamanho_arq * 1024 * 1024))
    echo ">>>>>>> Gerando um arquivo texto de: ${tamanho_arq}MB..."
    base64 /dev/urandom | head -c "$tamanho_bytes" > "$nome_arquivo"
    echo >> "$nome_arquivo"

    servidores=("servidor-inet_processos" "servidor-inet_threads" "servidor-inet_muxes" "servidor-unix_threads")
    clientes=("cliente-inet" "cliente-inet" "cliente-inet" "cliente-unix")

    linha_resultado="$tamanho_arq"

    for ((s=0; s<4; s++)); do
        
        echo "Subindo o servidor ep4-${servidores[$s]}"
        /tmp/${servidores[$s]} > /dev/null 2>&1 &

        inicio_servidor=$(/bin/date "+%Y-%m-%d %H:%M:%S")
        
        sleep 1
        server_pid=$(ps -eo pid,cmd | grep "[./]${servidores[$s]}\\b" | awk '{print $1}' | head -n 1)
        
        echo ">>>>>>> Fazendo ${num_clientes} clientes ecoarem um arquivo de: ${tamanho_arq}MB..."
    
        for ((c=1; c<=num_clientes; c++)); do
            if [[ "${clientes[$s]}" == "cliente-inet" ]]; then
                cat "$nome_arquivo" | /tmp/cliente-inet 127.0.0.1 > /dev/null 2>&1 &
            else
                cat "$nome_arquivo" | /tmp/cliente-unix > /dev/null 2>&1 &
            fi
        done

        echo "Esperando os clientes terminarem..."
        cliente_proc="${clientes[$s]}"
        while pgrep -x "$cliente_proc" > /dev/null; do
            sleep 1
        done
        
        echo "Verificando os instantes de tempo no journald..."
       
        logs=$(journalctl -q --since "$inicio_servidor" | grep "${servidores[$s]}")
     
        log_accept_line=$(echo "$logs" | grep 'Passou pelo accept' | head -1)
     
        log_last_line=$(echo "$logs" | tail -1)

        
        log_accept_time_raw=$(echo "$log_accept_line" | awk '{print $1, $2, $3}')
        log_last_time_raw=$(echo "$log_last_line" | awk '{print $1, $2, $3}')

        log_accept_time=$(date -d "$log_accept_time_raw" "+%Y-%m-%d %H:%M:%S" 2>/dev/null)
        log_last_time=$(date -d "$log_last_time_raw" "+%Y-%m-%d %H:%M:%S" 2>/dev/null)


        tempo_total=$(dateutils.ddiff "${log_accept_time}" "${log_last_time}" -f "%0M:%0S")

        echo ">>>>>>> ${num_clientes} clientes encerraram a conexão"
        echo ">>>>>>> Tempo para servir os ${num_clientes} clientes com o ep4-${servidores[$s]}: $tempo_total"

        echo "Enviando um sinal 15 para o servidor ep4-${servidores[$s]}..."
        
        kill -15 $server_pid

        linha_resultado="$linha_resultado $tempo_total"
            
    done
    echo "$linha_resultado" >> /tmp/ep4-resultados-100.data
done

cat <<EOF > /tmp/ep4-resultados-100.gpi
set ydata time
set timefmt "%M:%S"
set format y "%M:%S"
set xlabel 'Dados transferidos por cliente (MB)'
set ylabel 'Tempo para atender ${num_clientes} clientes concorrentes'
set term pdfcairo
set output "ep4-resultados-${num_clientes}.pdf"
set grid
set key top left
plot "/tmp/ep4-resultados-100.data" using 1:4 with linespoints title "Sockets da Internet: Mux de E/S",\
     "/tmp/ep4-resultados-100.data" using 1:3 with linespoints title "Sockets da Internet: Threads",\
     "/tmp/ep4-resultados-100.data" using 1:2 with linespoints title "Sockets da Internet: Processos",\
     "/tmp/ep4-resultados-100.data" using 1:5 with linespoints title "Sockets Unix: Threads"
EOF

lista_arq=("${@:2}")
lista_arq_mb=""
for t in "${lista_arq[@]}"; do
    lista_arq_mb+="${t}MB "
done

echo ">>>>>>> Gerando o gráfico de ${num_clientes} clientes com arquivos de: ${lista_arq_mb}"

gnuplot /tmp/ep4-resultados-100.gpi


rm -f /tmp/servidor-inet_processos /tmp/servidor-inet_threads /tmp/servidor-inet_muxes /tmp/servidor-unix_threads \
      /tmp/cliente-inet /tmp/cliente-unix /tmp/arquivo_de_*MB.txt \
      /tmp/ep4-resultados-100.data /tmp/ep4-resultados-100.gpi

exit 0