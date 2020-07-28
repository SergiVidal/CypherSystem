#ifndef CYPHERSYSTEM_CONSOLE_H
#define CYPHERSYSTEM_CONSOLE_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#include "cli.h"

#define TRNAME      "[TR_NAME]"
#define CONOK       "[CONOK]"
#define CONKO       "[CONKO]"
#define MSG         "[MSG]"
#define MSGOK       "[MSGOK]"
#define EMPTY       "[]"

Trinity trinity;
pthread_mutex_t mtx;

void *CONSOLE_threadNewConnections(void *);

void *CONSOLE_threadServer(void *);

void CONSOLE_exit();

void CONSOLE_download(char *);

void CONSOLE_show_audios(char *);

void CONSOLE_say(char *);

void CONSOLE_connect(char *);

void CONSOLE_show_connections();

int CONSOLE_get_console_option(char *);

void CONSOLE_init_trinity(File);

#endif