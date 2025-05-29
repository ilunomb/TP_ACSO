#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int start, pid, n;
    int buffer[1];

    if (argc != 4) {
        fprintf(stderr, "Uso: anillo <n> <c> <s>\n");
        return EXIT_FAILURE;
    }

    n      = atoi(argv[1]);
    buffer[0] = atoi(argv[2]);
    start  = atoi(argv[3]);

    /* Validación: start debe estar en [0, n-1] */
    if (start < 0 || start >= n) {
        fprintf(stderr, "Start fuera de rango\n");
        return EXIT_FAILURE;
    }

    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n",
           n, buffer[0], start);

    int pipes[n][2];
    int padre_env[2];
    int padre_ret[2];

    if (pipe(padre_env) < 0 || pipe(padre_ret) < 0) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < n; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        if (pid == 0) {
            int read_fd, write_fd, buf;

            /* Determino de dónde leo */
            if (i == start) {
                read_fd = padre_env[0];
            } else {
                read_fd = pipes[(i + n - 1) % n][0];
            }

            /* Determino a dónde escribo */
            if (i == (start + n - 1) % n) {
                write_fd = padre_ret[1];
            } else {
                write_fd = pipes[i][1];
            }

            /* Cerrar todos los pipes que no use */
            for (int j = 0; j < n; j++) {
                if (pipes[j][0] != read_fd)  close(pipes[j][0]);
                if (pipes[j][1] != write_fd) close(pipes[j][1]);
            }
            if (read_fd  != padre_env[0])  close(padre_env[0]);
            if (write_fd != padre_ret[1])  close(padre_ret[1]);
            close(padre_env[1]);
            close(padre_ret[0]);

            /* Paso el mensaje */
            read(read_fd, &buf, sizeof(buf));
            buf++;
            write(write_fd, &buf, sizeof(buf));

            close(read_fd);
            close(write_fd);
            exit(0);
        }
    }

    /* Proceso padre */
    close(padre_env[0]);
    close(padre_ret[1]);

    write(padre_env[1], buffer, sizeof(buffer[0]));
    close(padre_env[1]);

    read(padre_ret[0], buffer, sizeof(buffer[0]));
    close(padre_ret[0]);

    for (int i = 0; i < n; i++) wait(NULL);

    printf("Valor final recibido por el padre: %d\n", buffer[0]);
    return 0;
}
