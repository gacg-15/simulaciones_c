#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Proceso {

    char nombre[50];
    int id;
    int prioridad;
    int rafaga;
    struct Proceso *siguiente;

};

void crear_proceso(struct Proceso **inicio , struct Proceso **final) {

    struct Proceso *nuevo = malloc(sizeof(struct Proceso));
    if (nuevo == NULL) {
        printf("Ha ocurrido un fallo en la memoria, saliendo...\n");
        return;
    }

    printf("Ingresa el nombre del proceso: ");
    fgets(nuevo->nombre , sizeof(nuevo->nombre) , stdin);
    nuevo->nombre[strcspn(nuevo->nombre , "\n")] = '\0';

    printf("Ingresa su prioridad: ");
    scanf("%d" , &nuevo->prioridad);
    getchar();

    printf("Ingresa ahora su rafaga (duracion): ");
    scanf("%d" , &nuevo->rafaga);
    getchar();

    static int id = 1;
    nuevo->id = id;
    id++;

    if (*inicio == NULL) {
        nuevo->siguiente = NULL;
        *inicio = nuevo;
        *final = *inicio;
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

void reubicar_puntero( struct Proceso **intermedio , struct Proceso *actual) {
    while(actual!= NULL) {
        if (actual->prioridad <= 4) *intermedio = actual;
        actual = actual->siguiente;
    }
}


void estado_actual(struct Proceso *inicio) {
    
    if (inicio == NULL) {
        printf("[INFO] No hay proceso en cola para mostrar.\n");
        return;
    }

    while (inicio != NULL ) {
        if (inicio->prioridad <= 4) {
            printf("[INFO] INICIO DE LA COLA QUE SE EJECUTARA CON ROUND ROBIN\n");
            while (inicio != NULL && inicio->prioridad <= 4) {
                printf("[ID %d] %s  Prioridad: %d  Rafaga: %d\n" , inicio->id , inicio->nombre , inicio->prioridad , inicio->rafaga);
                inicio = inicio->siguiente;
            }
        } else if (inicio->prioridad >= 5) {
            printf("\n[INFO] INICIO DE LA COLA QUE SE EJECUTARA CON FIFO\n");
            while (inicio != NULL) {
                printf("[ID %d] %s  Prioridad: %d  Rafaga: %d\n" , inicio->id , inicio->nombre , inicio->prioridad , inicio->rafaga);
                inicio = inicio->siguiente;
            }
            return;
        }
    }

}

void ejecutar_prcesos(struct Proceso **inicio , struct Proceso **intermedio , struct Proceso **final , int quantum) {

    while (*inicio != NULL) {
        if ((*inicio)->prioridad <= 4) {  // Round Robin
            struct Proceso *temporal = (*intermedio)->siguiente;
            (*intermedio)->siguiente = *inicio;
            (*inicio)->rafaga -= quantum;
            if ((*inicio)->rafaga > 0) {
                for (int i = 0; i < quantum; i++) {
                    printf("[INFO] Ejecutando Round Robin - Completado %d/%d\n" , i + 1 , quantum);
                }
                printf("[INFO] Proceso inconcluso, movido al final.\n\n");

                *intermedio = *inicio;
                *inicio = (*inicio)->siguiente;
                (*intermedio)->siguiente = temporal;
                return;
            } else {
                for (int i = 0; i < (*inicio)->rafaga + quantum; i++) {
                    printf("[INFO] Ejecutando Round Robin - Completado %d/%d\n" , i + 1 , (*inicio)->rafaga + quantum);
                }
                printf("[!!!] Proceso finalizado\n\n");

                struct Proceso *aux = *inicio;
                (*intermedio)->siguiente = temporal;
                *inicio = (*inicio)->siguiente;
                
                free(aux);
                return;
            }

        } else {   // FIFO 
            struct Proceso *aux = *inicio;
            for (int i = 0; i < (*inicio)->rafaga; i++) {
                printf("[INFO] Ejecutando FIFO - Completado %d/%d\n" , i + 1 , (*inicio)->rafaga);
            }
            printf("[!!!] Proceso finalizado\n\n");

            *inicio = (*inicio)->siguiente;
            free(aux);
            return;
        }
    }
}

void liberar_memoria(struct Proceso *actual) {
    if (actual == NULL) return;

    while (actual != NULL) {
        struct Proceso *temporal = actual;
        actual = actual->siguiente;
        free(temporal);
    }
}

int main() {

    struct Proceso *inicio = NULL;
    struct Proceso *intermedio = NULL;
    struct Proceso *final = NULL;

    int quantum = 3;

    int entrada;
    int controlador = 1;

    while (controlador == 1) {
        printf("\n=== SIMULACION DE PLANIFICADOR HIBRIDO ===\n1.Agregar un proceso\n2. Ver estado actual.\n3. Ejecutar siguiente proceso.\n4. Salir. \n >    ");
        scanf("%d" , &entrada);
        getchar();

        switch (entrada)
        {
        case 1:
            crear_proceso(&inicio , &final);
            reubicar_puntero(&intermedio , inicio);
            break;
        case 2:
            estado_actual(inicio);
            break;
        case 3:
            if (inicio == NULL) {
                printf("[INFO] No hay procesos en cola para ejecutar.\n");
            } else {
                while (inicio != NULL) {
                    ejecutar_prcesos(&inicio , &intermedio , &final , quantum);
                }
            }
            break;
        case 4:
            liberar_memoria(inicio);
            printf("Hasta luego :)\n");
            controlador = 0;
            break;
        }
    }

    return 0;
}