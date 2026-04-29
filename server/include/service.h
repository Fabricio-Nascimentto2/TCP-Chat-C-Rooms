// Created by andesson on 07/05/25.

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>

// Cores ANSI para um terminal profissional
#define CLR_RESET  "\x1b[0m"
#define CLR_RED    "\x1b[31m"
#define CLR_GREEN  "\x1b[32m"
#define CLR_YELLOW "\x1b[33m"
#define CLR_BLUE   "\x1b[34m"
#define CLR_CYAN   "\x1b[36m"

#define BACKLOG 10
#define MAXMSG 512
#define DEFAULT_ROOM "Geral"

typedef struct client_node {
    int sockfd;
    char nickname[32];
    char room[32];
    struct client_node *next;
} client_node_t;

extern client_node_t *clients;
extern pthread_mutex_t clients_lock;
extern pthread_mutex_t log_lock;
extern int sockfd;

// client
void add_client(int client_sockfd);
void remove_client(int client_sockfd);
void set_client_nickname(int sockfd, const char *nickname);
int is_nickname_taken(const char *nickname);

// broadcast
void broadcast_message(int sender_sockfd, const char *msg);

// handlers
void *client_handler(void *arg);
void *server_input_handler(void *arg);

// logger
void log_message(const char *msg);
void log_with_timestamp(const char *msg);
void save_message_to_log(const char *sender, const char *msg, const char *chat_type);

// signals
void handle_sigint(int sig);

#endif // SERVER_H
