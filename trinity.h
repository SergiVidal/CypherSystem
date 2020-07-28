#ifndef CYPHERSYSTEM_TRINITY_H
#define CYPHERSYSTEM_TRINITY_H
#include <pthread.h>

typedef struct {
    char *username;
    char *audio_file;
    char *ip_num;
    int ip_port;
    char *direction;
    int *direction_ports;
    int num_direction_ports;
} File;

typedef struct {
    char *user;
    int clientSocketFD;
    pthread_t threadConnection;
    int active;
    int port;
} Connection;

typedef struct {
    char type;
    char *header;
    short length;
    char *data;
} Frame;

typedef struct {
    File file;
    int serverSockfd;
    Connection *serverConnections;
    Connection *clientConnections;
    int sNumConnections;
    int cNumConnections;
} Trinity;

#endif
