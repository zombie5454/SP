#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]){
    int player_id = atoi(argv[2]);
    int guess;
/* initialize random seed: */
    for(int i = 1; i <= 10; i++){
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