/***************************/
/** Sergi Vidal - s.vidal **/
/***************************/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "file.h"
#include "console.h"

/** /users/home/alumnes/LS/s.vidal/GTAS2/Trinity **/
/** Trinity config.dat **/
/** valgrind --leak-check=full --track-origins=yes Trinity config.dat **/

/** Funcion encargada de reprogramar el SIGINT **/
void ksigint() {
    CONSOLE_exit();
    raise(SIGKILL);
}

/** Funcion encargada de reprogramar el SIGPIPE **/
void ksigpipe(){}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        signal(SIGINT, ksigint);
        signal(SIGPIPE, ksigpipe);

        char *command;
        int option;
        File file;
        pthread_t threadServer;

        file = FILE_read_config_file(argv[1]);
        CONSOLE_init_trinity(file);
        pthread_create(&threadServer, NULL, CONSOLE_threadServer, &file.ip_port);

        CLI_display_config_file(file);

        do {
            CLI_show_prompt(file.username);
            command = CLI_get_string();
            option = CONSOLE_get_console_option(command);

            switch (option) {
                case 1:
                    CONSOLE_show_connections();
                    break;
                case 2:
                    CONSOLE_connect(command);
                    break;
                case 3:
                    CONSOLE_say(command);
                    break;
                case 4:
                    CONSOLE_show_audios(command);
                    break;
                case 5:
                    CONSOLE_download(command);
                    break;
                case 6:
                    CONSOLE_exit();
                    break;
                default:
                    write(STDOUT_FILENO, "Opcion incorrecta!\n", sizeof(char) * strlen("Opcion incorrecta!\n"));
                    break;
            }
            free(command);
        } while (option != 6);

        return 0;
    } else {
        write(STDERR_FILENO, "Error: Tienes que introducir 1 argumento\n",
              sizeof(char) * strlen("Error: Tienes que introducir 1 argumento\n"));
        return 1;
    }

}