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

#include <errno.h>

extern int errno;

//#define DEBUG

#define HEALTHCENTER_PORT 30497
#define BUFF_LEN 1024

#define CONTENT_PROVIDER_PORT1  41497
#define CONTENT_PROVIDER_PORT2  42497

#define QUEUE 20

int execDoctor(int id);

int main(int argc, char **argv) {

    pid_t childpid1, childpid2;
    childpid1 = fork();
    if (0 == childpid1) {
        execDoctor(1);
        exit(EXIT_SUCCESS);
    }
    childpid2 = fork();
    if (0 == childpid2) {
        execDoctor(2);
        exit(EXIT_SUCCESS);
    }

    waitpid(childpid1, NULL, 0);
    waitpid(childpid2, NULL, 0);
    exit(EXIT_SUCCESS);

//    execDoctor(1);
#ifdef DEBUG
    printf("Doctor exit!\n");
#endif

    return 0;
}

int count = 0;

int execDoctor(int id) {
    char hname[128];
    gethostname(hname, sizeof(hname));
    struct hostent *hent = gethostbyname(hname);

    struct sockaddr_in addr_serv_healthcenter;
    int s_healthcentor = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr_serv_healthcenter, 0, sizeof(addr_serv_healthcenter));
    addr_serv_healthcenter.sin_family = AF_INET;
    addr_serv_healthcenter.sin_addr = *(struct in_addr *) (*(hent->h_addr_list));
    addr_serv_healthcenter.sin_port = htons(HEALTHCENTER_PORT);

//    char buff[BUFF_LEN];
//    while (fgets(buff, BUFF_LEN, stdin) != NULL) {
//        sendto(s_healthcentor, buff, BUFF_LEN, 0, (struct sockaddr *) &addr_serv_healthcenter, len);
//    }

    if (id == 1) {
        sendto(s_healthcentor, "doctor1 aaaaa", BUFF_LEN, 0, (struct sockaddr *) &addr_serv_healthcenter,
               sizeof(addr_serv_healthcenter));
    } else {
        sendto(s_healthcentor, "doctor2 bbbbb", BUFF_LEN, 0, (struct sockaddr *) &addr_serv_healthcenter,
               sizeof(addr_serv_healthcenter));
    }

    char buffer[BUFF_LEN];
    socklen_t length = sizeof(addr_serv_healthcenter);

    int patient_count = 0;
    char first_ip[16] = "";
    in_port_t first_port = 0;

    while (1) {
        int n = recvfrom(s_healthcentor, buffer, BUFF_LEN, 0, (struct sockaddr *) &addr_serv_healthcenter, &length);
#ifdef DEBUG
        printf("len=%d,%s\n", n, buffer);
#endif
        if (n < 6)
            break;

        if (n > 0) {
            int in = 0;
            char *p[4];
            char *buf = buffer;
            while ((p[in] = strtok(buf, " ")) != NULL) {
#ifdef DEBUG
//                printf("p[%d]=%s\t", in, p[in]);
#endif
                in++;
                buf = NULL;
            }

            int sock_cli = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in servaddr;
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(atoi(p[2]));
            servaddr.sin_addr.s_addr = inet_addr(p[1]);

            int ret = connect(sock_cli, (struct sockaddr *) &servaddr, sizeof(servaddr));
            if (ret < 0) {
#ifdef DEBUG
                printf("connect patients fail,errno=%d,port=%d", errno, atoi(p[2]));
#endif
                continue;
            }

            char txtBuff[BUFF_LEN] = "";
            sprintf(txtBuff, "welcome to doctor%d group\n", id);
            send(sock_cli, txtBuff, strlen(txtBuff), 0);

            char file_name[16] = "";
            sprintf(file_name, "doctor%d.txt", id);
            FILE *fp = fopen(file_name, "r");
            if (NULL == fp) {
#ifdef DEBUG
                printf("failed to open %s\n", file_name);
#endif
                continue;
            }
            while (!feof(fp)) {
                memset(txtBuff, 0, sizeof(txtBuff));
                fgets(txtBuff, sizeof(txtBuff) - 1, fp);
                send(sock_cli, txtBuff, strlen(txtBuff), 0);
            }
            fclose(fp);
            patient_count++;
            printf("%s joined doctor%d\n",p[0],id);
        } else if (n == 0) {
            break;
        }
    }

    if (patient_count == 0) {
        printf("Doctor%d has no peer subscribers!", id);
        return 0;
    }
    if (patient_count == 1) {
        printf("Doctor%d has only one patient subscriber.", id);

    } else if (patient_count > 1) {
        printf("Doctor%d has <%d> patients!", id, patient_count);
    }

    return 0;
}