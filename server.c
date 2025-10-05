#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKNAME "socket"
#define MAX_NUMS 100
#define BACKLOG 5
#define MAX_THREADS 50

pthread_t thread[MAX_THREADS];
int thread_count = 0;

void *connection_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    int n;
    if (read(client_sock, &n, sizeof(int)) <= 0 || n <= 0 || n > MAX_NUMS) {
        write(client_sock, "Invalid input\n", 15);
        close(client_sock);
        return NULL;
    }

    int numbers[MAX_NUMS];
    if (read(client_sock, numbers, sizeof(int) * n) <= 0) {
        write(client_sock, "Read error\n", 12);
        close(client_sock);
        return NULL;
    }

    int sum = 0;
    for (int i = 0; i < n; i++) sum += numbers[i];
    float avg = (float)sum / n;

    char response[256];
    if (avg > 20.0)
        snprintf(response, sizeof(response), "Average: %.2f\nSequence OK\n", avg);
    else
        snprintf(response, sizeof(response), "Check Failed\n");

    write(client_sock, response, strlen(response));
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, *client_sock_ptr;
    struct sockaddr_un server_addr;

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKNAME);
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKNAME);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for a connection on %s...\n", SOCKNAME);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }

        if (thread_count >= MAX_THREADS) {
            printf("Max threads reached. Rejecting connection.\n");
            close(client_sock);
            continue;
        }

        client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;

        if (pthread_create(&thread[thread_count], NULL, connection_handler, client_sock_ptr) != 0) {
            perror("pthread_create");
            close(client_sock);
            free(client_sock_ptr);
        } else {
            pthread_detach(thread[thread_count]);
            thread_count++;
        }
    }

    close(server_sock);
    unlink(SOCKNAME);
    return 0;
}