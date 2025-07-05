#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>


struct Proceso {

    char nombre[50];
    int duracion;
    int prioridad;
    char estado[50];
    struct Proceso *siguiente;
};

struct Proceso *inicio = NULL;
struct Proceso *final = NULL;
struct Proceso *pivote = NULL;
int en_sigusr1 = 0;
int en_sigusr2 = 0;

void solicitar_datos(struct Proceso **nuevo) {

    

    printf("Ingresa el nombre del proceso: ");
    fgets((*nuevo)->nombre , sizeof((*nuevo)->nombre) , stdin);
    (*nuevo)->nombre[strcspn((*nuevo)->nombre , "\n")] = '\0';

    printf("Ingresa su prioridad: ");
    scanf("%d" , &(*nuevo)->prioridad);
    getchar();

    printf("Ingresa ahora su rafaga (duracion): ");
    scanf("%d" , &(*nuevo)->duracion);
    getchar();

    (*nuevo)->siguiente = NULL;
    strcpy((*nuevo)->estado , "Pendiente");

}

void crear_proceso(struct Proceso **inicio , struct Proceso **final , struct Proceso **pivote) {

    struct Proceso *nuevo = malloc(sizeof(struct Proceso));
    if (!nuevo) {
        printf("[INFO] Ha ocurrido un fallo en la memoria.\n");
        return;
    }

    solicitar_datos(&nuevo);

     if (*inicio == NULL) {
        nuevo->siguiente = NULL;
        *inicio = nuevo;
        *final = *inicio;
        *pivote = *inicio;
    } else {
        struct Proceso *actual = *inicio;
        struct Proceso *anterior = NULL;

        while (actual != NULL) {
            if ((actual->siguiente != NULL) && (actual->siguiente->prioridad == actual->prioridad || actual->siguiente->prioridad <= nuevo->prioridad)) {
                anterior = actual;
                actual = actual->siguiente;
                continue;
            }

            if (anterior == NULL && nuevo->prioridad < actual->prioridad) {
                nuevo->siguiente = *inicio;
                *inicio = nuevo;
                *pivote = *inicio;
                return;
            }

            nuevo->siguiente = actual->siguiente;
            actual->siguiente = nuevo;
            if (nuevo->siguiente == NULL) {
                *final = nuevo;
            }
            return;

        }
    }
}

void estado_actual(struct Proceso *actual) {
    if (actual == NULL) {
        printf("[INFO] No hay procesos en cola para mostrar.\n");
        return;
    }
    while (actual != NULL) {
        printf("[PROCESO] %s  Duracion: %d  Prioridad: %d  Estado: %s\n" , actual->nombre , actual->duracion , actual->prioridad , actual->estado);
        actual = actual->siguiente;
    }
}

void reubicar_proceso() {
    struct Proceso *actual = inicio;
    struct Proceso *anterior = NULL;

    while (actual != NULL) {
        if (actual == pivote) {
            if (anterior == NULL && actual->siguiente == NULL) {
                break;

            } else  {
                while (actual->siguiente != NULL && actual->prioridad >= actual->siguiente->prioridad) {
                    struct Proceso *temporal = actual->siguiente->siguiente;
                    anterior->siguiente = actual->siguiente;
                     actual->siguiente->siguiente = actual;
                    actual->siguiente = temporal;
                   
                    pivote = anterior->siguiente;
                    anterior = anterior->siguiente;
                }
                return;
            }
        }
    anterior = actual;
    actual = actual->siguiente;
    }
}

void ejecutar_proceso(struct Proceso **pivote) {
    while (*pivote != NULL) {   
        while ((*pivote)->duracion > 0) {

             if (en_sigusr1 == 1) {
                printf("\nSennal SIGUSR1 recibida, duracion del proceso aumentada en 5.\n");
                (*pivote)->duracion += 5;
                en_sigusr1 = 0;
            }

            if (en_sigusr2 == 1) {
                printf("\nSennal SIGUSR2 recibida, prioridad del proceso actual aumentada en 3.\n");
                if ((*pivote)->prioridad + 3 > 10) {
                    (*pivote)->prioridad = 10;
                } else {
                    (*pivote)->prioridad += 3;
                }
                reubicar_proceso();
                sleep(1);
                en_sigusr2 = 0;
            }

            printf("[INFO] Ejecutando proceso %s... %d\n" , (*pivote)->nombre , (*pivote)->duracion);
            (*pivote)->duracion -= 1;
            strcpy((*pivote)->estado , "Ejecutandose");

            if ((*pivote)->duracion == 0) {
                printf("[!!!] Proceso %s finalizado.\n\n" , (*pivote)->nombre);
                strcpy((*pivote)->estado , "Finalizado");
                *pivote = (*pivote)->siguiente;
                
            }

           
            sleep(1);
            
            if (*pivote == NULL) break;
        }

    if (*pivote != NULL) *pivote = (*pivote)->siguiente;
    }
}

void eliminar_proceso() {
    struct Proceso *actual = inicio;
    struct Proceso *anterior = NULL;

    while (actual != NULL) {
        if (actual == pivote) {
            printf("\n[!!!] Abortando y eliminando el proceso %s\n\n" , actual->nombre);
            struct Proceso *temporal = actual;
            if (anterior == NULL) {
                inicio = inicio->siguiente;
            } else {
                anterior->siguiente = actual->siguiente;
            }

            free(temporal);
            break;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

}



int verificar_estado() {
    if (pivote == NULL) return 0;
    if (strcmp(pivote->estado , "Ejecutandose") == 0) return 1; else return 0;
}

void handler(int signal) {
    int estado = verificar_estado();
    if (signal == SIGINT) {
        if (pivote != NULL && estado == 1) {
            eliminar_proceso();
            pivote = inicio;
        }
    } else if (signal == SIGUSR1) {
        if (pivote != NULL && estado == 1) {
            en_sigusr1 = 1;
        }
    } else if (signal == SIGUSR2) {
        if (pivote != NULL && estado == 1) {
            en_sigusr2 = 1;
        }
    }
}

void liberar_memoria() {
    if (inicio == NULL) return;
    while (inicio != NULL) {
        struct Proceso *temporal = inicio;
        inicio = inicio->siguiente;
        free(temporal);
    }
}

int main() {



    int entrada;
    int controlador = 1;

    signal(SIGINT , handler);
    signal(SIGUSR1 , handler);
    signal(SIGUSR2 , handler);

    while (controlador == 1) {
        printf("\nPID %d\n" , getpid());
        printf("=== MANEJO DE INTERRUPCIONES CON PRIORIDAD ===\n1.Agregar proceso.\n2.Ver estado actual\n3.Ejecutar procesos.\n4.Salir.\n");
        scanf("%d" , &entrada);
        getchar();

        switch (entrada)
        {
        case 1:
            crear_proceso(&inicio , &final , &pivote);
            break;
        case 2:
            estado_actual(inicio);
            break;
        case 3:
            ejecutar_proceso(&pivote);
            pivote = inicio;
            break;
        case 4:
            liberar_memoria();
            controlador = 0;
            break;
        }

    }


    return 0;
}