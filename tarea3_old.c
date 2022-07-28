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
    int capacidad_almacen;
    float* almacen;
    int cantidad_clientes;
    int tiempo_min_caja;
    int tiempo_max_caja;
    int tiempo_min_cliente;
    int tiempo_max_cliente;
    sem_t puede_producir;
    sem_t puede_consumir;
} datos_compartidos_t;

int random_entre(int min, int max);
void* cliente(void* data);
void* cajera(void *data);

int main(int argc, char *argv[])
{
    int error[cantidad_cajas];
    struct timespec tiempo_ini;
    struct timespec tiempo_fin;
    pthread_t productor[cantidad_cajas];
    pthread_t consumidor[cantidad_cajas];
    datos_compartidos_t datos_compartidos[cantidad_cajas];

    //desde aqui asigno variables de datos_compartidos

    datos_compartidos[0].capacidad_almacen = 15;

    datos_compartidos[0].tiempo_min_cliente=0;
    datos_compartidos[0].tiempo_max_cliente=100;
    datos_compartidos[0].tiempo_min_caja=20;
    datos_compartidos[0].tiempo_max_caja=200;
    /*
    if (argc == 5) {
        datos_compartidos[0].tiempo_min_cliente=atoi(argv[1]);
        datos_compartidos[0].tiempo_max_cliente=atoi(argv[2]);
        datos_compartidos[0].tiempo_min_caja=atoi(argv[3]);
        datos_compartidos[0].tiempo_max_caja=atoi(argv[4]);
    } else {
        printf("Usar: %s demora_min_productor demora_max_productor demora_min_consumidor demora_max_consumidor\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    */
    for(int i =0; i < cantidad_cajas; i++){

        datos_compartidos[i].tiempo_min_cliente=datos_compartidos[0].tiempo_min_cliente;
        datos_compartidos[i].tiempo_max_cliente=datos_compartidos[0].tiempo_max_cliente;
        datos_compartidos[i].tiempo_min_caja=datos_compartidos[0].tiempo_min_caja;
        datos_compartidos[i].tiempo_max_caja=datos_compartidos[0].tiempo_max_caja;
        
        datos_compartidos[i].capacidad_almacen = datos_compartidos[0].capacidad_almacen;
        datos_compartidos[i].cantidad_clientes = 10;

        datos_compartidos[i].almacen = (float*) calloc(datos_compartidos[i].capacidad_almacen, sizeof(float));
        sem_init(&datos_compartidos[i].puede_producir, 0, datos_compartidos[i].capacidad_almacen);
        sem_init(&datos_compartidos[i].puede_consumir, 0, 0);

    }
    
    //hasta aqui

    clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);

    for (int i = 0; i < cantidad_cajas; i++)
    {
        error[i] = pthread_create(&productor[i], NULL, cliente, &datos_compartidos[i]);
        if(error[i] == 0){
            error[i] = pthread_create(&consumidor[i], NULL, cajera, &datos_compartidos[i]);
            if(error[i] !=0){
                printf("error: no puede crear consumidor n°%i \n", i);
                error[i] = 1;
            }
        }else{
            printf("error: no puede crear productor n°%i \n", i);
            error[i] = 1;
        }

        if (error == 0) {
        pthread_join(productor[i], NULL);
        pthread_join(consumidor[i], NULL);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    return EXIT_SUCCESS;
}

void* cliente(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {
    printf("INICIO RONDA P: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->capacidad_almacen; ++indice) {
      sem_wait(&datos_compartidos->puede_producir);
      usleep(1000 * random_entre(datos_compartidos->tiempo_min_cliente, datos_compartidos->tiempo_max_cliente)); //lo que se demora en producir
      datos_compartidos->almacen[indice] = ++contador;
      printf("Indice almacen %i se produce %lg\n", indice, datos_compartidos->almacen[indice]);
      sem_post(&datos_compartidos->puede_consumir);
    }
  }
  return NULL;
}

void* cajera(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {
    printf("\t\tINICIO RONDA C: %i\n", ronda);
    for (int indice = 0; indice < datos_compartidos->capacidad_almacen; ++indice) {
      sem_wait(&datos_compartidos->puede_consumir);
      float value = datos_compartidos->almacen[indice];
      usleep(1000 * random_entre(datos_compartidos->tiempo_min_caja, datos_compartidos->tiempo_max_caja));
      printf("\t\tIndice almacen %i se consume %lg\n", indice, value);
      sem_post(&datos_compartidos->puede_producir);
    }
  }
  return NULL;
}

int random_entre(int min, int max)
{
    int aux = 0;
    if (max > min)
        aux = random() % (max - min);
    return min + aux;
}