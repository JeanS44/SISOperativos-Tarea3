/* Programa de ejemplo de semaphore. */
/* para compilar usar: gcc -o tarea3 tarea3.c -lpthread */
/* ./tarea3 0 100 20 200 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define cantidad_cajas 2

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
    int error[cantidad_cajas];
    struct timespec tiempo_ini;
    struct timespec tiempo_fin;
    pthread_t productor1;
    pthread_t consumidor1;
    pthread_t productor2;
    pthread_t consumidor2;
    datos_compartidos_t datos_compartidos[cantidad_cajas];

    //desde aqui se asignan variables de datos_compartidos
    if (argc == 5) {
        datos_compartidos[0].tiempo_min_cliente=atoi(argv[1]);
        datos_compartidos[0].tiempo_max_cliente=atoi(argv[2]);
        datos_compartidos[0].tiempo_min_caja=atoi(argv[3]);
        datos_compartidos[0].tiempo_max_caja=atoi(argv[4]);
    } else {
        printf("Usar: %s demora_min_productor demora_max_productor demora_min_consumidor demora_max_consumidor\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for(int i=0; i < cantidad_cajas; i++){
        datos_compartidos[i].id = i;
        datos_compartidos[i].capacidad_almacen = random_entre(5,15);
        datos_compartidos[i].productos = random_entre(1,20);

        datos_compartidos[i].tiempo_min_cliente=datos_compartidos[0].tiempo_min_cliente;
        datos_compartidos[i].tiempo_max_cliente=datos_compartidos[0].tiempo_max_cliente;
        datos_compartidos[i].tiempo_min_caja=datos_compartidos[0].tiempo_min_caja;
        datos_compartidos[i].tiempo_max_caja=datos_compartidos[0].tiempo_max_caja;

        datos_compartidos[i].cantidad_clientes = random_entre(1,10);

        datos_compartidos[i].almacen = (float*) calloc(datos_compartidos[i].capacidad_almacen, sizeof(float));
        datos_compartidos[i].correa = 0;
        sem_init(&datos_compartidos[i].puede_producir, 0, datos_compartidos[i].capacidad_almacen);
        sem_init(&datos_compartidos[i].puede_consumir, 0, 0);
        sem_init(&datos_compartidos[i].siguiente, 0, 0);
    }

    //se inicia el cronometro
    clock_gettime(CLOCK_MONOTONIC, &tiempo_ini);

    //se crean e inician los hilos, 2 por cliente y 2 por caja, 4 hilos en total
    error[0] = pthread_create(&productor1, NULL, cliente, &datos_compartidos[0]);
    error[1] = pthread_create(&productor2, NULL, cliente, &datos_compartidos[1]);
    if(error[0] == 0 && error[1] == 0){
        error[0] = pthread_create(&consumidor1, NULL, cajera, &datos_compartidos[0]);
        error[1] = pthread_create(&consumidor2, NULL, cajera, &datos_compartidos[1]);
        if(error[0] !=0 && error[1] != 0){
            printf("error: no puede crear consumidor");
            error[0] = 1;
            error[1] = 1;
        }
    }else{
        printf("error: no puede crear productor");
        error[0] = 1;
        error[1] = 1;
    }

    if (error[0] == 0 && error[1] == 0) {
    pthread_join(productor1, NULL);
    pthread_join(productor2, NULL);
    pthread_join(consumidor1, NULL);
    pthread_join(consumidor2, NULL);
    }
    else{
        exit(-1);
    }
    
    //se pone fin al cronometro y se imprime el tiempo
    clock_gettime(CLOCK_MONOTONIC, &tiempo_fin);
    float periodo = (tiempo_fin.tv_sec - tiempo_ini.tv_sec) + 
            (tiempo_fin.tv_nsec - tiempo_ini.tv_nsec) * 1e-9;
    printf("Tiempo de ejecución: %.9lfs\n", periodo);

    //se destruyen los semaforos creaados, se desasigna memoria de la funcion calloc y se termina de ejecutar el programa
    for(int i=0; i < cantidad_cajas; i++){
        sem_destroy(&datos_compartidos[i].puede_consumir);
        sem_destroy(&datos_compartidos[i].puede_producir);
        sem_destroy(&datos_compartidos[i].siguiente);
        free(datos_compartidos[i].almacen);
    }
    return EXIT_SUCCESS;
}

//funcion hilo del cliente, esta se estara ejecutando 2 veces, una por cada caja
void* cliente(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  int contador = 0;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {    //se itera por la cantidad de clientes, siendo "ronda" el cliente en atencion
    printf("CAJA %i ; CLIENTES: %i ; PRODUCTOS: %i ; ALMACEN: %i\n", datos_compartidos->id, datos_compartidos->cantidad_clientes, datos_compartidos->productos, datos_compartidos->capacidad_almacen);
    printf("CAJA %i ; INICIO RONDA P: %i\n", datos_compartidos->id, ronda);
    for (int indice = 0; indice < datos_compartidos->productos; ++indice) {       //se itera por la cantidad de productos, siendo indice el producto que se pondra en almacen
        usleep(1000 * random_entre(datos_compartidos->tiempo_min_cliente, datos_compartidos->tiempo_max_cliente)); //tiempo que se demora en producir el productor
        sem_wait(&datos_compartidos->puede_producir);          //productor usa un semaforo para saber si puede acceder a la seccion critica, que son las variables "correa" y "almacen"
        datos_compartidos->almacen[indice] = ++contador;        //productor accede y modifica el "almacen" (seccion critica)
        datos_compartidos->correa += 1;                         //productor accede y modifica a "correa" (seccion critica)
        printf("CAJA %i ; Indice almacen %i se produce %lg\n", datos_compartidos->id, indice, datos_compartidos->almacen[indice]);
        sem_post(&datos_compartidos->puede_consumir);       //productor envia señal semaforo para indicar que abandono seccion critica
    }
    sem_wait(&datos_compartidos->siguiente);    //se usa semaforo para saber si se han consumido todos los productos antes que acceda un nuevo productor (cliente)
  }
  return NULL;
}

//funcion hilo del cajero, esta se estara ejecutando 2 veces, una por cada caja
void* cajera(void* data) {
  datos_compartidos_t* datos_compartidos = (datos_compartidos_t*)data;
  for (int ronda = 0; ronda < datos_compartidos->cantidad_clientes; ++ronda) {      //se itera por la cantidad de clientes, siendo "ronda" el cliente en atencion
    printf("\t\t CAJA %i ; INICIO RONDA C: %i\n", datos_compartidos->id, ronda);
    for (int indice = 0; indice < datos_compartidos->productos; ++indice) {         //se itera por la cantidad de productos, siendo indice el producto que se pondra en almacen
      sem_wait(&datos_compartidos->puede_consumir);                                 //semaforo para saber si consumidor puede acceder a seccion critica
      float value = datos_compartidos->almacen[indice];                             //consumidor accede a "almacen" (seccion critica)
      usleep(1000 * random_entre(datos_compartidos->tiempo_min_caja, datos_compartidos->tiempo_max_caja)); //lo que se demora en consumir
      datos_compartidos->correa -= 1;                                           //consumidor accede y modifica a "correa" (seccion critica)
      printf("\t\tCAJA %i ; Indice almacen %i se consume %lg\n", datos_compartidos->id, indice, value);
      if(datos_compartidos->correa < datos_compartidos->capacidad_almacen){
        sem_post(&datos_compartidos->puede_producir);      //si es que el almacen no esta lleno, consumidor envia una señal semaforo a productor para que produzca mas items
      }
    }
    datos_compartidos->productos = random_entre(1,20);  //antes que pase un nuevo productor se genera una nueva cantidad de productos para el nuevo productor
    sem_post(&datos_compartidos->siguiente);           //se envia señal semaforo para indicar que pase el otro productor (cliente)
  }
  return NULL;
}

//devuelve un numero aleatorio entre dos valores "min" y "max"
int random_entre(int min, int max)
{
    int aux = 0;
    if (max > min)
        aux = random() % (max - min);
    return min + aux;
}
