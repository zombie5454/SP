#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_ROW 1654321
#define MAX_COL 8340
#define MAX_EPOCH 1000
#define MAX_THREAD 100

int global_row, global_col;
int global_quantity;
pthread_mutex_t mutex;    //mutex for finish
pthread_cond_t cond_main, cond_worker; //cond_main used for wake up thread_main
int finish = 0;             //num of thread who finishs his job
int global_left;         //means 0 ~ global_left-1 need to do (row/quantity+1)個row
                         //global_left ~ row-1 need to do (row/quantity)個row
char **global_graph;  //live = 1, dead = 0, 2表示原本是活的但下次是死的, -1表示原本是死的等等變活的
/*
void Copy_graph(char graph[global_row][global_col], char source[global_row][global_col], 
                int start_row, int end_row, int start_col, int end_col, int type){
    //type == 1, source 賦值給 graph
    //type == 2, source 賦值給 global_graph
    if(type == 1){
    for(int i = start_row; i <= end_row; i++)
        for(int j = start_row; j <= end_col; j++)
            graph[i][j] = source[i][j];
    }

    if(type == 2){
    for(int i = start_row; i <= end_row; i++)
        for(int j = start_row; j <= end_col; j++)
            global_graph[i][j] = source[i][j];
    }

    return;
}
*/
void Print_graph(){

    printf("graph:\n");
    for(int i = 0; i < global_row; i++){
        for(int j = 0; j < global_col; j++)
            printf("%d ", global_graph[i][j]);
        printf("\n");
    }
    
    return;
}
/*
void Copy_graph_2(char** graph, char source[global_row][global_col], 
                int start_row, int end_row, int start_col, int end_col, int type){
    //type == 1, source 賦值給 graph
    //type == 2, source 賦值給 global_graph
    if(type == 1){
    for(int i = start_row; i <= end_row; i++)
        for(int j = start_row; j <= end_col; j++)
            graph[i][j] = source[i][j];
    }

    if(type == 2){
    for(int i = start_row; i <= end_row; i++)
        for(int j = start_row; j <= end_col; j++)
            global_graph[i][j] = source[i][j];
    }

    return;
}

void Print_graph_2(char** graph, int type){
    //type == 1, print graph
    //type == 2, print global_graph
    if(type == 1){
    printf("graph:\n");
    for(int i = 0; i < global_row; i++){
        for(int j = 0; j < global_col; j++)
            printf("%d ", graph[i][j]);
        printf("\n");
    }
    }

    if(type == 2){
    printf("graph:\n");
    for(int i = 0; i < global_row; i++){
        for(int j = 0; j < global_col; j++)
            printf("%d ", global_graph[i][j]);
        printf("\n");
    }
    }

    return;
}
*/

int Count_neighbor(int i, int j){
    int count = 0;
    if(i+1 < global_row && global_graph[i+1][j] >= 1)
        count++;
    if(i-1 >= 0 && global_graph[i-1][j] >= 1)
        count++;
    if(j+1 < global_col && global_graph[i][j+1] >= 1)
        count++;
    if(j-1 >= 0 && global_graph[i][j-1] >= 1)
        count++;
    if(i+1 < global_row && j+1 < global_col && global_graph[i+1][j+1] >= 1)
        count++;
    if(i-1 >= 0 && j-1 >= 0 && global_graph[i-1][j-1] >= 1)
        count++;
    if(i+1 < global_row && j-1 >= 0 && global_graph[i+1][j-1] >= 1)
        count++;
    if(i-1 >= 0 && j+1 < global_col && global_graph[i-1][j+1] >= 1)
        count++;
    return count;
}

void Task_read(int id, char graph[(global_row/global_quantity)+1][global_col]){      //id做 id*(row/quantity) ~ (id+1)*(row/quantity)-1 的row
    //printf("in task_read\n");
    int ii = 0, k, start;
    if(id > global_left-1){        
        k = 0;
        start = global_left*(global_row/global_quantity+1)+
            (id-global_left)*(global_row/global_quantity);
    }    
    else{
        k = 1;
        start = id*(global_row/global_quantity+1);
    }
    for(int i = start; i < start + (global_row/global_quantity + k); i++){
        for(int j = 0; j < global_col; j++){
            //printf("i = %d, ii = %d\n", i, ii);
            //printf("i = %d, j = %d\n", i, j);
            int count = Count_neighbor(i, j);
            if(global_graph[i][j] == 0){       //dead cell
                if(count == 3)
                    graph[ii][j] = 1;        //-1表示原本是死的等等變活的
                else
                    graph[ii][j] = 0;
            }
            else if(global_graph[i][j] == 1){     //live cell 
                if(count != 2 && count != 3)
                    graph[ii][j] = 0;     //2表示原本是活的但下次是死的
                else
                    graph[ii][j] = 1;
            }
        }
        ii++;
    }
    return;
}

void Task_write(int id, char graph[(global_row/global_quantity)+1][global_col]){
    //printf("in task_write, id = %d\n", id);
    int ii = 0, k, start;
    if(id > global_left-1){
        k = 0;
        start = global_left*(global_row/global_quantity+1)+
            (id-global_left)*(global_row/global_quantity);
    }    
    else{
        k = 1;
        start = id*(global_row/global_quantity+1);
    }
    //printf("id %d start = %d\n", id, start);
    for(int i = start; i < start + (global_row/global_quantity + k); i++){
        //printf("i = %d, ii = %d\n", i, ii);
        for(int j = 0; j < global_col; j++){
            global_graph[i][j] = graph[ii][j];
        }
        ii++;
    }
    //printf("id %d in task_write finish\n", id);
    return;
}

void handler(int signo){
    /*if(signo == SIGUSR1)
        printf("receive SIGUSR1\n");
*/    return;
}

void MultiProcess(int quantity, char* in_file, char* out_file){
    //printf("run MultiProcess with %d process for %s to %s\n", quantity, in_file, out_file);
    //read input start
    FILE* fp = fopen(in_file, "r");
    char str1[64], str2[16];
    fgets(str1, 64, fp);
    int j = 0, k = 0;
    int data[3];
    for(int i = 0; i < 3; i++){
        while(str1[j] != ' ' && str1[j] != '\n'){
            str2[k] = str1[j];
            j++;
            k++;
        }
        str2[k] = '\0';
        data[i] = atoi(str2);
        j++;
        k = 0;
    }
    int row = data[0], col = data[1], epoch = data[2];
    global_row = row; 
    global_col = col;
    if(quantity > row)
        quantity = row;
    global_quantity = quantity;
    global_left = row % quantity;
    //printf("row = %d, col = %d, epoch = %d\n", row, col, epoch);

    //deal with shared_memory start
    int fd_flag = shm_open("flag_mem", O_RDWR | O_CREAT, 0777);
    
    if(fd_flag <= -1)
        printf("open shared_mem failed");
    ftruncate(fd_flag, 4);
    int *shared_flag = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd_flag, 0);
    *shared_flag = 0;
    //printf("shared_flag = %d\n", *shared_flag);

    int fd_row[row];
    char fd_name[64];
    char *graph_row[row]; 
    for(int i = 0; i < row; i++){
        strcpy(fd_name, "0");
        sprintf(fd_name, "row%d_num", i);
        fd_row[i] = shm_open(fd_name, O_RDWR | O_CREAT, 0777);
        ftruncate(fd_row[i], col);
        graph_row[i] = mmap(NULL, col, PROT_READ | PROT_WRITE, MAP_SHARED, fd_row[i], 0);
    }
    global_graph = graph_row;
    //deal with shared_memory end

    char str3[col+10];
    for(int i = 0; i < row; i++){
        fgets(str3, col+10, fp);
        for(j = 0; j < col; j++)
            if(str3[j] == '.')
                global_graph[i][j] = 0;
            else if(str3[j] == 'O')
                global_graph[i][j] = 1;
            else{
                printf("input format error\n"); 
                exit(0);
            }
    }
    //Print_graph();
    fclose(fp);
    
    //read input end

    //deal with signal start
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);
    sigset_t block_mask, empty_mask;
    sigemptyset(&block_mask);
    sigemptyset(&empty_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block_mask, NULL);   //一開始SIGUSR1被block;
    //deal with signal end

    struct flock lock, unlock;   //lock for shared_flag
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 4;    
    unlock.l_type = F_UNLCK;
    unlock.l_start = 0;
    unlock.l_whence = SEEK_SET;
    unlock.l_len = 4;   

    pid_t ppid[quantity];
    pid_t pid;
    for(int i = 0; i < quantity; i++){
        pid = fork();
        ppid[i] = pid;
        if(pid == 0)
            break;
    }
    if(pid == 0){               //child do
        sigsuspend(&empty_mask);
        fcntl(fd_flag, F_GETLK, lock);      //child拿id，parent和其他child完全'不'平行進行
        int id = *shared_flag;
        *shared_flag += 1;
        //printf("pid = %d, id = %d ready\n", (int)getpid(), id);
        kill(getppid(), SIGUSR1);
        sigprocmask(SIG_BLOCK, &block_mask, NULL);
        fcntl(fd_flag, F_GETLK, unlock);
                                            //id完成，因此給parent或child lock，並等待parent的signal
        
        char next_graph[(global_row/global_quantity)+1][global_col];
        while(1){
            //printf("suspend before read\n");
            sigsuspend(&empty_mask); 
            //printf("suspend after read\n");
            Task_read(id, next_graph);
            fcntl(fd_flag, F_GETLK, lock);
            *shared_flag += 1;
            kill(getppid(), SIGUSR1);
            sigprocmask(SIG_BLOCK, &block_mask, NULL);
            fcntl(fd_flag, F_GETLK, unlock);
            
            //printf("suspend before write\n");
            sigsuspend(&empty_mask);
            //printf("suspend after write\n");
            Task_write(id, next_graph);
            fcntl(fd_flag, F_GETLK, lock);
            *shared_flag += 1;
            kill(getppid(), SIGUSR1);
            sigprocmask(SIG_BLOCK, &block_mask, NULL);
            fcntl(fd_flag, F_GETLK, unlock);
            kill(getppid(), SIGUSR1);

        }
        //printf("shared_flag = %d\n", *shared_flag);
        //pause();
    }
    else{               //parent do
    
        for(int i = 0; i < quantity; i++){                //for child get id
            fcntl(fd_flag, F_GETLK, lock);
            kill(ppid[i], SIGUSR1);
            sigprocmask(SIG_BLOCK, &block_mask, NULL); 
            fcntl(fd_flag, F_GETLK, unlock);
            sigsuspend(&empty_mask);
        }

        *shared_flag = 0;
        sigprocmask(SIG_BLOCK, &block_mask, NULL);
        for(int i = 0; i < quantity; i++)   //叫醒所有child
            kill(ppid[i], SIGUSR1);

        int round = 0;
        int task = 0;
        while(round < epoch){
            //printf("host before suspend\n");
            sigsuspend(&empty_mask);
            //printf("host after suspend\n");
            fcntl(fd_flag, F_GETLK, lock);
            if(*shared_flag < quantity){
                //printf("shared_flag == %d\n", *shared_flag);
                sigprocmask(SIG_BLOCK, &block_mask, NULL);
                fcntl(fd_flag, F_GETLK, unlock);
                continue;
            }
            else{
                task ++;
                *shared_flag = 0;
                sigprocmask(SIG_BLOCK, &block_mask, NULL);
                if(task == 2){
                    round ++;
                    task = 0;
                    //printf("round = %d\n", round);
                }
                if(round >= epoch)
                    continue;
                for(int i = 0; i < quantity; i++)   //叫醒所有child
                    kill(ppid[i], SIGUSR1);
                fcntl(fd_flag, F_GETLK, unlock);
            }
        }
        
        //printf("shared_flag = %d\n", *shared_flag);
        //Print_graph();
        FILE *fp2 = fopen(out_file, "w");
        if(fp2 == NULL)
            printf("open %s fail\n", out_file);
        for(int i = 0; i < global_row; i++){
            for(int j = 0; j < global_col; j++){
                if(global_graph[i][j] == 0)
                    fputc('.', fp2);
                else if(global_graph[i][j] == 1)
                    fputc('O', fp2);
            
            }
            fputc('\n', fp2);
        }
        fclose(fp2);
        return;
    }

    return;
}

void *Thread_funct(void *arg){
    char graph[(global_row/global_quantity)+1][global_col];
    int id = (int)arg;
    //printf("thread %d start\n", id);
    int ii = 0, k, start;
    if(id > global_left-1){        
        k = 0;
        start = global_left*(global_row/global_quantity+1)+
            (id-global_left)*(global_row/global_quantity);
    }    
    else{
        k = 1;
        start = id*(global_row/global_quantity+1);
    }
    //printf("start = %d, end = %d\n", start, start + (global_row/global_quantity + k));
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    while(1){
        Task_read(id, graph);
        pthread_mutex_lock(&mutex);
        finish++;
        pthread_cond_signal(&cond_main);
        pthread_cond_wait(&cond_worker, &mutex);
        pthread_mutex_unlock(&mutex);
        
        Task_write(id, graph);
        pthread_mutex_lock(&mutex);
        finish++;
        pthread_cond_signal(&cond_main);
        pthread_cond_wait(&cond_worker, &mutex);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}
/*
void Refresh_graph(){
    for(int i = 0; i < global_row; i++){
        for(int j = 0; j < global_col; j++){
            if(global_graph[i][j] == -1)
                global_graph[i][j] = 1;
            else if(global_graph[i][j] == 2)
                global_graph[i][j] = 0;
        }
    }
    return;
}
*/
void MultiThread(int quantity, char* in_file, char* out_file){
    //printf("run MultiThread with %d thread for %s to %s\n", quantity, in_file, out_file);
    //read input start
    FILE* fp = fopen(in_file, "r");
    char str1[64], str2[16];
    fgets(str1, 64, fp);
    int j = 0, k = 0;
    int data[3];
    for(int i = 0; i < 3; i++){
        while(str1[j] != ' ' && str1[j] != '\n'){
            str2[k] = str1[j];
            j++;
            k++;
        }
        str2[k] = '\0';
        data[i] = atoi(str2);
        j++;
        k = 0;
    }
    int row = data[0], col = data[1], epoch = data[2];
    global_row = row; 
    global_col = col;
    if(quantity > row)
        quantity = row;
    global_quantity = quantity;
    global_left = row % quantity;
    //printf("row = %d, col = %d, epoch = %d\n", row, col, epoch);

    char *graph_row[row];        //live = 1, dead = 0
    char graph[row][col];
    for(int i = 0; i < row; i++)
        graph_row[i] = graph[i];
    global_graph = graph_row;
    
    char str3[col+10];
    for(int i = 0; i < row; i++){
        fgets(str3, col+10, fp);
        //printf("%s",str3);
        for(j = 0; j < col; j++){
            if(str3[j] == '.')
                global_graph[i][j] = 0;
            else if(str3[j] == 'O')
                global_graph[i][j] = 1;
            else{
                printf("error input is %d\n", str3[j]);
                printf("input format error\n"); 
                exit(0);
            }
        }
    }
    //Copy_graph(NULL, graph, 0, row-1, 0, col-1, 2);
    //Print_graph();
    fclose(fp);
    //read input end
    pthread_t tid[quantity];
    
    pthread_mutex_lock(&mutex);   //確保main是第一個拿到mutex的
    for(int i = 0; i < quantity; i++){
        pthread_create(&tid[i], NULL, Thread_funct, (void *)i);
    }
    int round = 0;
    int task = 0;
    while(round < epoch){       //有thread完成工作
        if(finish != quantity){           //有worker還沒完成
            pthread_cond_wait(&cond_main, &mutex);
            continue;
        }      
        else if(finish == quantity){        //大家都完成了，叫醒大家準備做下一個round
            pthread_cond_broadcast(&cond_worker);
            //Refresh_graph();
            finish = 0;
            task ++;
            if(task == 1){
                //printf("read done\n");
            }
            else if(task == 2){
                task = 0;
                round ++;
                //printf("write done\n");
                //printf("round = %d\n", round);
                //Print_graph();
            }
        
        }
    }
    //Print_graph();
    FILE *fp2 = fopen(out_file, "w");
    if(fp2 == NULL)
        printf("open %s fail\n", out_file);
    for(int i = 0; i < global_row; i++){
        for(int j = 0; j < global_col; j++){
            if(global_graph[i][j] == 0)
                fputc('.', fp2);
            else if(global_graph[i][j] == 1)
                fputc('O', fp2);
            
        }
        fputc('\n', fp2);
    }
    fclose(fp2);
    return;
}

int main(int argc, char *argv[]){
    if(strcmp(argv[1], "-p") == 0)
        MultiProcess(atoi(argv[2]), argv[3], argv[4]);
    else if(strcmp(argv[1], "-t") == 0)
        MultiThread(atoi(argv[2]), argv[3], argv[4]);
    else
        printf("input format error\n"); 
    
    /*for(int i = 0; i < global_row; i++)
        for(int j = 0; j < global_col; j++){
            if(global_graph[i][j] == 0)
                write(fd, ".", 1);
            else if(global_graph[i][j] == 1)
                write(fd, "O", 1);
            write(fd, "\n", 1);
        }
        */
    return 0; 
}
