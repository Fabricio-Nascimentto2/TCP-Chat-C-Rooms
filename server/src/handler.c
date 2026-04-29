//

#include "../include/service.h"

void *server_input_handler(void *arg) {
    char buffer[MAXMSG];
    while (fgets(buffer, MAXMSG, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        log_with_timestamp(buffer);
        broadcast_message(-1, buffer);
    }
    return NULL;
}

void *client_handler(void *arg) {
    int client_sockfd = *(int *)arg;
    free(arg);

    char welcome_msg[] = "\n\tBem vindo ao servidor de chat!!\n"
                         " Use o comando /nick <seu_nome> para definir seu apelido.\n"
                         " Exemplo: /nick Giovani\n"
                         " Use o comando /join <sala> para entrar em uma sala.\n"
                         " Use o comando /leave para voltar à sala Geral.\n"
                         " Use o comando /msg <apelido> <mensagem> para enviar mensagens privadas.\n\n";
    send(client_sockfd, welcome_msg, strlen(welcome_msg), 0);

    char buffer[MAXMSG];
    ssize_t bytes_read;
    char sender_nick[32] = "Desconhecido";

    add_client(client_sockfd);

    while ((bytes_read = recv(client_sockfd, buffer, MAXMSG - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';

        // Buscar nickname do remetente para garantir que temos o mais atual
        char sender_nick[32] = "Desconhecido";
        pthread_mutex_lock(&clients_lock);
        client_node_t *curr = clients;
        while (curr) {
            if (curr->sockfd == client_sockfd) {
                if (strlen(curr->nickname) > 0) strncpy(sender_nick, curr->nickname, sizeof(sender_nick));
                break;
            }
            curr = curr->next;
        }
        pthread_mutex_unlock(&clients_lock);

        // Comando /nick
        if (strncmp(buffer, "/nick ", 6) == 0) {
            char *new_nick = buffer + 6;
            new_nick[strcspn(new_nick, "\r\n")] = '\0';
            if (strlen(new_nick) > 0) {
                set_client_nickname(client_sockfd, new_nick);
            }
            continue;
        }

        // Comando /leave
        if (strncmp(buffer, "/leave", 6) == 0) {
            pthread_mutex_lock(&clients_lock);
            client_node_t *curr = clients;
            while (curr && curr->sockfd != client_sockfd) curr = curr->next;
            if (curr) strncpy(curr->room, DEFAULT_ROOM, sizeof(curr->room) - 1);
            if (curr) curr->room[sizeof(curr->room) - 1] = '\0';
            pthread_mutex_unlock(&clients_lock);

            send(client_sockfd, "[SISTEMA] Você voltou para a sala Geral.\n", 41, 0);
            continue;
        }

        // Comando /join <sala>
        if (strncmp(buffer, "/join ", 6) == 0) {
            char *new_room = buffer + 6;
            new_room[strcspn(new_room, "\r\n")] = '\0';
            
            pthread_mutex_lock(&clients_lock);
            client_node_t *curr = clients;
            while (curr && curr->sockfd != client_sockfd) curr = curr->next;
            if (curr) strncpy(curr->room, new_room, sizeof(curr->room) - 1);
            if (curr) curr->room[sizeof(curr->room) - 1] = '\0';
            pthread_mutex_unlock(&clients_lock);

            char join_msg[64];
            snprintf(join_msg, sizeof(join_msg), "[SISTEMA] Você entrou na sala: %s\n", new_room);
            send(client_sockfd, join_msg, strlen(join_msg), 0);
            continue;
        }

        // Comando /msg (mensagem privada)
        if (strncmp(buffer, "/msg ", 5) == 0) {
            char *space = strchr(buffer + 5, ' ');
            if (space) {
                *space = '\0';
                const char *recipient_nick = buffer + 5;
                const char *private_msg = space + 1;

                // Remover quebra de linha da mensagem privada
                char clean_msg[MAXMSG];
                strncpy(clean_msg, private_msg, sizeof(clean_msg) - 1);
                clean_msg[sizeof(clean_msg) - 1] = '\0';
                clean_msg[strcspn(clean_msg, "\r\n")] = '\0';

                // Buscar socket do destinatário
                int recipient_sockfd = -1;
                pthread_mutex_lock(&clients_lock);
                client_node_t *curr = clients;
                while (curr) {
                    if (strcmp(curr->nickname, recipient_nick) == 0) {
                        recipient_sockfd = curr->sockfd;
                        break;
                    }
                    curr = curr->next;
                }
                pthread_mutex_unlock(&clients_lock);

                if (recipient_sockfd != -1) {
                    char final_msg[MAXMSG + 64];
                    snprintf(final_msg, sizeof(final_msg), "[Privado] @%s: %s", sender_nick, clean_msg);
                    send(recipient_sockfd, final_msg, strlen(final_msg), 0);

                    // Logar a mensagem privada
                    save_message_to_log(sender_nick, clean_msg, "private");
                } else {
                    char error_msg[] = "[ERRO] Usuário não encontrado.\n";
                    send(client_sockfd, error_msg, strlen(error_msg), 0);
                }
                continue;
            }
        }

        // Remover quebra de linha da mensagem pública
        buffer[strcspn(buffer, "\r\n")] = '\0';

        // Montar mensagem formatada
        char final_msg[MAXMSG + 64];
        snprintf(final_msg, sizeof(final_msg), "@%s: %s", sender_nick, buffer);

        // Enviar mensagem (o broadcast ja faz o log_with_timestamp e save_message_to_log)
        broadcast_message(client_sockfd, buffer);  
    }

    // Notificar outros usuários que o cliente saiu
    char exit_msg[64];
    snprintf(exit_msg, sizeof(exit_msg), CLR_YELLOW "[INFO] @%s saiu do servidor." CLR_RESET, sender_nick);
    log_with_timestamp(exit_msg);
    broadcast_message(-1, exit_msg);

    close(client_sockfd);
    remove_client(client_sockfd);
    pthread_exit(NULL);
}
