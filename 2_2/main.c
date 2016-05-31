#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <libgen.h>
#include <netdb.h>
#include <pthread.h>


typedef struct sockaddr_in sockaddr;
const int client_port = 12800;
const int server_port = 25600;
const int buf_size = 1024;

void* ListenBroadcastProc(void * arg)
{
	int sock;
	char msg[buf_size];
	int val = 1; 
	struct sockaddr_in other_addr, me_addr;
	memset(&other_addr, 0, sizeof(other_addr));
	struct hostent *host;

	host = gethostbyname("localhost");

	me_addr.sin_family = AF_INET;
	me_addr.sin_port   = htons(client_port);
	me_addr.sin_addr.s_addr = INADDR_ANY;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
    {
        perror("Failed to set socket option SO_REUSEADDR");
        exit(EXIT_FAILURE);
	}

	if (bind(sock, (const struct sockaddr*) &me_addr, sizeof(me_addr)) < 0)
	{
		perror("Failed to bind socket");
		exit(EXIT_FAILURE);
	}
	while (1) {
		int ret = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *) &other_addr, &val);
		if (ret < 0)
		{
			perror( "Failed to read datagram from server" );
			exit(EXIT_FAILURE);
		}
		strtok(msg, "\n");
		printf( "%s\n", msg);  /* output client's data buffer  */
		printf("> ");
	}
	return 0;
}

int main(int argc, char** argv)
{
    sockaddr client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    sockaddr server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    sockaddr broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(client_port);
    broadcast_addr.sin_addr.s_addr = htonl(0x7fffffff);

    int sock;
    int val = 1;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    char* msg = calloc(buf_size, sizeof(char));

    if (strcmp(basename(argv[0]), "client") == 0)
    {
        printf("Running as client.\n");
        if ((server_addr.sin_addr.s_addr = inet_aton(argv[1])) == INADDR_NONE)
        {
            perror("Invalid server address");
            exit(EXIT_FAILURE);

        }

        pthread_create(NULL, NULL, &ListenBroadcastProc, NULL);

        while (1)
        {
            fgets(msg, sizeof(msg), stdin);
            int ret = sendto(sock, msg, sizeof(msg), 0,
                    (const struct sockaddr*) &server_addr, sizeof(server_addr));
            if (ret < 0)
            {
                printf("Failed to write datagram to server");
                exit(EXIT_FAILURE);
            }
        }
        sendto(sock, 0, 0, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    }
    else
    {
        server_addr.sin_addr.s_addr = INADDR_ANY;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) < 0)
        {
            perror("Failed to set socket option SO_BROADCAST");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
        {
            perror("Failed to set socket option SO_REUSEADDR");
            exit(EXIT_FAILURE);
        }
        if (bind(sock, (const struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
        {
            perror("Failed to bind socket");
            exit(EXIT_FAILURE);
        }
        printf("Running as server.\n");
        while (1)
        {
            int ret = recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr*) &client_addr, &val);
            if (ret < 0)
            {
                perror("Failed to receive datagram");
                exit(EXIT_FAILURE);
            }
            else if (ret == 0)
                break;
            printf("%s\n", msg);
            
            char broadcast_msg[buf_size];
            sprintf(msg, "%s:%d > %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), msg);
            ret = sendto(sock, broadcast_msg, strlen(broadcast_msg), 0,\
                        (struct sockaddr *) &broadcast_addr, sizeof(server_addr));
            if (ret < 0)
            {
                printf("Failed to broadcast");
                exit(EXIT_FAILURE);
            }
        }
    }
    close(sock);
}
