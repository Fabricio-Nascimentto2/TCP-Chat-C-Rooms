//
// Created by andesson on 07/05/25.
//

#include "../../include/service.h"

void save_message_to_log(const char *sender, const char *msg, const char *chat_type) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    char log_path[256];
    snprintf(log_path, sizeof(log_path), "server/logs/%s.log", chat_type); // public.log ou private.log

    pthread_mutex_lock(&log_lock);
    FILE *fp = fopen(log_path, "a");
    if (fp) {
        fprintf(fp, "[%s] %s: %s\n", timestamp, sender, msg);
        fclose(fp);
    }
    pthread_mutex_unlock(&log_lock);
}

void log_with_timestamp(const char *msg) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", t);
    printf(CLR_BLUE "%s" CLR_RESET " %s\n", timestamp, msg);
    fflush(stdout);
}

void handle_sigint(int sig) {
    log_with_timestamp("Servidor encerrando...");
    close(sockfd);
    exit(0);
}
