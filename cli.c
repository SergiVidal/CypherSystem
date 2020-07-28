#include "cli.h"

/** Función encargada de mostrar por pantalla los archivos de configuracion **/
void CLI_display_config_file(File file) {
    char *ip_port = NULL, *direction_port = NULL;

    write(STDOUT_FILENO, file.username, sizeof(char) * strlen(file.username));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

    write(STDOUT_FILENO, file.audio_file, sizeof(char) * strlen(file.audio_file));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

    write(STDOUT_FILENO, file.ip_num, sizeof(char) * strlen(file.ip_num));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

    ip_port = (char *) malloc(sizeof(int));
    sprintf(ip_port, "%d", file.ip_port);
    write(STDOUT_FILENO, ip_port, sizeof(char) * strlen(ip_port));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
    free(ip_port);

    write(STDOUT_FILENO, file.direction, sizeof(char) * strlen(file.direction));
    write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));

    for (int i = 0; i < file.num_direction_ports; i++) {
        direction_port = (char *) malloc(sizeof(int));
        sprintf(direction_port, "%d", file.direction_ports[i]);
        write(STDOUT_FILENO, direction_port, sizeof(char) * strlen(direction_port));
        write(STDOUT_FILENO, "\n", sizeof(char) * strlen("\n"));
        free(direction_port);
    }
}

/** Función encargada de obtener y comprobar un char[] (String) **/
char* CLI_get_string() {
    char character = ' ';
    char *buffer = NULL;
    int i = 0;

    while (character != '\n') {
        read(STDIN_FILENO, &character, sizeof(char));
        buffer = (char *) realloc(buffer, i + 1);
        buffer[i] = character;

        if (character == '\n') {
            buffer[i] = '\0';
        }
        i++;
    }

    return buffer;
}

/** Función encargada de mostrar el prompt **/
void CLI_show_prompt(char *username) {
    write(STDOUT_FILENO, "$", sizeof(char) * strlen("$"));
    write(STDOUT_FILENO, username, sizeof(char) * strlen(username));
    write(STDOUT_FILENO, ": ", sizeof(char) * strlen(": "));
}

/** Función encargada de obtener y devolver una palabra (split) **/
char *CLI_get_word(char *command, int pos, int params) {
    int length = (int) strlen(command);
    char *aux = NULL;
    int j = 0;
    int word_count = 0;
    int find = 0;
    for (int i = 0; i < length; i++) {
        if (command[i] == ' ' && command[i + 1] != ' ' && word_count < params) {
            word_count++;
        } else if (word_count == params && !find) {
            aux = (char *) realloc(aux, j + 1);
            aux[j] = command[i];
            j++;
        } else if (word_count == pos && command[i] != ' ') {
            find = 1;
            aux = (char *) realloc(aux, j + 1);
            aux[j] = command[i];
            j++;
        }

    }
    aux = (char *) realloc(aux, j + 1);
    aux[j] = '\0';
    return aux;
}