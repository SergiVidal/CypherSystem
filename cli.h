#ifndef CYPHERSYSTEM_CLI_H
#define CYPHERSYSTEM_CLI_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "trinity.h"

void CLI_display_config_file(File);

char* CLI_get_string();

void CLI_show_prompt(char*);

char *CLI_get_word(char*, int, int);
#endif
