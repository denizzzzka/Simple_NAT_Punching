#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define PORT_NUM 8888
#define IP_HELP_SERVER "169.3.3.3"
#define HELP_SERVER_MES_LENGTH 21
#define IP_LENGTH 16

void client_UDP();
void server_UDP();
struct sockaddr_in connect_help_server(int);
void do_NAT_PUNCHING_UDP_client(int, int, struct sockaddr_in);
void do_NAT_PUNCHING_UDP_server(int, int, struct sockaddr_in);
int start_game_server();
void start_game_client();
void server_UDP_help();
void do_help(int);

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Sendto(int fd, const void *buf, size_t n, int flags, __CONST_SOCKADDR_ARG addr, socklen_t addr_len) {
    int res = sendto(fd, buf, n, flags, addr, addr_len);
    if (res == -1) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
    if (res < n) {
        printf("Sendto failed\nSent less data, than in buf\n");
        exit(EXIT_FAILURE);
    }
}

void Recvfrom(int fd, void *buf, size_t n, int flags, struct sockaddr *addr, socklen_t *addr_len) {
    int res = recvfrom(fd, buf, n, flags, addr, addr_len);
    if (res == -1) {
        perror("recvfrom failed");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        printf("inet_pton failed: src does not contain a character"
            " string representing a valid network address in the specified"
            " address family\n");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
}

void Inet_ntop(int af, const void *src, char *dst, socklen_t length) {
    const char *res = inet_ntop(AF_INET, src, dst, HELP_SERVER_MES_LENGTH);;
    if (res == NULL) {
        perror("inet_ntop failed");
        exit(EXIT_FAILURE);
    }
}

void Close(int sockfd) {
    int res = close(sockfd);
    if (res == -1) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

// actual program

void client_UDP() {
    start_game_client();
    printf("I am waiting for your num...\n");
    int num;
    scanf("%d", &num);

    int client = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in global_server = connect_help_server(client);

    do_NAT_PUNCHING_UDP_client(client, num, global_server);
    Close(client);
}

void server_UDP() {
    int num = start_game_server();
    int server = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in global_client = connect_help_server(server);

    do_NAT_PUNCHING_UDP_server(server, num, global_client);
    Close(server);
}

struct sockaddr_in connect_help_server(int client) {
    printf("sending info to help server\n");

    struct sockaddr_in adr = {0}; //help server's adr  struct
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM); //server's port
    Inet_pton(AF_INET, IP_HELP_SERVER, &adr.sin_addr); //help server's ip

    char buf[HELP_SERVER_MES_LENGTH] = {0};
    int tmp;
    int adr_len = sizeof(adr);

    Sendto(client, &tmp, sizeof(tmp), 0, (struct sockaddr *) &adr, adr_len);
    printf("Waiting...\n");
    Recvfrom(client, buf, HELP_SERVER_MES_LENGTH, 0, (struct sockaddr *) &adr, &adr_len);
    printf("All ok\n\n");
    
    unsigned short global_port;
    char global_ip[IP_LENGTH] = {0};
    sscanf(buf, "%hu %s", &global_port, global_ip);
    printf("received : %d  |  %s\n", global_port, global_ip);

    struct sockaddr_in adr_global = {0}; //global server's adr  struct
    adr_global.sin_family = AF_INET;
    adr_global.sin_port = htons(global_port); //global server's port
    Inet_pton(AF_INET, global_ip, &adr_global.sin_addr); //global server's ip

    return adr_global;
}

void do_NAT_PUNCHING_UDP_client(int client, int num, struct sockaddr_in global_server) {
    printf("Starting NAT PUNCHING...\n");

    int buf = -1;
    while (1) {
        int adr_len = sizeof(global_server);

        Sendto(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_server, adr_len);
        Recvfrom(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_server, &adr_len);
        if (buf == -1) {
            printf("You have connection\n");
            buf = num;
        } else if (buf == 1) {
            printf("You guessed!\n");
            return;
        } else if (buf == 0){
            printf("You missed\n");
            return;
        }
    }
}

void do_NAT_PUNCHING_UDP_server(int client, int num, struct sockaddr_in global_client) {
    printf("Starting NAT PUNCHING...\n");

    int buf = -1;
    while (1) {
        int adr_len = sizeof(global_client);

        Sendto(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_client, adr_len);
        Recvfrom(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_client, &adr_len);
        if (buf == -1) {
            printf("You have connection\n");
        } else if (buf == num) {
            printf("Client guessed!\n");
            buf = 1;
            Sendto(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_client, adr_len);
            return;
        } else {
            printf("Client missed\n");
            buf = 0;
            Sendto(client, &buf, sizeof(buf), 0, (struct sockaddr *) &global_client, adr_len);
            return;
        }
    }
}

void start_game_client() {
    printf("Hi! you are client\nTry to guess server number from 1 to 3\n"
            "You fave only one try!!!\n\n");

    return;
}

int start_game_server() {
    printf("Hi! You are server\nTry to think of number from 1 to 3\n"
            "Client will try to guess it\nYou also can use rand"
            "\n1 - for rand\n2 - to input your own num\n\n");

    int buf;
    scanf("%d", &buf);
    if (buf == 1) {
        srand(time(NULL));
        return rand() % 3 + 1;
    } else if (buf == 2) {
        printf("Write your number\n");
        scanf("%d", &buf);
        printf("\nI have your number\n\n");
        return buf;
    }
    return 0;
}

void server_UDP_help() {
    int server_help = Socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in adr = {0};
    adr.sin_family = AF_INET;
    adr.sin_port = htons(PORT_NUM);
    Bind(server_help, (struct sockaddr *) &adr, sizeof(adr));

    do_help(server_help);
    Close(server_help);
}

void do_help(int client) {
    int tmp;
    printf("Wait for connections...\n");

    struct sockaddr_in adr1 = {0};
    int adr_len1 = sizeof(adr1);

    char buf1[HELP_SERVER_MES_LENGTH] = {0};
    char buf2[HELP_SERVER_MES_LENGTH] = {0};
    char buf_ip1[IP_LENGTH] = {0};
    char buf_ip2[IP_LENGTH] = {0};

    Recvfrom(client, &tmp, sizeof(tmp), 0, (struct sockaddr *) &adr1, &adr_len1);
    unsigned short port1 = ntohs(adr1.sin_port);
    Inet_ntop(AF_INET, &adr1.sin_addr, buf_ip1, HELP_SERVER_MES_LENGTH);
    snprintf(buf1, HELP_SERVER_MES_LENGTH - 1, "%d %s", port1, buf_ip1);
    printf("received : %s\n", buf1);

    struct sockaddr_in adr2 = {0};
    int adr_len2 = sizeof(adr2);
    printf("Wait for connections...\n");

    Recvfrom(client, &tmp, sizeof(tmp), 0, (struct sockaddr *) &adr2, &adr_len2);
    unsigned short port2 = ntohs(adr2.sin_port);
    Inet_ntop(AF_INET, &adr2.sin_addr, buf_ip2, HELP_SERVER_MES_LENGTH);
    sprintf(buf2, "%d %s", port2, buf_ip2);
    printf("received : %s\n", buf2);

    printf("swapping and sending...\n");

    Sendto(client, buf1, strlen(buf1) + 1, 0, (struct sockaddr *) &adr2, adr_len2);
    Sendto(client, buf2, strlen(buf2) + 1, 0, (struct sockaddr *) &adr1, adr_len1);

    printf("All done!!!");
    return;
}

int main() {
    printf("Who are you?\n1 - client\n2 - server\n3 - help server\n");
    int buf;
    scanf("%d", &buf);
    if (buf == 1) {
        client_UDP();
    } else if (buf == 2) {
        server_UDP();
    } else if (buf == 3){
        server_UDP_help();
    } else {
        printf("Write correct num!!!\n");
    }
    
    return 0;
}