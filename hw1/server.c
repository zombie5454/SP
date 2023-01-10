#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;

int handle_read(request* reqP) {
    int r;
    char buf[512];

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    printf("in function handle read\n");
    printf("%c%c%c\n", buf[0], buf[1], buf[2]);
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int main(int argc, char** argv) {
    printf("pid = %d\n", (int)(getpid()));
    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;

    int wrlock_in_process[20];
    //int rdlock_in_process[20];
    for(int i = 0; i < 20; i++){
        wrlock_in_process[i] = 0;
    //    rdlock_in_process[i] = 0;
    }


    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);
    fcntl(svr.listen_fd, F_SETFL, O_NONBLOCK);
    fd_set original_set, working_set;

    FD_ZERO(&original_set);
    FD_SET(svr.listen_fd, &original_set);
    struct flock lock;

    while (1) {
        // TODO: Add IO multiplexing
    //    printf("maxfd = %d\n", maxfd);
        // Check new connection

        memcpy(&working_set, &original_set, sizeof(original_set));
        if(select(maxfd, &working_set, NULL, NULL, NULL) <= 0)
            continue;

        if(FD_ISSET(svr.listen_fd, &working_set)){
        printf("deal with connect\n");
        clilen = sizeof(cliaddr);
        conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
        if (conn_fd < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;  // try again
            if (errno == ENFILE) {
                (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                continue;
            }
            ERR_EXIT("accept");
        }
        requestP[conn_fd].conn_fd = conn_fd;
        strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
        fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
        //我寫的
        write(requestP[conn_fd].conn_fd, "Please enter your id (to check your preference order):\n", strlen("Please enter your id (to check your preference order):\n"));
        FD_SET(conn_fd, &original_set);
        printf("fd = %d\n", conn_fd);
        //
        continue;
        }

        conn_fd = -1;
        for(int i = 3; i < maxfd; i++) {
            if(i != svr.listen_fd && FD_ISSET(i, &working_set)) {
                conn_fd = i;
                break;
            } 
        }
        if(conn_fd == -1)
            continue;

        printf("deal with request\n");
        printf("fd = %d\n", conn_fd);
        int ret = handle_read(&requestP[conn_fd]); // parse data from client to requestP[conn_fd].buf
        fprintf(stderr, "ret = %d\n", ret);
	    if (ret < 0) {
            fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
            continue;
        }
        
        
    // TODO: handle requests from clients
#ifdef READ_SERVER    

        
        requestP[conn_fd].id = atoi(requestP[conn_fd].buf);
        if(902001 > requestP[conn_fd].id || 902020 < requestP[conn_fd].id){
            printf("[Error] Operation failed. Please try again.\n");
            write(conn_fd, "[Error] Operation failed. Please try again.\n", sizeof("[Error] Operation failed. Please try again.\n"));
            FD_CLR(conn_fd, &original_set);
            close(requestP[conn_fd].conn_fd);
            free_request(&requestP[conn_fd]);
            continue;
        }

        lock.l_type = F_RDLCK;
        lock.l_start = sizeof(registerRecord) * (requestP[conn_fd].id - 902001);
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(registerRecord);
        file_fd = open("registerRecord", O_RDONLY);
        if(fcntl(file_fd, F_SETLK, &lock) != -1 && wrlock_in_process[requestP[conn_fd].id - 902001] == 0){
        
        
        registerRecord current_record;
        lseek(file_fd, sizeof(registerRecord)*(requestP[conn_fd].id-902001), SEEK_SET);
        read(file_fd, &current_record, sizeof(registerRecord));
        lock.l_type = F_UNLCK;
        lock.l_start = sizeof(registerRecord) * (requestP[conn_fd].id - 902001);
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(registerRecord);
        fcntl(file_fd, F_SETLK, &lock);
        printf("current id = %d, requestP id = %d\n", current_record.id, requestP[conn_fd].id);
        char string[4][100];
        strcpy(string[current_record.AZ], "AZ");
        strcpy(string[current_record.BNT], "BNT");
        strcpy(string[current_record.Moderna], "Moderna");
        sprintf(string[0], "Your preference order is %s > %s > %s.\n", string[1], string[2], string[3]);
        write(conn_fd, string[0], sizeof(char)*strlen(string[0]));
        close(file_fd);
        }
        else{
            close(file_fd);
            printf("Locked.\n");
            write(conn_fd, "Locked.\n", sizeof("Locked.\n"));
        }

/*        fprintf(stderr, "%s", requestP[conn_fd].buf);
        sprintf(buf,"%s : %s",accept_read_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
*/
#elif defined WRITE_SERVER
        if(requestP[conn_fd].buf[1] != ' '){
        
        requestP[conn_fd].id = atoi(requestP[conn_fd].buf);
        if(902001 > requestP[conn_fd].id || 902020 < requestP[conn_fd].id){
            printf("[Error] Operation failed. Please try again.\n");
            write(conn_fd, "[Error] Operation failed. Please try again.\n", sizeof("[Error] Operation failed. Please try again.\n"));
            FD_CLR(conn_fd, &original_set);
            close(requestP[conn_fd].conn_fd);
            free_request(&requestP[conn_fd]);
            continue;
        }
        lock.l_type = F_WRLCK;
        lock.l_start = sizeof(registerRecord) * (requestP[conn_fd].id - 902001);
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(registerRecord);
        lock.l_pid = getpid();
        file_fd = open("registerRecord", O_RDWR);
        if(fcntl(file_fd, F_SETLK, &lock) != -1 && wrlock_in_process[requestP[conn_fd].id - 902001] == 0){
        wrlock_in_process[requestP[conn_fd].id - 902001] = 1;
        registerRecord current_record;
        lseek(file_fd, sizeof(registerRecord)*(requestP[conn_fd].id-902001), SEEK_SET);
        read(file_fd, &current_record, sizeof(registerRecord));

        printf("current id = %d, requestP id = %d\n", current_record.id, requestP[conn_fd].id);
        char string[4][100];
        strcpy(string[current_record.AZ], "AZ");
        strcpy(string[current_record.BNT], "BNT");
        strcpy(string[current_record.Moderna], "Moderna");
        sprintf(string[0], "Your preference order is %s > %s > %s.\n", string[1], string[2], string[3]);
        write(conn_fd, string[0], sizeof(char)*strlen(string[0]));
        write(conn_fd, "Please input your preference order respectively(AZ,BNT,Moderna):\n", sizeof("Please input your preference order respectively(AZ,BNT,Moderna):\n"));
        requestP[conn_fd].buf[1] = ' ';
        continue;
        }
        
        else{
            close(file_fd);
            printf("Locked.\n");
            write(conn_fd, "Locked.\n", sizeof("Locked.\n"));
        }


        }

        else{
        
        printf("process 2 in write\n");
        file_fd = open("registerRecord", O_RDWR);

        registerRecord current_record;
        lseek(file_fd, sizeof(registerRecord)*(requestP[conn_fd].id-902001), SEEK_SET);
        read(file_fd, &current_record, sizeof(registerRecord));
        printf("current id = %d, requestP id = %d\n", current_record.id, requestP[conn_fd].id);
        char string[4][100];
        strcpy(buf, requestP[conn_fd].buf);
        printf("%s\n", buf);
        int priority[3];
        char numstr[10];
        numstr[0] = buf[0];
        numstr[1] = '\0';
        priority[0] = atoi(numstr);
        numstr[0] = buf[2];
        numstr[1] = '\0';
        priority[1] = atoi(numstr);
        numstr[0] = buf[4];
        numstr[1] = '\0';
        priority[2] = atoi(numstr);
        if(buf[1] != ' ' || buf[3] != ' ' ||
        priority[0] > 3 || priority[1] > 3 || priority[2] > 3 ||
        priority[0] < 1 || priority[1] < 1 || priority[2] < 1 ||
        strlen(buf) > 5
        ){
            printf("[Error] Operation failed. Please try again.\n");
            write(conn_fd, "[Error] Operation failed. Please try again.\n", sizeof("[Error] Operation failed. Please try again.\n"));
            FD_CLR(conn_fd, &original_set);
            close(requestP[conn_fd].conn_fd);
            free_request(&requestP[conn_fd]);
            continue;
        }
        lseek(file_fd, -sizeof(registerRecord), SEEK_CUR);
        current_record.AZ = priority[0];
        current_record.BNT = priority[1];
        current_record.Moderna = priority[2];
        if(write(file_fd, &current_record, sizeof(registerRecord)) == -1)
            printf("write to record fail\n");
        /*else
            printf("write to record success\n");
        printf("%d %d %d\n", priority[0], priority[1], priority[2]);
        
        lseek(file_fd, -sizeof(registerRecord), SEEK_CUR);
        for(int i = 0; i < 20; i++){
            read(file_fd, &current_record, sizeof(registerRecord));
            if(current_record.id == requestP[conn_fd].id){
                break;
            }
        }*/
        strcpy(string[current_record.AZ], "AZ");
        strcpy(string[current_record.BNT], "BNT");
        strcpy(string[current_record.Moderna], "Moderna");
        sprintf(string[0], "Preference order for %d modified successed, new preference order is %s > %s > %s.\n", requestP[conn_fd].id, string[1], string[2], string[3]);
        printf("%s\n", string[0]);
        write(conn_fd, string[0], sizeof(char)*strlen(string[0]));
        lock.l_type = F_UNLCK;
        lock.l_start = sizeof(registerRecord) * (requestP[conn_fd].id - 902001);
        lock.l_whence = SEEK_SET;
        lock.l_len = sizeof(registerRecord);
        lock.l_pid = getpid();
        fcntl(file_fd, F_SETLK, &lock);
        wrlock_in_process[requestP[conn_fd].id - 902001] = 0;
        close(file_fd);


        }
/*        fprintf(stderr, "%s", requestP[conn_fd].buf);
        sprintf(buf,"%s : %s",accept_write_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
*/     
#endif
        FD_CLR(conn_fd, &original_set);
        close(requestP[conn_fd].conn_fd);
        free_request(&requestP[conn_fd]);
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
