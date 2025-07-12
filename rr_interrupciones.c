#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

struct Proceso {

    char nombre[50];
    int duracion;
    int id;
    char estado[50];
    struct Proceso *siguiente;

};

struct Proceso *inicio = NULL;
struct Proceso *final = NULL;
int controlador = 1;
int en_sigint = 0;
int en_sigusr1 = 0;
int en_sigusr2 = 0;
int en_sigterm = 0;

void solicitar_datos(struct Proceso **nuevo) {

    

    printf("Ingresa el nombre del proceso: ");
    fgets((*nuevo)->nombre , sizeof((*nuevo)->nombre) , stdin);
    (*nuevo)->nombre[strcspn((*nuevo)->nombre , "\n")] = '\0';

    printf("Ingresa ahora su duracion: ");
    scanf("%d" , &(*nuevo)->duracion);
    getchar();
    strcpy((*nuevo)->estado , "Pendiente");
    static int id = 1;
    (*nuevo)->id = id;
    id++;
}


void eliminar_puntero() {
    struct Proceso *temporal = inicio;
    if (inicio->siguiente == inicio) {
        inicio = NULL;
        final = NULL;
        free(temporal);
        return;
    }
    inicio = inicio->siguiente;
    final->siguiente = inicio;
    free(temporal);
}

void liberar_memoria() {
    if (inicio == NULL) return;
    while (inicio != NULL) {
        struct Proceso *temporal = inicio;
        if (inicio->siguiente == inicio) {
            inicio = NULL;
            final = NULL;
            free(temporal);
            return;
        }
        inicio = inicio->siguiente;
        final->siguiente = inicio;
        free(temporal);
    }
}

void agregar_proceso(struct Proceso **inicio , struct Proceso **final) {
    struct Proceso *nuevo = malloc(sizeof(struct Proceso));
    if (nuevo == NULL) {
        printf("[INFO] Ha ocurrido un fallo en la memoria, procedimiento abortado\n");
        return;
    }

    solicitar_datos(&nuevo);
    FILE *archivo = fopen("logs.txt" , "a");
    if (*inicio == NULL) {
        *inicio = nuevo;
        (*inicio)->siguiente = *inicio;
        *final = *inicio;
    } else {
        (*final)->siguiente = nuevo;
        nuevo->siguiente = *inicio;
        *final = nuevo;
    }
    fprintf(archivo , "[LOG] Nuevo proceso agregado: [%s   ID: %d  Duracion: %d  Estado: %s]\n" , nuevo->nombre , nuevo->id , nuevo->duracion , nuevo->estado);
    fclose(archivo);
}


void estado_actual(struct Proceso *actual) {
    if (actual == NULL) {
        printf("[INFO] No hay procesos en cola para mostrar.\n");
        return;
    }
    struct Proceso *inicio = actual;
    while (actual != NULL) {
        printf("[PROCESO] %s  Duracion: %d  ID: %d  Estado: %s\n" , actual->nombre , actual->duracion , actual->id , actual->estado);
        if (actual->siguiente == inicio) {
            break;
        } else {
            actual = actual->siguiente;
        }
    }

}

void ejecutar_procesos(struct Proceso **inicio , struct Proceso **final , int quantum) {
    if (*inicio == NULL) {
        printf("[INFO] No hay procesos en cola para ejecutar.\n");
        return;
    }
    FILE *archivo = fopen("logs.txt" , "a");
    while (*inicio != NULL) {
        while ((*inicio)->duracion > 0) {

            if (en_sigint) {
                printf("\n[!!!] Sennal SIGINT recibida, eliminando proceso %s...\n" , (*inicio)->nombre);
                fprintf(archivo , "\n[LOG] Sennal SIGINT recibida, eliminando proceso %s...\n" , (*inicio)->nombre);
                eliminar_puntero();
                en_sigint = 0;
                if (*inicio == NULL) fclose(archivo);break;
            }

            if (en_sigusr1) {
                printf("\n[INFO] Sennal SIGUSR1 recibida, pausando la ejecucion de procesos durante 3 segundos...\n");
                fprintf(archivo , "\n[LOG] Sennal SIGUSR1 recibida, pausando la ejecucion de procesos durante 3 segundos...\n");
                en_sigusr1 = 0;
                sleep(3);
            }

            if (en_sigusr2) {
                printf("\n[INFO] Sennal SIGUSR2 recibida, aumento temporal de +1 quantum en este proceso\n");
                fprintf(archivo ,"\n[LOG] Sennal SIGUSR2 recibida, aumento temporal de +1 quantum en este proceso\n");
                (*inicio)->duracion -= 1;
                en_sigusr2 = 0;
            }

            

            strcpy((*inicio)->estado , "Ejecutandose");
            printf("[INFO] Ejecutando %s... %d\n", (*inicio)->nombre , (*inicio)->duracion);
            fprintf(archivo , "[LOG] Ejecutando %s... %d\n", (*inicio)->nombre , (*inicio)->duracion);
            (*inicio)->duracion -= quantum;


            if ((*inicio)->duracion <= 0) {
                struct Proceso *temporal = *inicio;
                if ((*inicio)->siguiente == *inicio) {
                    strcpy((*inicio)->estado , "Finalizado");
                    *inicio = NULL;
                    free(temporal);
                    printf("[!!!] Todos los procesos han finalizado\n");
                    fprintf(archivo , "[!!!] Todos los procesos han finalizado\n");
                    fclose(archivo);
                    break;
                }

                strcpy((*inicio)->estado , "Finalizado");
                *inicio = (*inicio)->siguiente;
                (*final)->siguiente = *inicio;
                free(temporal);
                printf("[!!!] Proceso finalizado.\n");
                fprintf(archivo , "[!!!] Proceso finalizado.\n");
                continue;
            } else if ((*inicio)->duracion > 0) {
                fprintf(archivo , "[LOG] Proceso %s inconcluso, movido al final.\n" , (*inicio)->nombre);
                *final = *inicio;
                *inicio = (*inicio)->siguiente;
                
            }

            sleep(1);

            if (en_sigterm) {
                en_sigterm = 0;
                fprintf(archivo , "\n[LOG] Sennal SIGTERM recibida, programa cerrado con exito\n");
                fclose(archivo);
                return;
            }

        }

        if (*inicio != NULL) *inicio = (*inicio)->siguiente;

    }
    
}




int verificar_estado() {
    if (inicio != NULL && strcmp(inicio->estado , "Ejecutandose") == 0) return 1; else return 0;
}

void handler(int signal) {
    int estado = verificar_estado();
    if (signal == SIGINT && estado) {
        en_sigint = 1;
    } else if (signal == SIGUSR1 && estado) {
        en_sigusr1 = 1;
    } else if (signal == SIGUSR2 && estado) {
        en_sigusr2 = 1;
    } else if (signal == SIGTERM) {
        en_sigterm = 1;
        FILE *archivo = fopen("logs.txt" , "a");
        if (!estado) fprintf(archivo , "\n[INFO] Sennal SIGTERM recibida, programa cerrado con exito\n");
        liberar_memoria();
        printf("[INFO] Sennal SIGTERM recibida, programa cerrado con exito.\n");
        fclose(archivo);
        controlador = 0;
    }
    
}

int main() {

    signal(SIGINT ,  handler);
    signal(SIGUSR1 , handler);
    signal(SIGUSR2 , handler);
    signal(SIGTERM , handler);
    
    int quantum = 2;

    int entrada;
    
    while (controlador == 1) {
        printf("\nPID %d\n" , getpid());
        printf("=== ROUND ROBIN CON INTERRUPCIONES ===\n1.Agregar proceso.\n2.Estado actual.\n3.Ejecutar procesos.\n4.Salir\n");
        scanf("%d" , &entrada);
        getchar();

        switch (entrada)
        {
        case 1:
            agregar_proceso(&inicio , &final);
            break;
        
        case 2:
            estado_actual(inicio);
            break;
        case 3:
            ejecutar_procesos(&inicio , &final , quantum);
            break;
        case 4:
            liberar_memoria();
            controlador = 0;
            break;
        }
    }

    return 0;
}