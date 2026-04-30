//
// Created by andesson on 09/05/25.
//

#include "../include/service.h"

client_node_t *clients = NULL;
pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

int sockfd;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <IP> <PORTA>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN); // Garante que o servidor não feche se um cliente cair

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(ip)
    };

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro no bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) < 0) {
        perror("Erro no listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char msg[100];
    snprintf(msg, sizeof(msg), "Servidor escutando em %s:%d", ip, port);
    log_with_timestamp(msg);

    pthread_t input_thread;
    pthread_create(&input_thread, NULL, server_input_handler, NULL);
    pthread_detach(input_thread);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int *client_sockfd = malloc(sizeof(int));
        if (!client_sockfd) continue;

        *client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        if (*client_sockfd < 0) {
            perror("Erro no accept");
            free(client_sockfd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_sockfd);
        pthread_detach(tid);
    }

    close(sockfd);
    return 0;
}
