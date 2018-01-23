#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

extern int errno;

#define PORT_SERV 30497
#define BUFF_LEN 256
#define PORT_START 50497

#define QUEUE 20

//#define DEBUG

int execPatient(int id);

int main(int argc, char **argv) {

    pid_t childpid1, childpid2, childpid3, childpid4;
    childpid1 = fork();
    if (0 == childpid1) {
        execPatient(1);
        exit(EXIT_SUCCESS);
    }
    childpid2 = fork();
    if (0 == childpid2) {
        execPatient(2);
        exit(EXIT_SUCCESS);
    }
    childpid3 = fork();
    if (0 == childpid3) {
        execPatient(3);
        exit(EXIT_SUCCESS);
    }
    childpid4 = fork();
    if (0 == childpid4) {
        execPatient(4);
        exit(EXIT_SUCCESS);
    }

    waitpid(childpid1, NULL, 0);
    waitpid(childpid2, NULL, 0);
    waitpid(childpid3, NULL, 0);
    waitpid(childpid4, NULL, 0);
    exit(EXIT_SUCCESS);

//    execPatient(1);

#ifdef DEBUG
    printf("Patient exit!\n");
#endif

    return 0;
}

int execPatient(int id) {
#ifdef DEBUG
    printf("execPatient,id=%d\n",id);
#endif
    char hname[128];
    gethostname(hname, sizeof(hname));
    struct hostent *hent = gethostbyname(hname);

    struct sockaddr_in addr_serv;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr = *(struct in_addr *) (*(hent->h_addr_list));
    addr_serv.sin_port = htons(PORT_SERV);

    char buff[BUFF_LEN];
    int len = sizeof(addr_serv);
//    while (fgets(buff, BUFF_LEN, stdin) != NULL) {
//        sendto(s, buff, BUFF_LEN, 0, (struct sockaddr *) &addr_serv, len);
//    }

//    srand(time(NULL));
//    int doctor_id = rand() % 2 + 1;
    int doctor_id = 1;
    if (id % 2 == 0)
        doctor_id = 2;

    sprintf(buff, "patient%d %s %d doctor%d", id, inet_ntoa(*(in_addr *) hent->h_addr_list[0]), PORT_START + id * 1000,
            doctor_id);
    sendto(s, buff, strlen(buff), 0, (struct sockaddr *) &addr_serv, len);
    close(s);

    int ss = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT_START + id * 1000);
    server_sockaddr.sin_addr = *(struct in_addr *) (*(hent->h_addr_list));
#ifdef DEBUG
    printf("bind port=%d\n",PORT_START + id * 1000);
#endif

    if(bind(ss, (struct sockaddr* ) &server_sockaddr, sizeof(server_sockaddr))==-1) {
#ifdef DEBUG
        printf("bind error=%d",errno);
#endif
        exit(1);
    }
    if(listen(ss, QUEUE) == -1) {
#ifdef DEBUG
        printf("listen error=%d",errno);
#endif
        exit(1);
    }
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    int conn = accept(ss, (struct sockaddr*)&client_addr, &length);
    if( conn < 0 ) {
        printf("accept error=%d",errno);
        exit(1);
    }
    char buffer[1024];
    while(1) {
        memset(buffer, 0 ,sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer), 0);
        if(strcmp(buffer, "exit\n") == 0) break;
        printf("%s", buffer);
        send(conn, buffer, len , 0);
    }
    close(conn);
    close(ss);
}