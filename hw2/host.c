#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX_FILENAME 64
#define BUFSIZE 256

void fillArray(int *array, char *buf, int size){
    char *start = strtok(buf, " ");
    for(int i=0; i<size; i++){
        array[i] = atoi(start);
        start = strtok(NULL, " ");
    }
    return;
}

int main(int argc, char **argv){

    /****** argv ********/
    int host_id, depth, lucky_number;
    for(int i=1; i<=6; i++){
        if(strncmp(argv[i], "-m", 2) == 0)
            host_id = atoi(argv[i+1]);
        else if(strncmp(argv[i], "-d", 2) == 0)
            depth = atoi(argv[i+1]);
        else if(strncmp(argv[i], "-l", 2) == 0)
            lucky_number = atoi(argv[i+1]);
        else{
            fputs("command error\n", stderr);
            return -1;
        }
        i++;
    }
    printf("host_id = %d, depth = %d, lucky_number = %d\n", host_id, depth, lucky_number);

    /*************  Variable for all process *************/
    char filename[MAX_FILENAME];
    snprintf(filename, MAX_FILENAME, "fifo_%d.tmp", host_id);
    //printf("%s\n", filename);
    FILE *fifo_fp_r = fopen(filename, "r");
    FILE *fifo_fp_w = fopen("fifo_0.tmp", "w");
    int fifo_fd_r = open(filename, O_RDONLY);
    int fifo_fd_w = open("fifo_0.tmp", O_WRONLY);
    char buf[BUFSIZE];

    // for layer0 to use pipe
    /*int p0to1_1[2], p0to1_2[2];   
    // for layer1 to use pipe
    int p1to0[2], p1to2_1[2], p1to2_2[2], p;          
    // for layer2 to use pipe          
    int p2to1[2], p2to3_1[2], p2to3_2[2];           
    // for layer3 to use pipe
    int p3to2[2];       */

    int p10[2], p21[2], p32[2];     // for child to read/write
    int p01_1[2], p01_2[2], p12_1[2], p12_2[2], p23_1[2], p23_2[2];     // for parent to read/write

    int layer0 = 0, layer1 = 0, layer2 = 0;     // determine whether layerN forked yet  
    while(1){
        if(depth==0){   
            if(layer0 == 0){                 // layer0 haven't fork child yet

                /************* Pipe and Fork ************/
                int childtoP[2];
                int PtoChild[2];
                pipe(PtoChild);      // this pipe is for parent to write, child to read
                pipe(childtoP);      // this pipe is for child to write, parent to read
                if(fork() == 0){
                    p10[0] = PtoChild[0];
                    p10[1] = childtoP[1];
                    depth++;                // let child run depth 1
                    continue;
                }
                p01_1[0] = childtoP[0];
                p01_1[1] = PtoChild[1];

                pipe(PtoChild);      
                pipe(childtoP); 
                if(fork() == 0){
                    p10[0] = PtoChild[0];
                    p10[1] = childtoP[1];
                    depth++;
                    continue;
                }
                p01_2[0] = childtoP[0];
                p01_2[1] = PtoChild[1];

                //snprintf(buf, BUFSIZE, "%d -1 -1\n", host_id);
                //write(fifo_fd_w, buf, 30);   
                //fgets(buf, BUFSIZE, fifo_fp_r);
                fprintf(fifo_fp_w, "%d -1 -1\n", host_id);     // to tell server we're ready.
                // ALWAYS flush when you want other endpoint to read synchronously!!!!!
                fflush(fifo_fp_w);   
                printf("open host %d\n", host_id);
                layer0 = 1;                 // don't fork twice
                
            }
            else{
                /************* Read from FIFO ************/   
                
                memset(buf, 0, BUFSIZE);
                read(fifo_fd_r, buf, BUFSIZE);
                //printf("%s", buf);           
                //fgets(buf, BUFSIZE, fifo_fp_r);
                //printf("%s", buf);

                int players[8];
                fillArray(players, buf, 8);
                
                /******** Write to layer1 child ******/
                FILE* pipe_w = fdopen(p01_1[1], "w");
                fprintf(pipe_w, "%d %d %d %d\n", players[0], players[1], players[2], players[3]);
                fflush(pipe_w);
                pipe_w = fdopen(p01_2[1], "w");
                fprintf(pipe_w, "%d %d %d %d\n", players[4], players[5], players[6], players[7]);
                fflush(pipe_w);

                /******** Handle end message *******/
                if(players[0] == -1){
                    wait(NULL);
                    wait(NULL);
                    printf("close host %d\n", host_id);
                    break;
                }

                /******** Read from layer1 child ******/
                char buf1[BUFSIZE], buf2[BUFSIZE];
                int array1[2], array2[2];
                memset(buf1, 0, sizeof(buf1));
                read(p01_1[0], buf1, BUFSIZE);
                //printf("%s", buf1);
                fillArray(array1, buf1, 2);
                int guess1 = array1[1];

                memset(buf2, 0, sizeof(buf2));
                read(p01_2[0], buf2, BUFSIZE);
                //printf("%s", buf2);
                fillArray(array2, buf2, 2);
                int guess2 = array2[1];
                /******** Write to FIFO ******/
                if(guess2 > guess1){
                    fprintf(fifo_fp_w, "%d %d %d\n", host_id, array2[0], array2[1]);
                    fflush(fifo_fp_w);
                    //printf("final winner: %d: %d\n", array2[0], array2[1]);
                }
                else{
                    fprintf(fifo_fp_w, "%d %d %d\n", host_id, array1[0], array1[1]);
                    fflush(fifo_fp_w);
                    //printf("final winner: %d: %d\n", array1[0], array1[1]);
                }

                //break;
                continue;
            }
        }
        else if(depth==1){
            if(layer1 == 0){                // layer0 haven't fork child yet

                /************* Pipe and Fork ************/
                int childtoP[2];
                int PtoChild[2];
                pipe(PtoChild);      // this pipe is for parent to write, child to read
                pipe(childtoP);      // this pipe is for child to write, parent to read
                if(fork() == 0){
                    p21[0] = PtoChild[0];
                    p21[1] = childtoP[1];
                    depth++;              
                    continue;
                }
                p12_1[0] = childtoP[0];
                p12_1[1] = PtoChild[1];

                pipe(PtoChild);      // this pipe is for parent to write, child to read
                pipe(childtoP);      // this pipe is for child to write, parent to read
                if(fork() == 0){
                    p21[0] = PtoChild[0];
                    p21[1] = childtoP[1];
                    depth++;            
                    continue;
                }
                p12_2[0] = childtoP[0];
                p12_2[1] = PtoChild[1];

                layer1 = 1;
            }
            else{
                /******** Read from layer0 parent ******/
                FILE* pipe_r = fdopen(p10[0], "r");
                fgets(buf, BUFSIZE, pipe_r);
                //printf("%s", buf);
                int players[4];
                fillArray(players, buf, 4);

                /******** Write to layer2 child ******/
                FILE* pipe_w = fdopen(p12_1[1], "w");
                fprintf(pipe_w, "%d %d\n", players[0], players[1]);
                fflush(pipe_w);
                pipe_w = fdopen(p12_2[1], "w");
                fprintf(pipe_w, "%d %d\n", players[2], players[3]);
                fflush(pipe_w);

                /******** Handle end message *******/
                if(players[0] == -1){
                    wait(NULL);
                    wait(NULL);
                    break;
                }
                
                /******** Read from layer2 child ******/
                char buf1[BUFSIZE], buf2[BUFSIZE];
                int array1[2], array2[2];
                memset(buf1, 0, sizeof(buf1));
                read(p12_1[0], buf1, BUFSIZE);
                //printf("%s", buf1);
                fillArray(array1, buf1, 2);
                int guess1 = array1[1];

                memset(buf2, 0, sizeof(buf2));
                read(p12_2[0], buf2, BUFSIZE);
                //printf("%s", buf2);
                fillArray(array2, buf2, 2);
                int guess2 = array2[1];
                
                /******** Write to layer0 parent ******/
                pipe_w = fdopen(p10[1], "w");
                if(guess2 > guess1){
                    fprintf(pipe_w, "%d %d\n", array2[0], array2[1]);
                    fflush(pipe_w);
                    //printf("winner2: %d: %d\n", array2[0], array2[1]);
                }
                else{
                    fprintf(pipe_w, "%d %d\n", array1[0], array1[1]);
                    fflush(pipe_w);
                    //printf("winner2: %d: %d\n", array1[0], array1[1]);
                }

                //break;
                continue;
            }

        }  
        else if(depth==2){
            if(layer2 == 0){   

                /************* Pipe and Fork ************/
                int childtoP[2];
                int PtoChild[2];
                pipe(PtoChild);      // this pipe is for parent to write, child to read
                pipe(childtoP);      // this pipe is for child to write, parent to read
                if(fork() == 0){
                    p32[0] = PtoChild[0];
                    p32[1] = childtoP[1];
                    depth++;              
                    continue;
                }
                p23_1[0] = childtoP[0];
                p23_1[1] = PtoChild[1];

                pipe(PtoChild);      // this pipe is for parent to write, child to read
                pipe(childtoP);      // this pipe is for child to write, parent to read
                if(fork() == 0){
                    p32[0] = PtoChild[0];
                    p32[1] = childtoP[1];
                    depth++;              
                    continue;
                }
                p23_2[0] = childtoP[0];
                p23_2[1] = PtoChild[1];

                layer2 = 1;
            }
            else{
                /******** Read from layer1 parent ******/
                FILE* pipe_r = fdopen(p21[0], "r");
                fgets(buf, BUFSIZE, pipe_r);
                //printf("%s", buf);
                int players[2];
                fillArray(players, buf, 2);

                /******** Write to layer3 child ******/
                FILE* pipe_w = fdopen(p23_1[1], "w");
                fprintf(pipe_w, "%d\n", players[0]);
                fflush(pipe_w);
                pipe_w = fdopen(p23_2[1], "w");
                fprintf(pipe_w, "%d\n", players[1]);
                fflush(pipe_w);

                // DEADLOCK!!!!! //
                // We need to wait before reading from player !!!!
                // The tasks are:
                // 1. layer2 write id to pipe
                // 2. player asking id from pipe
                // 3. player write guess to pipe
                // 4. layer2 asking guess from pipe
                // The flow we expect is: 
                // 1 -> 2 -> 3 -> 4, 1->2 or 2->1 are acceptable, so does 3->4 or 4->3.
                // BUT if we don't wait before, the following flow would cause deadlock:
                // 1 -> 4 -> 2 -> forever block, since 2 would never be completed.
                
                /******** Handle end message *******/
                if(players[0] == -1){
                    wait(NULL);
                    wait(NULL);
                    break;
                }

                /***** Read from layer3 child *****/
                char buf1[BUFSIZE], buf2[BUFSIZE];
                int array1[2], array2[2];
                FILE* pipe_r1 = fdopen(p23_1[0], "r");
                FILE* pipe_r2 = fdopen(p23_2[0], "r");
                fgets(buf1, BUFSIZE, pipe_r1);
                //printf("%s", buf1);
                fgets(buf2, BUFSIZE, pipe_r2);
                //printf("%s", buf2);
    
                /******** Write to layer1 parent ******/
                fillArray(array1, buf1, 2);
                int guess1 = array1[1];
                fillArray(array2, buf2, 2);
                int guess2 = array2[1];
                pipe_w = fdopen(p21[1], "w");
                if(guess2 > guess1){
                    fprintf(pipe_w, "%d %d\n", array2[0], array2[1]);
                    fflush(pipe_w);
                    //printf("winner1: %d: %d\n", array2[0], array2[1]);
                }
                else{
                    fprintf(pipe_w, "%d %d\n", array1[0], array1[1]);
                    fflush(pipe_w);
                    //printf("winner1: %d: %d\n", array1[0], array1[1]);
                }

                //break;
                continue;
            }
        }
        else if(depth==3){
            /******** Read from layer2 parent ******/
            FILE* pipe_r = fdopen(p32[0], "r");
            fgets(buf, BUFSIZE, pipe_r);
            buf[strlen(buf)-1] = '\0';
            //printf("player_id: %s\n", buf);

            /******** Handle end message *******/
            if(strncmp(buf, "-1", 2) == 0){
                break;
            }

            /*********** Fork player **********/
            if(fork() == 0){              // fork child to exec player
                dup2(p32[1], 1);          // redirect player's stdout to write pipe
                execl("player", "foo", "-n", buf, NULL);
            }
            wait(NULL);                   // wait player
            continue;
        }
    }
    return 0;
}