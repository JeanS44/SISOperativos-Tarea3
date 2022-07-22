/* Programa de ejemplo de semaphore. */
/* para compilar usar: gcc -o tarea3 tarea3.c -lpthread */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define cantidad_cajas 2

typedef struct
{
    int cantidad_clientes;
    int tiempo_min_caja;
    int tiempo_max_caja;
    int tiempo_min_cliente_caja;
    int tiempo_max_cliente_caja;
    int cantidad_min_productos;
    int cantidad_max_productos;
    sem_t manejo_c1;
    sem_t manejo_c2;
} datos_compartidos_t;

int random_entre(int min, int max);
void *atiende(void *data);

int main(int argc, char *argv[])
{
    int error[cantidad_cajas];
    struct timespec tiempo_ini;
    struct timespec tiempo_fin;
    pthread_t caja[cantidad_cajas];
    datos_compartidos_t datos_compartidos;

    clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);
    for (int i = 0; i < cantidad_cajas; i++)
    {
        error[i] = pthread_create(&caja[i], NULL, atiende, &datos_compartidos);
        if(error[i] == 0){
            error[i] = pthread_create(&consumidor, NULL, consume, &datos_compartidos);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    return EXIT_SUCCESS;
}

int random_entre(int min, int max)
{
    int aux = 0;
    if (max > min)
        aux = random() % (max - min);
    return min + aux;
}