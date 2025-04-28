#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 5
#define NCLIENTI 10

struct officina_t
{
    sem_t mutex;
    sem_t operaio[N];
    sem_t attesa_riparazione;
    sem_t attesa_ufficio;

    int b_ufficio, dentro_ufficio;
} o;

void init_officina(struct officina_t *officina)
{
    sem_init(&officina->mutex, 0, 1);
    sem_init(&officina->attesa_riparazione, 0, 0);
    sem_init(&officina->attesa_ufficio, 0, 0);

    for (int i = 0; i < N; i++)
    {
        sem_init(&officina->operaio[i], 0, 0);
    }
    officina->b_ufficio = officina->dentro_ufficio = 0;
}

void cliente_arrivo(struct officina_t *officina, int r)
{
    sem_wait(&officina->mutex);

    if (officina->dentro_ufficio == 0){
        // posso entrare e richiedere il servizio.
        printf("CLIENTE %ld: chiedo il servizio %d\n", pthread_self(), r);
        sem_post(&officina->attesa_ufficio);
        officina->dentro_ufficio++;
    }
    else{
        printf("CLIENTE %ld: aspetto fuori l'ufficio %d.\n", pthread_self(), r);
        officina->b_ufficio++;
    }

    sem_post(&officina->mutex);
    sem_wait(&officina->attesa_ufficio);

    // sveglio l'operaio che deve eseguire la riparazione.
    sem_post(&officina->operaio[r]);
}

void cliente_attesafineservizio(struct officina_t *officina)
{
    printf("CLIENTE %ld: aspetto la fine del servizio\n", pthread_self());
    sem_wait(&officina->mutex);
    printf("CLIENTE %ld: vado a casa\n", pthread_self());
    officina->dentro_ufficio--;
    if (officina->b_ufficio){
        officina->b_ufficio--;
        officina->dentro_ufficio++;
        sem_post(&officina->attesa_ufficio);
    }
    sem_post(&officina->mutex);
}

void operaio_attesacliente(struct officina_t *officina, int r)
{
    printf("Operaio %d: attendo un cliente\n", r);
    sem_wait(&officina->operaio[r]);
    printf("Operaio %d: chiesto il mio tipo di riparazione.\n", r);
}

void operaio_fineservizio(struct officina_t *officina)
{
    printf("Operaio %ld: servizio terminato.\n", pthread_self());
    sem_post(&officina->attesa_riparazione);
}

void *cliente(void *arg) {
    int r = rand() % N;
    cliente_arrivo(&o, r);
    cliente_attesafineservizio(&o);
    return 0;
}

void *operaio(void *arg) {
    int r = (int)arg;
    for(;;) {
        operaio_attesacliente(&o, r);
        sleep(1); // Simula il tempo per eseguire il servizio
        operaio_fineservizio(&o);
    }
    return 0;
}

int main() {
    pthread_attr_t a;
    pthread_t p;

    init_officina(&o);
    srand(555);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    // Creazione operai (uno per ogni tipo di servizio)
    for (int i = 0; i < N; i++) {
        pthread_create(&p, &a, operaio, (void*)i);
    }

    // Creazione clienti
    for (int i = 0; i < NCLIENTI; i++) {
        pthread_create(&p, &a, cliente, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(5);
    printf("Chiusura!\n");

    return 0;
}