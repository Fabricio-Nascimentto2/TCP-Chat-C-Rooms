//

#include "../include/service.h"

void *server_input_handler(void *arg) {
    char buffer[MAXMSG];
    while (fgets(buffer, MAXMSG, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        if (strlen(buffer) > 0) { // Only log and broadcast if the message is not empty
            log_with_timestamp(buffer);
            broadcast_message(-1, buffer);
        }
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
    send(client_sockfd, welcome_msg, strlen(welcome_msg), MSG_NOSIGNAL);

    char buffer[MAXMSG];
    ssize_t bytes_read;
    char sender_nick[32];
    strncpy(sender_nick, "Anonimo", sizeof(sender_nick) - 1);
    sender_nick[sizeof(sender_nick) - 1] = '\0';

    add_client(client_sockfd);

    while ((bytes_read = recv(client_sockfd, buffer, MAXMSG - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';

        // Comando /nick
        if (strncmp(buffer, "/nick ", 6) == 0) {
            char *new_nick = buffer + 6;
            new_nick[strcspn(new_nick, "\r\n")] = '\0';

            if (set_client_nickname(client_sockfd, new_nick)) {
                strncpy(sender_nick, new_nick, sizeof(sender_nick) - 1);
                sender_nick[sizeof(sender_nick) - 1] = '\0';
            }
            continue;
        }

        // Comando /leave
        if (strncmp(buffer, "/leave", 6) == 0) {
            pthread_mutex_lock(&clients_lock);
            client_node_t *curr = clients;
            while (curr && curr->sockfd != client_sockfd) curr = curr->next;
            if (curr) {
                memset(curr->room, 0, sizeof(curr->room));
                strncpy(curr->room, DEFAULT_ROOM, sizeof(curr->room) - 1);
                curr->room[sizeof(curr->room) - 1] = '\0';
            }
            pthread_mutex_unlock(&clients_lock);

            char msg[] = "[SISTEMA] Você voltou para a sala Geral.\n";
            send(client_sockfd, msg, strlen(msg), MSG_NOSIGNAL);
            continue;
        }

        // Comando /join <sala>
        if (strncmp(buffer, "/join ", 6) == 0) {
            char *new_room = buffer + 6;
            new_room[strcspn(new_room, "\r\n")] = '\0';

            if (strlen(new_room) == 0) {
                char err[] = "[ERRO] Nome da sala inválido.\n";
                send(client_sockfd, err, strlen(err), MSG_NOSIGNAL);
                continue;
            }
            
            pthread_mutex_lock(&clients_lock);
            client_node_t *curr = clients;
            while (curr && curr->sockfd != client_sockfd) curr = curr->next;
            if (curr) {
                memset(curr->room, 0, sizeof(curr->room));
                strncpy(curr->room, new_room, sizeof(curr->room) - 1);
                curr->room[sizeof(curr->room) - 1] = '\0';
            }
            pthread_mutex_unlock(&clients_lock);

            char join_msg[64];
            snprintf(join_msg, sizeof(join_msg), "[SISTEMA] Você entrou na sala: %s\n", new_room);
            send(client_sockfd, join_msg, strlen(join_msg), MSG_NOSIGNAL);
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
                char *msg_ptr = (char *)private_msg;
                msg_ptr[strcspn(msg_ptr, "\r\n")] = '\0';
                
                // Buscar socket do destinatário
                int recipient_sockfd = -1;
                pthread_mutex_lock(&clients_lock);
                client_node_t *curr = clients;
                while (curr) {
                    if (strcasecmp(curr->nickname, recipient_nick) == 0) {
                        recipient_sockfd = curr->sockfd;
                        break;
                    }
                    curr = curr->next;
                }
                pthread_mutex_unlock(&clients_lock);

                if (recipient_sockfd != -1) {
                    char final_msg[MAXMSG + 64];
                    snprintf(final_msg, sizeof(final_msg), "[Privado] @%s: %s\n", sender_nick, msg_ptr);
                    send(recipient_sockfd, final_msg, strlen(final_msg), MSG_NOSIGNAL);

                    // Enviar confirmação para o próprio remetente
                    char confirm_msg[MAXMSG + 64];
                    snprintf(confirm_msg, sizeof(confirm_msg), "[Para @%s]: %s\n", recipient_nick, msg_ptr);
                    send(client_sockfd, confirm_msg, strlen(confirm_msg), MSG_NOSIGNAL);

                    // Logar a mensagem privada
                    save_message_to_log(sender_nick, msg_ptr, "private");
                } else {
                    char error_msg[] = "[ERRO] Usuário não encontrado.\n";
                    send(client_sockfd, error_msg, strlen(error_msg), MSG_NOSIGNAL);
                }
                continue;
            } else {
                char error_msg[] = "[ERRO] Use: /msg <apelido> <mensagem>\n";
                send(client_sockfd, error_msg, strlen(error_msg), MSG_NOSIGNAL);
                continue;
            }
        }

        // Remover quebra de linha da mensagem pública
        buffer[strcspn(buffer, "\r\n")] = '\0';
        if (strlen(buffer) == 0) continue;

        // Envia apenas o buffer. O broadcast_message já cuida da formatação com cores e apelido.
        broadcast_message(client_sockfd, buffer);  
    }

    if (bytes_read == -1) {
        char err_log[128];
        snprintf(err_log, sizeof(err_log), "[ERRO] Socket error com cliente @%s", sender_nick);
        log_with_timestamp(err_log);
    }

    // Notificar outros usuários que o cliente saiu
    char exit_msg[128];
    snprintf(exit_msg, sizeof(exit_msg), CLR_YELLOW "[INFO] @%s saiu do servidor.\n" CLR_RESET, sender_nick);
    log_with_timestamp(exit_msg);
    broadcast_message(-1, exit_msg);

    close(client_sockfd);
    remove_client(client_sockfd);
    pthread_exit(NULL);
}
