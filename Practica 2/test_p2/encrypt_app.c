#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>

#define sys_my_encrypt 550  


int main(int argc, char *argv[]) {
    char *input_path = NULL;
    char *output_path = NULL;
    char *key_path = NULL;
    int thread_count = 0;
    int opt;

    // Procesar los argumentos de la línea de comandos
    while ((opt = getopt(argc, argv, "a:b:c:d:")) != -1) {
        switch (opt) {
            case 'a':
                input_path = optarg;
                break;
            case 'b':
                output_path = optarg;
                break;
            case 'c':
                thread_count = atoi(optarg);
                break;
            case 'd':
                key_path = optarg;
                break;
            default:
                return EXIT_FAILURE;
        }
    }

  
    if (!input_path || !output_path || thread_count <= 0 || !key_path) {
        fprintf(stderr, "Error: Hacen falta parametros.\n");
        return EXIT_FAILURE;
    }

   
    long status = syscall(sys_my_encrypt, input_path, output_path, thread_count, key_path);

    if (status == 0) {
        printf("Encriptacion con exitoso\n");
    } else {
        perror("Error en el proceso de encriptación");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
