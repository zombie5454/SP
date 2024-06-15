#!bin/bash

####### argv #########

arg[1]=${1}
arg[2]=${2}
arg[3]=${3}
arg[4]=${4}
arg[5]=${5}
arg[6]=${6}

n_hosts=''
lucky_number=''
n_players=''

for (( i=1 ; i <= 6 ; i++ )); do
    if [ ${arg[$i]} == "-m" ]; then
        n_hosts=${arg[$(( $i+1 ))]}    
    elif [ ${arg[$i]} == "-l" ]; then
        lucky_number=${arg[$(( $i+1 ))]}    
    elif [ ${arg[$i]} == "-n" ]; then
        n_players=${arg[$(( $i+1 ))]}
    else 
        echo "command error"
        exit 1
    fi
    i=$(( $i+1 ))    
done    

echo "n_hosts = ${n_hosts} lucky_number = ${lucky_number} n_players = ${n_players}"

######## FIFO ##########

for (( i=0 ; i <= ${n_hosts} ; i++ )); do
    filename="fifo_${i}.tmp"
    if [ ! -p ${filename} ]; then
        mkfifo ${filename}
    fi
    #exec {fifos[${i}]}<>${filename}
done

####### Generate Combinations #########

players[8]=''
#combination_count=0

function find_next_player() {

    local player_index=$1
    local start=$2
    #echo "player_index = ${player_index}, start = ${start}"
    if [ $player_index -gt 8 ]; then
        echo "new combination"
        #echo "players: ${players[1]} ${players[2]} ${players[3]} ${players[4]} ${players[5]} ${players[6]} ${players[7]} ${players[8]}"
        echo "${players[*]}\n"
        echo "${players[*]}\n" > "fifo_3.tmp" 
        #combination_count=$(( $combination_count+1))
        return 
    fi

    #echo "max = $(( $n_players-8+$player_index))"
    local i
    for (( i=$start ; i <= $(( $n_players-8+$player_index )) ; i++ )); do
        players[$player_index]=$i
        find_next_player $(( $player_index+1 )) $(( $i+1 ))
    done
}

find_next_player 1 1
#echo "combination_count = ${combination_count}"




