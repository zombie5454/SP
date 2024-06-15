#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX_FILENAME 64
#define BUFSIZE 256

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
    FILE *fifo_fp;
    char buf[BUFSIZE];

    // for layer0 to use pipe
    int p0to1_1[2], p0to1_2[2];   
    // for layer1 to use pipe
    int p1to0[2], p1to2_1[2], p1to2_2[2];          
    // for layer2 to use pipe          
    int p2to1[2], p2to3_1[2], p2to3_2[2];           
    // for layer3 to use pipe
    int p3to2[2];                                 

    int layer0 = 0, layer1 = 0, layer2 = 0;     // determine whether layerN forked yet  
    while(1){
        if(depth==0){   
            if(layer0 == 0){                 // layer0 haven't fork child yet
                /************* Read FIFO ************/
                char filename[MAX_FILENAME];
                snprintf(filename, MAX_FILENAME, "fifo_%d.tmp", host_id);
                //printf("%s\n", filename);
                fifo_fp = fopen(filename, "r");
                fgets(buf, BUFSIZE, fifo_fp);
                printf("%s", buf);

                int players[8];
                char *start;
                start = strtok(buf, " ");
                for(int i=0; i<8; i++){
                    players[i] = atoi(start);
                    //printf("%d\n", players[i]);
                    start = strtok(NULL, " ");
                }

                /************* Pipe and Fork ************/
                pipe(p0to1_1);
                if(fork() == 0){
                    p1to0[0] = p0to1_1[0];
                    p1to0[1] = p0to1_1[1];
                    depth++;
                    continue;
                }
                FILE* pipe_w = fdopen(p0to1_1[1], "w");
                
                fprintf(pipe_w, "%d %d %d %d\n", players[0], players[1], players[2], players[3]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[0], players[1], players[2], players[3]);
                //printf("write %d\n", write(p0_1_1[1], buf, sizeof(buf)));

                pipe(p0to1_2);
                if(fork() == 0){
                    p1to0[0] = p0to1_2[0];
                    p1to0[1] = p0to1_2[1];
                    depth++;
                    continue;
                }
                pipe_w = fdopen(p0to1_2[1], "w");
                fprintf(pipe_w, "%d %d %d %d\n", players[4], players[5], players[6], players[7]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[4], players[5], players[6], players[7]);
                //printf("write %d\n", write(p0_1_2[1], buf, sizeof(buf)));

                layer0 = 1;
            }
            else{
                wait(NULL);
                wait(NULL);
                break;
            }
        }
        else if(depth==1){
            if(layer1 == 0){                // layer0 haven't fork child yet
                /************* Read Pipe ************/
                FILE* pipe_r = fdopen(p1to0[0], "r");
                int fd = p1to0[0];
                //memset(buf, 0, sizeof(buf));
                //read(fd, buf, BUFSIZE);
                fgets(buf, BUFSIZE, pipe_r);
                printf("%s", buf);
                int players[4];
                char *start;
                start = strtok(buf, " ");
                for(int i=0; i<4; i++){
                    players[i] = atoi(start);
                    //printf("%d\n", players[i]);
                    start = strtok(NULL, " ");
                }
                /************* Pipe and Fork ************/
                pipe(p1to2_1);
                if(fork() == 0){
                    p2to1[0] = p1to2_1[0];
                    p2to1[1] = p1to2_1[1];
                    depth++;
                    continue;
                }
                FILE* pipe_w = fdopen(p1to2_1[1], "w");
                
                fprintf(pipe_w, "%d %d\n", players[0], players[1]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[0], players[1], players[2], players[3]);
                //printf("write %d\n", write(p0_1_1[1], buf, sizeof(buf)));

                pipe(p1to2_2);
                if(fork() == 0){
                    p2to1[0] = p1to2_2[0];
                    p2to1[1] = p1to2_2[1];
                    depth++;
                    continue;
                }
                pipe_w = fdopen(p1to2_2[1], "w");
                fprintf(pipe_w, "%d %d\n", players[2], players[3]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[4], players[5], players[6], players[7]);
                //printf("write %d\n", write(p0_1_2[1], buf, sizeof(buf)));

                layer1 = 1;
            }
            else{
                wait(NULL);
                wait(NULL);
                break;
            }

        }  
        else if(depth==2){
            if(layer2 == 0){   
                /************* Read Pipe ************/
                FILE* pipe_r = fdopen(p2to1[0], "r");
                int fd = p2to1[0];
                //memset(buf, 0, sizeof(buf));
                //read(fd, buf, BUFSIZE);
                fgets(buf, BUFSIZE, pipe_r);
                printf("%s", buf);
                int players[2];
                char *start;
                start = strtok(buf, " ");
                for(int i=0; i<2; i++){
                    players[i] = atoi(start);
                    //printf("%d\n", players[i]);
                    start = strtok(NULL, " ");
                }
                /************* Pipe and Fork ************/
                pipe(p2to3_1);
                if(fork() == 0){
                    p3to2[0] = p2to3_1[0];
                    p3to2[1] = p2to3_1[1];
                    depth++;
                    continue;
                }
                FILE* pipe_w = fdopen(p2to3_1[1], "w");
                
                fprintf(pipe_w, "%d\n", players[0]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[0], players[1], players[2], players[3]);
                //printf("write %d\n", write(p0_1_1[1], buf, sizeof(buf)));

                pipe(p2to3_2);
                if(fork() == 0){
                    p3to2[0] = p2to3_2[0];
                    p3to2[1] = p2to3_2[1];
                    depth++;
                    continue;
                }
                pipe_w = fdopen(p2to3_2[1], "w");
                fprintf(pipe_w, "%d\n", players[1]);
                fflush(pipe_w);
                //snprintf(buf, BUFSIZE, "%d %d %d %d\n", players[4], players[5], players[6], players[7]);
                //printf("write %d\n", write(p0_1_2[1], buf, sizeof(buf)));

                layer2 = 1;
            }
            else{
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
                wait(NULL);
                wait(NULL);

                /***** Read from player *****/
                //FILE* pipe_r1 = fdopen(p2to3_1[0], "r");
                //FILE* pipe_r2 = fdopen(p2to3_2[0], "r");
                //fgets(buf, BUFSIZE, pipe_r1);
                //printf("%s", buf);
                //fgets(buf, BUFSIZE, pipe_r2);
                //printf("%s", buf);
                memset(buf, 0, sizeof(buf));
                read(p2to3_1[0], buf, BUFSIZE);
                printf("%s", buf);
                memset(buf, 0, sizeof(buf));
                read(p2to3_2[0], buf, BUFSIZE);
                printf("%s", buf);
                
                break;
            }
        }
        else if(depth==3){
            FILE* pipe_r = fdopen(p3to2[0], "r");
            int fd = p3to2[0];
            //memset(buf, 0, sizeof(buf));
            //read(fd, buf, BUFSIZE);
            fgets(buf, BUFSIZE, pipe_r);
            //printf("%s", buf);
            //int player_id = atoi(buf);
            buf[strlen(buf)-1] = '\0';
            printf("player_id: %s\n", buf);
            dup2(p3to2[1], 1);          // redirect player's stdout to write pipe
            execl("player", "foo", "-n", buf, NULL);
        }
    }
    return 0;
}