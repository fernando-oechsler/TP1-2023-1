#!/bin/bash

# Verifique se o número de argumentos é igual a 1
if [ $# -ne 1 ]; then
    echo "Uso: $0 <linhas_por_segundo>"
    exit 1
fi

# Obtenha o número de linhas por segundo a partir do primeiro argumento
linhas_por_segundo=$1

# Calcule o intervalo de tempo entre cada linha
intervalo=$(bc <<< "scale=2; 1 / $linhas_por_segundo")

while true; do
    operation_list=("add" "subtract" "multiply" "divide")
    operation=${operation_list[$((RANDOM % 4))]}
    number1=$((RANDOM % 100))
    number2=$((RANDOM % 100))

    command="./client $operation $number1 $number2"
    echo "Running: $command"
    $command

    # Aguarde o intervalo
    sleep $intervalo
done

