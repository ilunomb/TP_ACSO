
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') return -1;

    int curr_inumber = 1;  // inodo raíz
    const char *curr = pathname + 1;  // saltar el primer '/'

    char name[14];
    while (*curr != '\0') {
        // Extraer siguiente componente entre '/' o fin
        int len = 0;
        while (*curr != '/' && *curr != '\0' && len < 14) {
            name[len++] = *curr++;
        }
        name[len] = '\0';

        // Saltar '/' extra si hubiera
        if (*curr == '/') curr++;

        // Buscar nombre en directorio actual
        struct direntv6 dirEnt;
        if (directory_findname(fs, name, curr_inumber, &dirEnt) < 0) {
            return -1;
        }

        curr_inumber = dirEnt.d_inumber;

        // Extra: validar que el siguiente componente sea directorio si hay más path por resolver
        if (*curr != '\0') {
            struct inode in;
            if (inode_iget(fs, curr_inumber, &in) < 0) return -1;
            if (!inode_is_directory(&in)) return -1;
        }
    }

    return curr_inumber;
}

