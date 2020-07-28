#include "file.h"

/**  Función encargada de leer los archivos de configuracion **/
File FILE_read_config_file(char *filename) {
    File file;

    int fd;
    int first_port = 0, last_port = 0;

    char *aux;
    fd = open(filename, O_RDWR | O_APPEND, 0666);

    if (fd < 0) {
        write(STDERR_FILENO, "Error: No se ha podido abrir el fichero\n",
              sizeof(char) * strlen("Error: No se ha podido abrir el fichero\n"));
    } else {
        file.username = FILE_read_line(&fd);
        file.audio_file = FILE_read_line(&fd);
        file.ip_num = FILE_read_line(&fd);

        aux = FILE_read_line(&fd);
        file.ip_port = atoi(aux);
        free(aux);

        file.direction = FILE_read_line(&fd);

        aux = FILE_read_line(&fd);
        first_port = atoi(aux);
        free(aux);

        aux = FILE_read_line(&fd);
        last_port = atoi(aux);
        free(aux);

        write(STDOUT_FILENO, "Leido archivo de configuracion!\n",
              sizeof(char) * strlen("Leido archivo de configuracion!\n"));

        file.direction_ports = NULL;
        file.num_direction_ports = 0;
        for (int i = first_port; i <= last_port; i++) {
            file.direction_ports = (int *) realloc(file.direction_ports, sizeof(int) * file.num_direction_ports + 1);
            file.direction_ports[file.num_direction_ports] = i;
            file.num_direction_ports++;
        }

        close(fd);
    }
    return file;
}

char* FILE_read_line(int *fd) {
    char character = ' ';
    char *buffer = NULL;
    int i = 0;

    while (character != '\n') {
        read(*fd, &character, sizeof(char));
        buffer = (char *) realloc(buffer, i + 1);
        buffer[i] = character;

        if (character == '\n') {
            buffer[i] = '\0';
        }
        i++;
    }

    return buffer;
}