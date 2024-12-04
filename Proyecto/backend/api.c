#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <unistd.h>
#include <sys/syscall.h>  // Para las llamadas al sistema

#define PORT 8888
#define SYS_get_memory_info 552
#define SYS_get_swap 553
#define SYS_get_fault 554
#define SYS_get_activeinactive 555


char* generate_json_response(const char* message) {
    cJSON *json = cJSON_CreateObject();    
    if (json == NULL) {
        return NULL;
    }
    
    cJSON_AddStringToObject(json, "status", "success");
    cJSON_AddStringToObject(json, "message", message);
    
    char *json_response = cJSON_Print(json);    
    cJSON_Delete(json);                        
    return json_response;
}

// USO DE MEMORIA
char* generate_memory_response() {
    long totalram, freeram, usedram, cachedram;
    
    long result = syscall(SYS_get_memory_info, &totalram, &freeram, &usedram, &cachedram);
    if (result != 0) {
        perror("Error al obtener la información de memoria");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return NULL;
    }
    
    cJSON_AddNumberToObject(json, "total", totalram);
    cJSON_AddNumberToObject(json, "free", freeram);
    cJSON_AddNumberToObject(json, "used", usedram);
    cJSON_AddNumberToObject(json, "cache", cachedram);

    char *json_response = cJSON_Print(json);
    cJSON_Delete(json);

    return json_response;
}

// Páginas de memoria de swap usadas y libres
char* generate_overtime_response() {
    unsigned long used_swap_pages;
    unsigned long free_swap_pages;

    long result = syscall(SYS_get_swap, &used_swap_pages, &free_swap_pages);
    if (result != 0) {
        printf("Error al obtener la información de swap\n");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(json, "used_swap_pages", used_swap_pages);
    cJSON_AddNumberToObject(json, "free_swap_pages", free_swap_pages);

    char *json_response = cJSON_Print(json);
    cJSON_Delete(json);

    return json_response;
}

// Fallos de página
char* generate_fault_response() {
    unsigned long minor_faults;
    unsigned long major_faults;

    long result = syscall(SYS_get_fault, &minor_faults, &major_faults);
    if (result != 0) {
        printf("Error al obtener información de fallos de página\n");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(json, "minor", minor_faults);
    cJSON_AddNumberToObject(json, "major", major_faults);

    char *json_response = cJSON_Print(json);
    cJSON_Delete(json);

    return json_response;
}

// Páginas activas e inactivas
char* generate_pages_response() {
    unsigned long active_pages;
    unsigned long inactive_pages;

    long result = syscall(SYS_get_activeinactive, &active_pages, &inactive_pages);
    if (result != 0) {
        printf("Error al obtener información de páginas de memoria\n");
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        return NULL;
    }

    cJSON_AddNumberToObject(json, "active", active_pages);
    cJSON_AddNumberToObject(json, "inactive", inactive_pages);

    char *json_response = cJSON_Print(json);
    cJSON_Delete(json);

    return json_response;
}


char* generate_processes_response() {
    cJSON *json_array = cJSON_CreateArray();
    if (json_array == NULL) {
        return NULL;
    }


    cJSON *json1 = cJSON_CreateObject();
    if (json1 == NULL) {
        cJSON_Delete(json_array);
        return NULL;
    }
    cJSON_AddStringToObject(json1, "name", "proceso1");
    cJSON_AddNumberToObject(json1, "PID", 1234);
    cJSON_AddNumberToObject(json1, "percentage", 25);
    cJSON_AddItemToArray(json_array, json1);  

    cJSON *json2 = cJSON_CreateObject();
    if (json2 == NULL) {
        cJSON_Delete(json_array);
        return NULL;
    }
    cJSON_AddStringToObject(json2, "name", "proceso2");
    cJSON_AddNumberToObject(json2, "PID", 5678);
    cJSON_AddNumberToObject(json2, "percentage", 15);
    cJSON_AddItemToArray(json_array, json2);  

    char *json_response = cJSON_Print(json_array);
    cJSON_Delete(json_array);

    return json_response;
}

// Función de manejo de solicitudes
enum MHD_Result request_handler(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method, const char *version,
                                const char *upload_data, size_t *upload_data_size, void **ptr) {
    const char *response_str;

    if (strcmp(url, "/") == 0) {
        response_str = generate_json_response("API en C funcionando correctamente!");
    } else if (strcmp(url, "/memory") == 0) {
        response_str = generate_memory_response();
    } else if (strcmp(url, "/overtime") == 0) {
        response_str = generate_overtime_response();
    } else if (strcmp(url, "/fault") == 0) {
        response_str = generate_fault_response();
    } else if (strcmp(url, "/pages") == 0) {
        response_str = generate_pages_response();
    } else if (strcmp(url, "/processes") == 0) {
        response_str = generate_processes_response();
    } else {
        response_str = generate_json_response("Endpoint no encontrado.");
    }

    if (response_str == NULL) {
        return MHD_NO;
    }

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str,
                                                                    MHD_RESPMEM_MUST_FREE);

    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");  // Habilitar CORS

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret == MHD_YES ? MHD_YES : MHD_NO;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, 
                              &request_handler, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        printf("Error al iniciar el servidor.\n");
        return 1;
    }
    printf("Servidor en el puerto %d\n", PORT);

    getchar();
    MHD_stop_daemon(daemon);
    return 0;
}
