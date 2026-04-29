//
// Created by andesson on 18/05/25.
//

#include "../include/service.h"

int is_nickname_taken(const char *nickname) {
    // Nota: Esta função agora assume que quem a chama já detém o clients_lock
    client_node_t *curr = clients;
    while (curr) {
        if (strlen(curr->nickname) > 0 && strcasecmp(curr->nickname, nickname) == 0) {
            return 1;
        }
        curr = curr->next;
    }
    return 0;
}

void set_client_nickname(int sockfd, const char *nickname_raw) {
    char nickname[32];
    strncpy(nickname, nickname_raw, sizeof(nickname) - 1);
    nickname[sizeof(nickname) - 1] = '\0';
    nickname[strcspn(nickname, "\r\n")] = '\0';

    if (strlen(nickname) < 3) {
        char error_msg[] = "[SISTEMA] O apelido deve ter pelo menos 3 caracteres.\n";
        send(sockfd, error_msg, strlen(error_msg), 0);
        return;
    }

    // Sanitização: Impede espaços ou caracteres especiais
    for (int i = 0; i < strlen(nickname); i++) {
        if (nickname[i] <= 32 || nickname[i] == '@') {
            char error_msg[] = CLR_RED "[ERRO] Nickname contém caracteres inválidos.\n" CLR_RESET;
            send(sockfd, error_msg, strlen(error_msg), 0);
            return;
        }
    }

    pthread_mutex_lock(&clients_lock);

    if (is_nickname_taken(nickname)) {
        pthread_mutex_unlock(&clients_lock);
        char error_msg[] = CLR_YELLOW "[SISTEMA] Este apelido já está em uso.\n" CLR_RESET;
        send(sockfd, error_msg, strlen(error_msg), 0);
        return;
    }

    client_node_t *curr = clients;
    while (curr) {
        if (curr->sockfd == sockfd) {
            snprintf(curr->nickname, sizeof(curr->nickname), "%s", nickname);
            break;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&clients_lock);

    char sys_msg[128];
    snprintf(sys_msg, sizeof(sys_msg), CLR_CYAN "[INFO] @%s entrou no chat" CLR_RESET, nickname);
    log_with_timestamp(sys_msg);
    broadcast_message(-1, sys_msg);
}
