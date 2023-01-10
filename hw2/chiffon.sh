#!bin/bash


arg[0]="none"
arg[1]=${1}
arg[2]=${2}
arg[3]=${3}
arg[4]=${4}
arg[5]=${5}
arg[6]=${6}
#echo "0 = ${arg[0]}"
#echo "1 = ${arg[1]}"
#echo "2 = ${arg[2]}"
#echo "3 = ${arg[3]}"
#echo "4 = ${arg[4]}"
#echo "5 = ${arg[5]}"
#echo "6 = ${arg[6]}"

for ((i=1 ; i <= 6 ; i++)); do
#    echo "i = ${i}"
    if [ ${arg[${i}]} == "-m" ]; then
        n_host=${arg[$((${i}+1))]}    
    elif [ ${arg[${i}]} == "-l" ]; then
        lucky_number=${arg[$((${i}+1))]}    
    else
        n_player=${arg[$((${i}+1))]} 
    fi
    i=$((${i}+1))
done

#echo "n_host = ${n_host} n_player = ${n_player} lucky_number = ${lucky_number}"

#make fifo
for (( i=0 ; i <= ${n_host} ; i++ )); do
        filename="fifo_${i}.tmp"
        if [ -p ${filename} ]; then
#                echo "${filename} has already existed"
                apple=1
        else
                mkfifo ${filename}
        fi
        exec {fifos[${i}]}<>${filename}
done

#initial score
for((i=0; i<${n_player}; i++)); do
        score[${i}]=0
done

#generate combinations
used_host_index=0

for ((a=1; a <= ${n_player} - 7; a++)); do
        for ((b=a+1; b <= ${n_player} - 6; b++)); do
                for ((c=b+1; c <= ${n_player} - 5; c++)); do
                        for ((d=c+1; d <= ${n_player} - 4; d++)); do
                                for ((e=d+1; e <= ${n_player} - 3; e++)); do
                                        for ((f=e+1; f <= ${n_player} - 2; f++)); do
                                                for ((g=f+1; g <= ${n_player} - 1; g++)); do
                                                        for ((h=g+1; h <= ${n_player} - 0; h++)); do
                                                                if [ ${used_host_index} -lt ${n_host} ]; then
                                                                        #echo "${a} ${b} ${c} ${d} ${e} ${f} ${g} ${h}"
                                                                        #there are hosts that have never been on
                                                                        used_host_index=$((${used_host_index}+1))
                                                                        ./host -m ${used_host_index} -d 0 -l ${lucky_number} &
                                                                        echo "${a} ${b} ${c} ${d} ${e} ${f} ${g} ${h}">&${fifos[${used_host_index}]}

                                                                else   #when all hosts have been used at least once
                                                                        read key < "fifo_0.tmp" #means host[key] is complete and idle
                                                                        for((i=0; i < 8; i++)); do
                                                                                read id rank < "fifo_0.tmp"
                                                                                score[${id}]=$((${score[${id}]} + ${rank}))
                                                                        done

                                                                        echo "${a} ${b} ${c} ${d} ${e} ${f} ${g} ${h}">&${fifos[${key}]}
                                                                fi
                                                        done
                                                done
                                        done
                                done
                        done
                done
        done
done

#last operation
for ((i=1; i<=${used_host_index}; i++)); do
        read key < "fifo_0.tmp" #means host[key] is complete and idle
                for((j=0; j < 8; j++)); do
                        read id rank < "fifo_0.tmp"
                        score[${id}]=$((${score[${id}]} + ${rank}))
                done
done

#send stop signal
for ((i = 1; i <= ${n_host}; i++)); do
        echo "-1 -1 -1 -1 -1 -1 -1 -1">&${fifos[${i}]}
done

#print score
#for ((i = 1; i <= ${n_player}; i++)); do
#        echo ${i} ${score[${i}]}
#done

#count ranking
max=-1
for ((place = 1; place <= ${n_player}; place++)); do       #find place 
        max=-1
        for ((i = 1; i <= ${n_player}; i++)); do   #find score of place 
#                echo "max = ${max}"
                if [ ${score[${i}]} -gt ${max} ]; then
                        max=${score[${i}]}
                fi
        done
        #echo "placemax = ${max}"
        count=-1
        for ((i = 1; i <= ${n_player}; i++)); do  #find player's score same as score of place
                if [ ${score[${i}]} == ${max} ]; then
#                        echo "place = ${place}"
                        ranking[${i}]=${place}
                        score[${i}]=-1
                        count=$((${count}+1))
                fi
        done
        place=$((${place}+${count}))
done

#print ranking
for ((i=1; i <= 8; i++)); do
        echo "${i} ${ranking[${i}]}"
done

for ((i = 0; i <= ${n_host}; i++)); do
        rm "fifo_${i}.tmp"
done
for ((i = 1; i <= ${n_host}; i++)); do
        wait
done
