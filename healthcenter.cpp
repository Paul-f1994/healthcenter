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
#include <libgen.h>

#define DEBUG

#define PORT_SERV 30497
#define BUFF_LEN 1024


bool checkDoctorLogin(char *name, char *pwd);

int main(int argc, char **argv) {
printf("11");

    char hname[128];
    gethostname(hname, sizeof(hname));
    struct hostent *hent = gethostbyname(hname);

    struct sockaddr_in addr_serv, addr_clie;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr_serv, 0, sizeof(addr_serv));
    addr_serv.sin_family = AF_INET;
    addr_serv.sin_addr = *(struct in_addr *) (*(hent->h_addr_list));
    addr_serv.sin_port = htons(PORT_SERV);
    bind(s, (struct sockaddr *) &addr_serv, sizeof(addr_serv));

    char patients_info[BUFF_LEN] = "";
    int n;
    char buffer[BUFF_LEN] = "";
    int patient_count = 0;
    FILE *wp = fopen("./directory.txt", "wt");
    while (1) {
        socklen_t length = sizeof(addr_clie);
        memset(buffer, BUFF_LEN, 0);
        n = recvfrom(s, buffer, BUFF_LEN, 0, (struct sockaddr *) &addr_clie, &length);
        if (n <= 0)
            continue;
#ifdef DEBUG
        printf("%s\n", buffer);
#endif
        //        fwrite(buffer, 1, n, wp);
//        fwrite("\r\n", 1, 2, wp);
        fputs(buffer, wp);
        fputs("\r\n", wp);
        int in = 0;
        char *p[4];
        char *buf = buffer;
        while ((p[in] = strtok(buf, " ")) != NULL) {
#ifdef DEBUG
            printf("p[%d]=%s\t", in, p[in]);
#endif
            in++;
            buf = NULL;
        }
        printf("%s registration is done successfully!\n", p[0]);

        patient_count++;
        if (patient_count == 4)
            break;
        //sendto(s, buff, n, 0, (struct sockaddr *) &addr_clie, len);
    }
    fclose(wp);
    printf("Registration of peers completed! Run the doctors!\n");


    int doctor_count = 0;
    while (1) {
        socklen_t length = sizeof(addr_clie);
        n = recvfrom(s, buffer, BUFF_LEN, 0, (struct sockaddr *) &addr_clie, &length);
        if (n <= 0)
            continue;

#ifdef DEBUG
        printf("%s\n", buffer);
#endif
        int in = 0;
        char *p[2];
        char *buf = buffer;
        while ((p[in] = strtok(buf, " ")) != NULL) {
#ifdef DEBUG
            printf("p[%d]=%s\n", in, p[in]);
#endif
            in++;
            buf = NULL;
        }
        if (checkDoctorLogin(p[0], p[1])) {
            printf("incoming message from doctor %s\n", p[0]);
            printf("%s logged in successfully\n", p[0]);

            FILE *fp = fopen("./directory.txt", "r");
            char txtBuff[BUFF_LEN] = "";
            while (!feof(fp)) {
                memset(txtBuff, 0, sizeof(txtBuff));
                fgets(txtBuff, sizeof(txtBuff) - 1, fp);
                if (strstr(txtBuff, p[0])) {
                    sendto(s, txtBuff, strlen(txtBuff), 0, (struct sockaddr *) &addr_clie, sizeof(addr_clie));
                }
            }
            fclose(fp);
            doctor_count++;
            if (doctor_count == 2)
                break;
        }
    }
    sendto(s, "exit", strlen("exit"), 0, (struct sockaddr *) &addr_clie, sizeof(addr_clie));
    sendto(s, "exit", strlen("exit"), 0, (struct sockaddr *) &addr_clie, sizeof(addr_clie));
    close(s);
#ifdef DEBUG
    printf("Healthcenter exit!\n");
#endif
    return 0;
}

bool checkDoctorLogin(char *name, char *pwd) {

    bool succ = false;
    char txtBuff[BUFF_LEN] = "";

    FILE *fp = fopen("./healthcenter.txt", "r");
    if (NULL == fp) {
        printf("failed to open healthcenter.txt\"\n");
        return succ;
    }
    while (!feof(fp)) {
        memset(txtBuff, 0, sizeof(txtBuff));
        fgets(txtBuff, sizeof(txtBuff) - 1, fp);
        char *name1 = strtok(txtBuff, " ");
        char *pwd1 = strtok(NULL, " ");
        succ = strncmp(name1, name, strlen(name)) == 0 && strncmp(pwd1, pwd, strlen(pwd)) == 0;
        if (succ)
            break;
    }
    fclose(fp);
    return succ;
}
