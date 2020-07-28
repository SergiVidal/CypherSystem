#ifndef CYPHERSYSTEM_FILE_H
#define CYPHERSYSTEM_FILE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "trinity.h"

File FILE_read_config_file(char *);

char* FILE_read_line(int *);
#endif
