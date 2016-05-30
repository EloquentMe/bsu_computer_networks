#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <libgen.h>

typedef struct sockaddr_in sockaddr;
const int port = 25600;

int main(int argc, char** argv)
{
    int n;
    int m;
    if (argc >= 3)
    {
        n = atoi(argv[1]);
        n = (n > 0) ? n : 1;
        m = atoi(argv[2]);
        m = (n > 0) ? m : 1;
    }
    else
    {
        n = m = 1;
    }
    int sock;
    int val = 1;
    sockaddr client_addr; //optional?
    memset(&client_addr, 0, sizeof(client_addr));
    sockaddr addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0)
    {
        perror("Failed to set socket option");
        exit(EXIT_FAILURE);
    }
    char* msg = calloc(m + 1, sizeof(char));

    if (strcmp(basename(argv[0]), "client") == 0)
    {
        printf("Running as client.\n");
        int i;
        memset(msg, 'a', m);
        msg[m] = 0;
        for(i = 0; i < n; i++) {
            int ret = sendto(sock, msg, sizeof(char) * m, 0,
                    (const struct sockaddr*) &addr, sizeof(addr));
            if (ret < 0)
            {
                printf("Failed to write datagram to server");
                exit(EXIT_FAILURE);
            }
        }
        sendto(sock, 0, 0, 0, (struct sockaddr *) &addr, sizeof(addr));
    }
    else
    {
        if (bind(sock, (const struct sockaddr*) &addr, sizeof(addr)) < 0)
        {
            perror("Failed to bind socket");
            exit(EXIT_FAILURE);
        }
        printf("Running as server.\n");
        int k = 0;
        while (1)
        {
            int ret = recvfrom(sock, msg, sizeof(char) * m, 0, NULL, NULL);
            if (ret < 0)
            {
                perror("Failed to receive datagram");
                exit(EXIT_FAILURE);
            }
            else if (ret == 0)
                break;
            ++k;
            printf("%s\n", msg);
        }
    }
    close(sock);
}
