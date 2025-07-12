#include <stdio.h>
#include <string.h>
#include <unistd.h>


struct Pagina {
    int marco;
    int bit_acceso;
    int bit_modificacion;
};

void solicitar_datos(int *pagina_cargadas , int *pagina_antigua , struct Pagina *tabla , char *memoria ) {

    int pagina;
    int offset;

    printf("Ingresa la pagina a la que quieras acceder: ");
    scanf("%d" , &pagina);
    getchar();


    int cargada = 0;
    for (int i = 0; i < 8; i++) {
        if (pagina == pagina_cargadas[i]) {
            cargada = 1;
            break;
        }
    }

    if (cargada == 0) {
        printf("La pagina no ha sido cargada, iniciando procedimiento...\n");
        sleep(1);
        int temp = tabla[pagina_cargadas[*pagina_antigua]].marco;
        tabla[pagina_cargadas[*pagina_antigua]].marco = -1;
        if (tabla[pagina_cargadas[*pagina_antigua]].bit_modificacion == 1) {
            printf("La pagina que se va a eliminar ha sido modificada, requiere escritura en disco.\n");
        }
        tabla[pagina_cargadas[*pagina_antigua]].bit_acceso = 0;
        tabla[pagina_cargadas[*pagina_antigua]].bit_modificacion = 0;

        pagina_cargadas[*pagina_antigua] = pagina;
        tabla[pagina_cargadas[*pagina_antigua]].marco = temp;
        tabla[pagina_cargadas[*pagina_antigua]].bit_acceso = 0;
        tabla[pagina_cargadas[*pagina_antigua]].bit_modificacion = 0;

        for (int i = 0; i < 4; i++) {
            memoria[tabla[pagina].marco * 4 + i] = 0;
        }

        if (*pagina_antigua == 7) {
            *pagina_antigua = 0;
        } else {
            *pagina_antigua = *pagina_antigua + 1;
        }

    }

    printf("Ingresa el offset: ");
    scanf("%d" , &offset);
    getchar();

    int modo;
    printf("De que modo quieres acceder a la pagina? Escritura (1) Lectura (0): ");
    scanf("%d" , &modo);
    getchar();

    if (modo == 1) {
        int valor;
        printf("Ingresa el valor que quieres annadir (0-255): ");
        scanf("%d" , &valor);
        getchar();

        memoria[tabla[pagina].marco * 4 + offset] = valor;
        tabla[pagina].bit_acceso = 1;
        tabla[pagina].bit_modificacion = 1;
        printf("Pagina %d cargada correctamente y byte fisico %d actualizado.\n" , pagina , tabla[pagina].marco * 4 + offset);
        return;
    } else if (modo == 0) {
        printf("Valor del byte al que quieres acceder: %d\n" , memoria[tabla[pagina].marco * 4 + offset]);
        tabla[pagina].bit_acceso = 1;
        return;
    }

}

void estado_pagina(struct Pagina *tabla) {
    for (int i = 0; i < 16; i++) {
        printf("[%d] Marco Asignado: %d [BitA: %d] [BitM %d]\n" , i , tabla[i].marco , tabla[i].bit_acceso , tabla[i].bit_modificacion);
    }
}

void contenido_memoria(char *memoria , struct Pagina *tabla) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            printf("[%d] " , memoria[i * 4 + j]);
        }
        if (tabla[i].marco != -1) {
            printf("<- Usando marco %d" , tabla[i].marco);
        }
        printf("\n");
    }
}

void paginas_cargadass(int *paginas_cargadas , int *pagina_antigua) {
    printf("\nPaginas cargadas\n");
    for (int i = 0; i < 8; i++) {
        printf("Pagina %d\n" , paginas_cargadas[*pagina_antigua]);
        *pagina_antigua = *pagina_antigua + 1;
        if (*pagina_antigua == 8) {
            *pagina_antigua = 0;
        }
    }
}

void resetear_bits(struct Pagina *tabla) {
    for (int i = 0; i < 16; i++) {
        tabla[i].bit_acceso = 0;
        tabla[i].bit_modificacion = 0;
    }
    printf("Reset exitoso!\n");
}

int main() {

    unsigned char memoria[64] = {0};
    
    struct Pagina tabla_paginas[16] = {
  {3, 0, 0},   // Página 0 → Marco 3
  {0, 0, 0},   // Página 1 → Marco 0
  {1, 0, 0},   // Página 2 → Marco 1
  {5, 0, 0},   // Página 3 → Marco 5
  {4, 0, 0},   // Página 4 → Marco 4
  {8, 0, 0},   // Página 5 → Marco 8
  {10, 0, 0},  // Página 6 → Marco 10
  {2, 0, 0},   // Página 7 → Marco 2
  {-1, 0, 0},  // Página 8 → No asignada
  {-1, 0, 0},  // Página 9 → No asignada
  {-1, 0, 0},  // Página 10 → No asignada
  {-1, 0, 0},  // Página 11 → No asignada
  {-1, 0, 0},  // Página 12 → No asignada
  {-1, 0, 0},  // Página 13 → No asignada
  {-1, 0, 0},  // Página 14 → No asignada
  {-1, 0, 0}   // Página 15 → No asignada
};

    int paginas_cargadas[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int pagina_antigua = 0;

    int controlador = 1;
    int entrada;

    while (controlador == 1) {
        printf("=== SIMULADOR DE PAGINACION CON FIFO ===\n1.Acceder a una direccion virtual.\n2.Mostrar estado de las paginas.\n3.Mostrar contenido de memoria.\n4.Mostrar paginas cargadas.\n5.Resetear bits\n6.Salir\n");
        scanf("%d" , &entrada);
        getchar();

        switch (entrada)
        {
        case 1:
            solicitar_datos(paginas_cargadas , &pagina_antigua , tabla_paginas , memoria);
            break;
        
        case 2:
            estado_pagina(tabla_paginas);
            break;
        case 3:
            contenido_memoria(memoria , tabla_paginas);
            break;
        case 4:
            paginas_cargadass(paginas_cargadas , &pagina_antigua);
            break;
        case 5:
            resetear_bits(tabla_paginas);
            break;
        case 6:
            printf("Hasta luego :)\n");
            controlador = 0;
            break;
        }
    }


    
    return 0;
}