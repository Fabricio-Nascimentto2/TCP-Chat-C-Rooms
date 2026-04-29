//
// Created by andesson on 17/05/25.
//

#include "../include/service.h"

void broadcast_message(int sender_sockfd, const char *msg) {
    pthread_mutex_lock(&clients_lock);

    client_node_t *sender = NULL;
    if (sender_sockfd != -1) {
        client_node_t *curr = clients;
        while (curr) {
            if (curr->sockfd == sender_sockfd) {
                sender = curr;
                break;
            }
            curr = curr->next;
        }
    }

    char formatted_msg[MAXMSG + 64];
    if (sender_sockfd == -1) {
        snprintf(formatted_msg, sizeof(formatted_msg), "%s\n", msg);
    } else {
        const char *sender_name = (sender && strlen(sender->nickname) > 0) ? sender->nickname : "Desconhecido";
        snprintf(formatted_msg, sizeof(formatted_msg), CLR_GREEN "@%s" CLR_RESET ": %s\n", sender_name, msg);
    }

    // Salva no log de mensagens públicas
    save_message_to_log(sender_sockfd == -1 ? "SISTEMA" : (sender ? sender->nickname : "Desconhecido"), msg, "public");

    const char *target_room = sender ? sender->room : "";

    client_node_t *curr = clients;
    while (curr) {
        // Envia apenas para quem está na mesma sala (exceto se for mensagem de sistema -1)
        if (curr->sockfd != sender_sockfd && (sender_sockfd == -1 || strcmp(curr->room, target_room) == 0)) {
            send(curr->sockfd, formatted_msg, strlen(formatted_msg), 0);
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&clients_lock);
}