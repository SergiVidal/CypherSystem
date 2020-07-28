#include "console.h"

/****** AUXILIAR FUNCTIONS *******/
/** Transforma una cadena a Minusculas **/
void to_lower_case(char *cadena);

/** Funcion encargada de crear y devolver una trama **/
Frame create_frame(char type, char *header, char *data);

/** Funcion encargada de mandar la trama de servidor a cliente **/
void server_send_frame(int fd, char type, char *header, short length, char *data);

/** Funcion encargada de comprobar si un cliente ya se ha conectado al puerto pasado por parametro **/
int port_used(int port);

/** Funcion encargada de obtener el header de la trama recibida **/
char *get_header_frame(int fd);

/****** PUBLIC ******/
/** Funcion Thread referente a la nueva conexión **/
void *CONSOLE_threadNewConnections(void *arg) {
    Frame frame;
    int clientSocketFD = *((int *) arg);
    int quantityBytes;
    int s;

    s = pthread_mutex_lock(&mtx);
    if (s != 0)
        write(STDOUT_FILENO, "Error: cannot mutex lock\n", strlen("Error: cannot mutex lock\n"));

    int pos = trinity.sNumConnections;
    trinity.sNumConnections++;

    s = pthread_mutex_unlock(&mtx);
    if (s != 0)
        write(STDOUT_FILENO, "Error: cannot mutex unlock\n", strlen("Error: cannot mutex unlock\n"));

    trinity.serverConnections[pos].active = 1;

    while (1) {
        /** Recibe la trama del ciente **/
        quantityBytes = read(clientSocketFD, &frame.type, sizeof(char));
        if (frame.type == 0x02 && quantityBytes > 0) {

//            frame.header = malloc(sizeof(char) * strlen(MSG));
//            read(clientSocketFD, frame.header, sizeof(char) * strlen(MSG));
            frame.header = get_header_frame(clientSocketFD);

            read(clientSocketFD, &frame.length, sizeof(short));

            frame.data = malloc(sizeof(char) * (int) frame.length);
            read(clientSocketFD, frame.data, sizeof(char) * frame.length);

            frame.data[frame.length] = '\0';
            /****/

            write(STDOUT_FILENO, "\n[", sizeof(char) * strlen("\n["));
            write(STDOUT_FILENO, trinity.serverConnections[pos].user,
                  sizeof(char) * strlen(trinity.serverConnections[pos].user));
            write(STDOUT_FILENO, "]: ", sizeof(char) * strlen("]: "));
            write(STDOUT_FILENO, frame.data, sizeof(char) * strlen(frame.data));
            write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

            bzero(frame.data, (int) frame.length);

            free(frame.header);
            free(frame.data);

            /** CREA SU TRAMA EN CASO DE OK **/
            frame = create_frame(0x02, MSGOK, "\0");
            /****/

            /** Manda su trama **/
            server_send_frame(clientSocketFD, frame.type, frame.header, frame.length, frame.data);
            /****/

            free(frame.header);
            free(frame.data);

            CLI_show_prompt(trinity.file.username);

        } else if (frame.type == 0x06 && quantityBytes > 0) {
            write(STDOUT_FILENO, "Terminando conexion...\n", sizeof("Terminando conexion...\n"));

            /** Recibe la trama del ciente **/
//            frame.header = malloc(sizeof(char) * strlen("[]"));
//            read(clientSocketFD, frame.header, sizeof(char) * strlen("[]"));
            frame.header = get_header_frame(clientSocketFD);

            read(clientSocketFD, &frame.length, sizeof(short));

            frame.data = malloc(sizeof(char) * (int) frame.length);
            read(clientSocketFD, frame.data, sizeof(char) * frame.length);
            /****/

            frame.data[frame.length] = '\0';

            bzero(frame.data, (int) frame.length);

            free(frame.header);
            free(frame.data);

//            frame.length = 0;
            if ((int) frame.length > 0) {
                /** CREA SU TRAMA EN CASO DE OK **/
                frame = create_frame(0x06, CONOK, "\0");
                /****/

                /** Manda su trama **/
                server_send_frame(clientSocketFD, frame.type, frame.header, frame.length, frame.data);
                /****/
            } else {
                /** CREA SU TRAMA EN CASO DE KO **/
                frame = create_frame(0x06, CONKO, "\0");
                /****/

                /** Manda su trama **/
                server_send_frame(clientSocketFD, frame.type, frame.header, frame.length, frame.data);
                /****/
            }
            free(frame.header);
            free(frame.data);
            /****/
            break;
        }
    }
    write(STDOUT_FILENO, "[Server]: Cerrando socket del cliente: ",
          sizeof(char) * strlen("[Server]: Cerrando socket del cliente: "));
    write(STDOUT_FILENO, trinity.serverConnections[pos].user,
          sizeof(char) * strlen(trinity.serverConnections[pos].user));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

    CLI_show_prompt(trinity.file.username);
    close(trinity.serverConnections[pos].clientSocketFD);
    free(trinity.serverConnections[pos].user);
    trinity.serverConnections[pos].active = 0;
    return (void *) arg;
}

/** Funcion Thread referente a la parte del servidor **/
void *CONSOLE_threadServer(void *arg) {
    int serverPort = *((int *) arg);
    int quantityBytes = 0;
    Frame frame;

    /** Creamos el socket del servidor **/
    trinity.serverSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (trinity.serverSockfd < 0) {
        perror("socket TCP");
        exit(EXIT_FAILURE);
    }

    /** Configura la conexion **/
    struct sockaddr_in s_addr;
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(serverPort);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    /** Linkea la conexion **/
    if (bind(trinity.serverSockfd, (void *) &s_addr, sizeof(s_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /** Permite un maximo de 5 conexiones simultaneas **/
    listen(trinity.serverSockfd, 5);

    trinity.serverConnections = (Connection *) malloc(sizeof(Connection));

    while (1) {
        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof(c_addr);

        /** Se queda a la espera de una nueva conexion **/
        int clientSocketFD = accept(trinity.serverSockfd, (void *) &c_addr, &c_len);
        if (clientSocketFD < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        } else {
            /** Evita el problema de las conexiones del show_connections() **/
            quantityBytes = read(clientSocketFD, &frame.type, sizeof(char));

            if (frame.type == 0x01 && quantityBytes > 0) {
                /** Recibe la trama del ciente **/
//                frame.header = malloc(sizeof(char) * strlen(TRNAME));
//                read(clientSocketFD, frame.header, sizeof(char) * strlen(TRNAME));
                frame.header = get_header_frame(clientSocketFD);

                read(clientSocketFD, &frame.length, sizeof(short));

                frame.data = malloc(sizeof(char) * (int) frame.length);
                read(clientSocketFD, frame.data, sizeof(char) * frame.length);
                frame.data[frame.length] = '\0';
                /****/
//                frame.length = 0;
                if ((int) frame.length > 0) {
                    /** Asigna la conexion al struct de las conexiones del servidor **/
                    trinity.serverConnections = (Connection *) realloc(trinity.serverConnections,
                                                                       sizeof(Connection) *
                                                                       (trinity.sNumConnections + 1));
                    trinity.serverConnections[trinity.sNumConnections].clientSocketFD = clientSocketFD;

                    trinity.serverConnections[trinity.sNumConnections].user = (char *) malloc(
                            sizeof(char) * strlen(frame.data));
                    strcpy(trinity.serverConnections[trinity.sNumConnections].user, frame.data);
                    /****/

                    free(frame.header);
                    free(frame.data);

                    /** CREA SU TRAMA EN CASO DE OK **/
                    frame = create_frame(0x01, CONOK, trinity.file.username);
                    /****/

                    /** Manda su trama **/
                    server_send_frame(clientSocketFD, frame.type, frame.header, frame.length, frame.data);
                    /****/

                    write(STDOUT_FILENO, "\nNew connection from: ", strlen("\nNew connection from: "));
                    write(STDOUT_FILENO, trinity.serverConnections[trinity.sNumConnections].user,
                          strlen(trinity.serverConnections[trinity.sNumConnections].user));
                    write(STDOUT_FILENO, "\n", strlen("\n"));

                    CLI_show_prompt(trinity.file.username);
                    pthread_create(&trinity.serverConnections[trinity.sNumConnections].threadConnection, NULL,
                                   CONSOLE_threadNewConnections,
                                   &trinity.serverConnections[trinity.sNumConnections].clientSocketFD);

                    free(frame.header);
                    free(frame.data);
                } else {
                    free(frame.header);
                    free(frame.data);

                    /** CREA SU TRAMA EN CASO DE KO **/
                    frame = create_frame(0x01, CONKO, "\0");
                    /****/

                    /** Manda su trama **/
                    server_send_frame(clientSocketFD, frame.type, frame.header, frame.length, frame.data);
                    /****/

                    close(clientSocketFD);
                    free(frame.header);
                    free(frame.data);
                }
            }
        }
    }
}

/** Funcion que se utiliza para cerrar el programa **/
void CONSOLE_exit() {
    Frame frame;
    int quantityBytes;
    int s;

    write(STDOUT_FILENO, "Disconnecting Trinity...\n", sizeof(char) * strlen("Disconnecting Trinity...\n"));

    /** Destruimos el mutex **/
    s = pthread_mutex_destroy(&mtx);
    if (s != 0)
        write(STDOUT_FILENO, "Error: cannot destroy mutex\n", strlen("Error: cannot destroy mutex\n"));

    /**Para cada una de las conexiones del servidor, cerramos los fd y liberamos memoria **/
    for (int i = 0; i < trinity.sNumConnections; i++) {
        if (trinity.serverConnections[i].active) {
            free(trinity.serverConnections[i].user);
            close(trinity.serverConnections[i].clientSocketFD);
        }
    }
    for (int i = 0; i < trinity.cNumConnections; i++) {
        if (trinity.clientConnections[i].active) {
            /** Crea la trama **/
            frame = create_frame(0x06, EMPTY, trinity.file.username);
            /****/

            /** Manda la trama **/
            quantityBytes = write(trinity.clientConnections[i].clientSocketFD, &frame.type, sizeof(char));
//            quantityBytes = 0;
            if (quantityBytes > 0) {
                write(trinity.clientConnections[i].clientSocketFD, frame.header, sizeof(char) * strlen(frame.header));
                write(trinity.clientConnections[i].clientSocketFD, &frame.length, sizeof(short));
                write(trinity.clientConnections[i].clientSocketFD, frame.data, sizeof(char) * frame.length);
                /****/

                free(frame.header);
                free(frame.data);

                /** Recibe la trama del servidor **/
                read(trinity.clientConnections[i].clientSocketFD, &frame.type, sizeof(char));

//                frame.header = malloc(sizeof(char) * strlen(CONOK));
//                read(trinity.clientConnections[i].clientSocketFD, frame.header, sizeof(char) * strlen(CONOK));
                frame.header = get_header_frame(trinity.clientConnections[i].clientSocketFD);

                read(trinity.clientConnections[i].clientSocketFD, &frame.length, sizeof(short));

                frame.data = malloc(sizeof(char) * (int) frame.length);
                read(trinity.clientConnections[i].clientSocketFD, frame.data, sizeof(char) * frame.length);

                frame.data[frame.length] = '\0';

//                strcpy(frame.header, "CONKO");
                if (strcmp(frame.header, CONOK) == 0) {
                    write(STDOUT_FILENO, "Se ha realizado la desconexion OK del servidor: ",
                          sizeof(char) * strlen("Se ha realizado la desconexion OK del servidor: "));
                    write(STDOUT_FILENO, trinity.clientConnections[i].user,
                          sizeof(char) * strlen(trinity.clientConnections[i].user));
                    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
                } else {
                    write(STDOUT_FILENO, "Se ha realizado la desconexion KO del servidor: ",
                          sizeof(char) * strlen("Se ha realizado la desconexion KO del servidor: "));
                    write(STDOUT_FILENO, trinity.clientConnections[i].user,
                          sizeof(char) * strlen(trinity.clientConnections[i].user));
                    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
                }
                free(frame.header);
                free(frame.data);
            } else {
                write(STDOUT_FILENO, "Se ha realizado la desconexion KO del servidor: ",
                      sizeof(char) * strlen("Se ha realizado la desconexion KO del servidor: "));
                write(STDOUT_FILENO, trinity.clientConnections[i].user,
                      sizeof(char) * strlen(trinity.clientConnections[i].user));
                write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

                free(frame.header);
                free(frame.data);
            }
            /****/
            free(trinity.clientConnections[i].user);
            close(trinity.clientConnections[i].clientSocketFD);
        }
    }

    free(trinity.file.username);
    free(trinity.file.audio_file);
    free(trinity.file.ip_num);
    free(trinity.file.direction);
    free(trinity.file.direction_ports);

    free(trinity.serverConnections);
    free(trinity.clientConnections);
}

/** Funcion encargada de descargar audios **/
/** No realizada (Fase 3: Opcional) **/
void CONSOLE_download(char *command) {
    write(STDOUT_FILENO, "CONSOLE_download()\n", sizeof(char) * strlen("CONSOLE_download()\n"));
    write(STDOUT_FILENO, "Has introducido el comando: ", sizeof(char) * strlen("Has introducido el comando: "));
    write(STDOUT_FILENO, command, sizeof(char) * strlen(command));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
}

/** Funcion encargada de mostrar audios **/
/** No realizada (Fase 3: Opcional) **/
void CONSOLE_show_audios(char *command) {
    write(STDOUT_FILENO, "CONSOLE_show_audios()\n", sizeof(char) * strlen("CONSOLE_show_audios()\n"));
    write(STDOUT_FILENO, "Has introducido el comando: ", sizeof(char) * strlen("Has introducido el comando: "));
    write(STDOUT_FILENO, command, sizeof(char) * strlen(command));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
}

/** Funcion encargada de mandar mensajes entre el Cliente y el Servidor **/
void CONSOLE_say(char *command) {
    Frame frame;
    char *say, *username, *msj;
    int exist = 0;
    int quantityBytes;

    say = CLI_get_word(command, 0, 2);
    username = CLI_get_word(command, 1, 2);
    msj = CLI_get_word(command, 2, 2);

    if (strcmp(say, "say") == 0 && username != NULL && msj != NULL && strlen(username) != 0 &&
        strlen(msj) != 0 && strlen(msj) <= 180) {
        /** Creamos la trama **/
        frame = create_frame(0x02, MSG, msj);
        /****/

        for (int i = 0; i < trinity.cNumConnections; i++) {
            if (strcmp(trinity.clientConnections[i].user, username) == 0 &&
                trinity.clientConnections[i].active == 1) {
                exist = 1;

                /** Manda la trama al servidor **/
                quantityBytes = write(trinity.clientConnections[i].clientSocketFD, &frame.type, sizeof(char));
//                            quantityBytes = 0;
                if (quantityBytes > 0) {
                    write(trinity.clientConnections[i].clientSocketFD, frame.header,
                          sizeof(char) * strlen(frame.header));
                    write(trinity.clientConnections[i].clientSocketFD, &frame.length, sizeof(short));
                    write(trinity.clientConnections[i].clientSocketFD, frame.data, sizeof(char) * frame.length);
                    /****/

                    free(frame.header);
                    free(frame.data);

                    /** Recibe la trama del servidor **/
                    read(trinity.clientConnections[i].clientSocketFD, &frame.type, sizeof(char));

//                    frame.header = malloc(sizeof(char) * strlen(MSGOK));
//                    read(trinity.clientConnections[i].clientSocketFD, frame.header, sizeof(char) * strlen(MSGOK));
                    frame.header = get_header_frame(trinity.clientConnections[i].clientSocketFD);

                    read(trinity.clientConnections[i].clientSocketFD, &frame.length, sizeof(short));

                    frame.data = malloc(sizeof(char) * (int) frame.length);
                    read(trinity.clientConnections[i].clientSocketFD, frame.data, sizeof(char) * frame.length);
                    /****/

                    frame.data[frame.length] = '\0';

                    free(frame.header);
                    free(frame.data);
                } else {
                    write(STDOUT_FILENO, "Error, se ha perdido la conexion con el servidor!\n",
                          sizeof(char) * strlen("Error, se ha perdido la conexion con el servidor!\n"));
                    free(frame.header);
                    free(frame.data);
                }
                break;
            }
        }
        if (!exist) {
            write(STDOUT_FILENO, "No existe el usuario!\n",
                  sizeof(char) * strlen("No existe el usuario!\n"));
            free(frame.header);
            free(frame.data);
        }
    } else {
        write(STDOUT_FILENO,
              "Error en el comando o el msj supera los 180 chars!\n",
              sizeof(char) * strlen("Error en el comando o el msj supera los 180 chars!\n"));
    }
    free(say);
    free(username);
    free(msj);
}

/** Funcion encargada de realizar una conexión del Cliente al Servidor **/
void CONSOLE_connect(char *command) {
    Frame frame;
    char *conn, *port;
    int quantityBytes, n_port;

    conn = CLI_get_word(command, 0, 1);
    port = CLI_get_word(command, 1, 1);

    n_port = atoi(port);

    if (strcmp(conn, "connect") == 0 && port != NULL && strlen(port) != 0) {
        if (n_port != trinity.file.ip_port) {
            if (!port_used(n_port)) {
                /** Comprueba la IP y la convierte a binario **/
                struct in_addr ip_addr;
                if (inet_aton(trinity.file.ip_num, &ip_addr) == 0) {
                    fprintf(stderr, "inet_aton (%s): %s\n", trinity.file.ip_num, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                /** Creamos el socket **/
                int sockfd;
                sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (sockfd < 0) {
                    perror("socket TCP");
                    exit(EXIT_FAILURE);
                }

                /** Configura la ip y el puerto del servidor **/
                struct sockaddr_in s_addr;
                bzero(&s_addr, sizeof(s_addr));
                s_addr.sin_family = AF_INET;
                s_addr.sin_port = htons(atoi(port));
                s_addr.sin_addr = ip_addr;

                /** Realiza la conexion **/
                if (connect(sockfd, (void *) &s_addr, sizeof(s_addr)) < 0) {
                    write(STDOUT_FILENO, "Error, no se ha podido realizar la conexion!\n",
                          sizeof(char) * strlen("Error, no se ha podido realizar la conexion!\n"));
                } else {

                    write(STDOUT_FILENO, "Connecting to ", sizeof(char) * strlen("Connecting to "));
                    write(STDOUT_FILENO, port, sizeof(char) * strlen(port));
                    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

                    /** Creamos la trama **/
                    frame = create_frame(0x01, TRNAME, trinity.file.username);
                    /****/

                    /** Manda la trama al servidor **/
                    quantityBytes = write(sockfd, &frame.type, sizeof(char));
//                    quantityBytes = 0;
                    if (quantityBytes > 0) {
                        write(sockfd, frame.header, sizeof(char) * strlen(frame.header));
                        write(sockfd, &frame.length, sizeof(short));
                        write(sockfd, frame.data, sizeof(char) * frame.length);
                        /****/

                        free(frame.header);
                        free(frame.data);

                        /** Recibe la trama del servidor **/
                        read(sockfd, &frame.type, sizeof(char));

//                        frame.header = malloc(sizeof(char) * strlen(CONOK));
//                        read(sockfd, frame.header, sizeof(char) * strlen(CONOK));
                        frame.header = get_header_frame(sockfd);
//                        strcpy(frame.header, "CONKO");
                        if (strcmp(frame.header, CONOK) == 0) {
                            read(sockfd, &frame.length, sizeof(short));
                            frame.data = malloc(sizeof(char) * (int) frame.length);
                            read(sockfd, frame.data, sizeof(char) * frame.length);
                            /****/

                            frame.data[frame.length] = '\0';

                            trinity.clientConnections = (Connection *) realloc(trinity.clientConnections,
                                                                               sizeof(Connection) *
                                                                               (trinity.cNumConnections +
                                                                                1));
                            trinity.clientConnections[trinity.cNumConnections].clientSocketFD = sockfd;

                            trinity.clientConnections[trinity.cNumConnections].user = (char *) malloc(
                                    sizeof(char) * strlen(frame.data));
                            strcpy(trinity.clientConnections[trinity.cNumConnections].user, frame.data);
                            trinity.clientConnections[trinity.cNumConnections].active = 1;
                            trinity.clientConnections[trinity.cNumConnections].port = n_port;

                            write(STDOUT_FILENO, port, sizeof(char) * strlen(port));
                            write(STDOUT_FILENO, " connected: ", sizeof(char) * strlen(" connected: "));
                            write(STDOUT_FILENO, trinity.clientConnections[trinity.cNumConnections].user,
                                  sizeof(char) *
                                  strlen(trinity.clientConnections[trinity.cNumConnections].user));
                            write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

                            to_lower_case(trinity.clientConnections[trinity.cNumConnections].user);

                            trinity.cNumConnections++;
                            free(frame.header);
                            free(frame.data);
                        } else {
                            write(STDOUT_FILENO,
                                  "Error, no se ha podido realizar la conexion por parte del servidor!\n",
                                  sizeof(char) *
                                  strlen("Error, no se ha podido realizar la conexion por parte del servidor!\n"));
                            close(sockfd);
                            free(frame.header);
                        }

                    } else {
                        write(STDOUT_FILENO, "Error, no se ha podido realizar la conexion!\n",
                              sizeof(char) * strlen("Error, no se ha podido realizar la conexion!\n"));
                        close(sockfd);
                        free(frame.header);
                        free(frame.data);
                    }
                }
            } else {
                write(STDOUT_FILENO, "Error no te puedes conectar 2 veces al mismo servidor!\n",
                      sizeof(char) * strlen("Error no te puedes conectar 2 veces al mismo servidor!\n"));
            }
        } else {
            write(STDOUT_FILENO, "Error no te puedes conectar a tu propio servidor!\n",
                  sizeof(char) * strlen("Error no te puedes conectar a tu propio servidor!\n"));
        }
    } else {
        write(STDOUT_FILENO,
              "Error en el comando!\n",
              sizeof(char) * strlen("Error en el comando!\n"));
    }
    free(conn);
    free(port);
}

/** Funcion encargada de mostrar los puertos los cuales los servidores estan escuchando **/
/** Llama al Script: show_connections.sh **/
void CONSOLE_show_connections() {
    int p;
    p = fork();

    if (p > 0) {
        wait(NULL);
    } else {
        if (p == 0) {
            char *first_port = NULL;
            char *last_port = NULL;
            char *buffer = NULL;
            buffer = (char *) malloc(sizeof(char));

            sprintf(buffer, "%d", trinity.file.direction_ports[0]);
            first_port = (char *) realloc(first_port, sizeof(char) * strlen(buffer));
            sprintf(first_port, "%s", buffer);

            sprintf(buffer, "%d", trinity.file.direction_ports[trinity.file.num_direction_ports - 1]);
            last_port = (char *) realloc(last_port, sizeof(char) * strlen(buffer));
            sprintf(last_port, "%s", buffer);

            write(STDOUT_FILENO, "connections available\n", sizeof(char) * strlen("connections available\n"));
            char *script[] = {"show_connections.sh", first_port, last_port, NULL};

            free(buffer);
            execvp(script[0], script);
        } else {
            write(STDOUT_FILENO, "Error creando el proceso!\n",
                  sizeof(char) * strlen("Error creando el proceso!\n"));
        }
    }
}

/** Funcion encargada de obtener la opcion de la consola (menu) **/
int CONSOLE_get_console_option(char *option) {
    to_lower_case(option);

    if (strcmp(option, "show connections") == 0) {
        return 1;
    } else if (strncmp(option, "connect", 7) == 0) {
        return 2;
    } else if (strncmp(option, "say", 3) == 0) {
        return 3;
    } else if (strncmp(option, "show audios", 11) == 0) {
        return 4;
    } else if (strncmp(option, "download", 8) == 0) {
        return 5;
    } else if (strcmp(option, "exit") == 0) {
        return 6;
    } else {
        return 0;
    }
}

/** Vincula el struct File al struct Trinity **/
void CONSOLE_init_trinity(File file) {
    pthread_mutex_init(&mtx, NULL);
    trinity.file = file;
    trinity.clientConnections = (Connection *) malloc(sizeof(Connection));
}

/******* PRIVADAS *******/
void to_lower_case(char *cadena) {
    for (int i = 0; cadena[i] != '\0'; i++) {
        if ((cadena[i] >= 'A') && (cadena[i] <= 'Z')) {
            cadena[i] = cadena[i] + ('a' - 'A');
        }
    }
}

Frame create_frame(char type, char *header, char *data) {
    int length;
    Frame frame;

    frame.type = type;

    frame.header = malloc(sizeof(char) * strlen(header));
    strcpy(frame.header, header);

    length = (int) strlen(data);
    frame.length = (short) length;

    frame.data = malloc(sizeof(char) * length);
    if (length > 0) {
        strcpy(frame.data, data);
    }
    return frame;
}

void server_send_frame(int fd, char type, char *header, short length, char *data) {
    write(fd, &type, sizeof(char));
    write(fd, header, sizeof(char) * strlen(header));
    write(fd, &length, sizeof(short));
    write(fd, data, sizeof(char) * length);
}

int port_used(int port) {
    for (int i = 0; i < trinity.cNumConnections; i++) {
        if (port == trinity.clientConnections[i].port) {
            return 1;
        }
    }
    return 0;
}

char *get_header_frame(int fd) {
    char *buffer = NULL;
    char caracter = ' ';
    int i = 0;
    int quantityBytes = -1;
    while (caracter != ']' && quantityBytes != 0) {
        quantityBytes = read(fd, &caracter, sizeof(char));
        buffer = (char *) realloc(buffer, i + 1);
        buffer[i] = caracter;
        i++;

        if(quantityBytes == 0){
            write(STDOUT_FILENO, "Error, se ha perdido la conexion con el servidor!\n",
                  sizeof(char) * strlen("Error, se ha perdido la conexion con el servidor!\n"));
        }
    }
    return buffer;
}