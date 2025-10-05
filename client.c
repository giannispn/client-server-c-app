#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_NUMS 100
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_un server;
    char buf[BUF_SIZE];

    if (argc < 2) {
        printf("usage: %s <socket_path>\n", argv[0]);
        exit(1);
    }

    while (1) {
        int n;
        printf("Enter number of integers (0 to exit): ");
        scanf("%d", &n);
        if (n <= 0 || n > MAX_NUMS)
            break;

        int numbers[MAX_NUMS];
        printf("Enter %d integers:\n", n);
        for (int i = 0; i < n; i++)
            scanf("%d", &numbers[i]);

        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("opening stream socket");
            exit(1);
        }

        server.sun_family = AF_UNIX;
        strcpy(server.sun_path, argv[1]);

        if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
            perror("connecting stream socket");
            close(sock);
            exit(1);
        }

        write(sock, &n, sizeof(int));
        write(sock, numbers, sizeof(int) * n);

        int bytes = read(sock, buf, BUF_SIZE - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            printf("Server response: %s\n", buf);
        }

        close(sock);
    }

    return 0;
}
