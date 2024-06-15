#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define GUESSROUND 1

int main(int argc, char *argv[]){
    if(strncmp(argv[1], "-n", 2) != 0){
        fputs("command error\n", stderr);
        return -1;
    }
    int player_id = atoi(argv[2]);
    int guess;
    int guess_round = GUESSROUND;
/* initialize random seed: */
    for(int i = 1; i <= guess_round; i++){
/* generate guess between 1 and 1000: */
        srand ((player_id + i) * 323);
        char message[20];
        guess = rand() % 1001;
        sprintf(message, "%d %d\n", player_id, guess);
        printf("%s", message);
        fflush(stdout);
    }
    return 0;
}