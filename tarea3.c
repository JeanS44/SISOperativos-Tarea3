/* Programa de ejemplo de semaphore. */
/* para compilar usar: gcc -o tarea3 tarea3.c -lpthread */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include <pthread.h>
#include <semaphore.h>

#define cantidad_cajas 2

#ifdef MUTEX
pthread_mutex_t pantalla;
#endif

typedef struct
{
    int id;
    int capacidad_almacen;
    float* almacen;
    int productos;
    int correa;
    int cantidad_clientes;
    int tiempo_min_caja;
    int tiempo_max_caja;
    int tiempo_min_cliente;
    int tiempo_max_cliente;
    sem_t puede_producir;
    sem_t puede_consumir;
    sem_t siguiente;
} datos_compartidos_t;

int random_entre(int min, int max);
void* cliente(void* data);
void* cajera(void *data);

int main(int argc, char *argv[])
{
    initscr();
    erase();
    refresh();

    int error[cantidad_cajas];
    struct timespec tiempo_ini;
    struct timespec tiempo_fin;
    pthread_t productor1;
    pthread_t consumidor[cantidad_cajas];
    datos_compartidos_t datos_compartidos[cantidad_cajas];

    #ifdef MUTEX
	    pthread_mutex_init (&pantalla, NULL); //inicializa variable pantalla tipo mutex
	#endif

    //desde aqui asigno variables de datos_compartidos
    for(int i=0; i < cantidad_cajas; i++){
        datos_compartidos[i].id = i;
        datos_compartidos[i].capacidad_almacen = random_entre(5,15);
        datos_compartidos[i].productos = random_entre(1,20);

        datos_compartidos[i].tiempo_min_cliente=0;
        datos_compartidos[i].tiempo_max_cliente=100;
        datos_compartidos[i].tiempo_min_caja=20;
        datos_compartidos[i].tiempo_max_caja=200;
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
        datos_compartidos[i].cantidad_clientes = 2;

        datos_compartidos[i].almacen = (float*) calloc(datos_compartidos[i].capacidad_almacen, sizeof(float));
        datos_compartidos[i].correa = 0;
        sem_init(&datos_compartidos[i].puede_producir, 0, datos_compartidos[i].capacidad_almacen);
        sem_init(&datos_compartidos[i].puede_consumir, 0, 0);
        sem_init(&datos_compartidos[i].siguiente, 0, 0);
    }
    //hasta aqui

    clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);

for (int i = 0; i < cantidad_cajas; i++)
    {
        error[i] = pthread_create(&productor[i], NULL, cliente, &datos_compartidos[i]);
        if(error[i] == 0){
            error[i] = pthread_create(&consumidor[i], NULL, cajera, &datos_compartidos[i]);
            if(error[i] !=0){
                perror("error: no puede crear consumidor");
                error[i] = 1;
            }
        }else{
            perror("error: no puede crear productor");
            error[i] = 1;
        }
        
        if (error[i] == 0) {
        pthread_join(productor[i], NULL);

        pthread_join(consumidor[i], NULL);

        }
        else{
            exit(-1);
        }
        
    }
    

    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    float periodo = (tiempo_fin.tv_sec - tiempo_ini.tv_sec) + 
            (tiempo_fin.tv_nsec - tiempo_ini.tv_nsec) * 1e-9;
    printf("Tiempo de ejecución: %.9lfs\n", periodo);

    for(int i=0; i < cantidad_cajas; i++){
        sem_destroy(&datos_compartidos[i].puede_consumir);
        sem_destroy(&datos_compartidos[i].puede_producir);
        sem_destroy(&datos_compartidos[i].siguiente);
        free(datos_compartidos[i].almacen);
    }
    return EXIT_SUCCESS;
}

void* cliente(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {

    #ifdef MUTEX
  	pthread_mutex_lock(&pantalla);		
    #endif
    mvprintw(ronda*datos_compartidos->cantidad_clientes,50*datos_compartidos->id,"INICIO RONDA P CAJA %i: %i\n", datos_compartidos->id, ronda);
    refresh();
    #ifdef MUTEX
  	pthread_mutex_unlock(&pantalla);		
    #endif
    for (int indice = 0; indice < datos_compartidos->productos; ++indice) {
        usleep(1000 * random_entre(datos_compartidos->tiempo_min_cliente, datos_compartidos->tiempo_max_cliente)); //lo que se demora en producir
        sem_wait(&datos_compartidos->puede_producir);
        datos_compartidos->almacen[indice] = ++contador;
        datos_compartidos->correa += 1;
        #ifdef MUTEX
  	    pthread_mutex_lock(&pantalla);		
        #endif
        mvprintw(ronda*datos_compartidos->cantidad_clientes+indice+1,50*datos_compartidos->id,"Caja %i ; Indice almacen %i se produce %lg\n", datos_compartidos->id, indice, datos_compartidos->almacen[indice]);
        refresh();
        #ifdef MUTEX
  	    pthread_mutex_unlock(&pantalla);		
        #endif
        sem_post(&datos_compartidos->puede_consumir);
    }
    sem_wait(&datos_compartidos->siguiente);
  }
  return NULL;
}

void* cajera(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {
    //printf("\t\tINICIO RONDA C CAJA %i: %i\n", datos_compartidos->id, ronda);
    for (int indice = 0; indice < datos_compartidos->productos; ++indice) {
      sem_wait(&datos_compartidos->puede_consumir);
      float value = datos_compartidos->almacen[indice];
      usleep(1000 * random_entre(datos_compartidos->tiempo_min_caja, datos_compartidos->tiempo_max_caja));
      datos_compartidos->correa -= 1;
      //printf("\t\tCaja %i ; Indice almacen %i se consume %lg\n", datos_compartidos->id, indice, value);
      if(datos_compartidos->correa < 5){
        sem_post(&datos_compartidos->puede_producir);
      }
    }
    datos_compartidos->productos = random_entre(1,20);
    sem_post(&datos_compartidos->siguiente);
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
