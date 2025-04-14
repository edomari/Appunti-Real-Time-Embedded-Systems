#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 8
#define NCLIENTI 20

struct officina_t {
    pthread_mutex_t mutex;
    pthread_cond_t attesa_ufficio;
    pthread_cond_t attesa_riparazione;
    pthread_cond_t operaio[N];

    int n_clienti_attesa;
    int dentro_ufficio;
    int servizio_richiesto;
} o;

void init_officina(struct officina_t *officina) {
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);

    pthread_mutex_init(&officina->mutex, &mutex_attr);
    pthread_cond_init(&officina->attesa_riparazione, &cond_attr);
    pthread_cond_init(&officina->attesa_ufficio, &cond_attr);

    for (int i = 0; i < N; i++) {
        pthread_cond_init(&officina->operaio[i], &cond_attr);
    }

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    officina->n_clienti_attesa = 0;
    officina->dentro_ufficio = 0; // 0 = libero, 1 = occupato
    officina->servizio_richiesto = -1; // nessun servizio richiesto inizialmente
}

void cliente_arrivo(struct officina_t *officina, int r) {
    pthread_mutex_lock(&officina->mutex);

    // Attende se l'ufficio è occupato
    while (officina->dentro_ufficio) {
        officina->n_clienti_attesa++;
        printf("%ld > Aspetto fuori per il servizio %d\n", (long)pthread_self(), r);
        pthread_cond_wait(&officina->attesa_ufficio, &officina->mutex);
        officina->n_clienti_attesa--;
    }
    
    officina->dentro_ufficio = 1; // Occupa l'ufficio
    officina->servizio_richiesto = r; // Imposta il servizio richiesto

    printf("%ld > Chiedo il servizio %d\n", (long)pthread_self(), r);
    pthread_cond_signal(&officina->operaio[r]); // Notifica l'operaio specifico
    pthread_mutex_unlock(&officina->mutex);
}

void cliente_attesafineservizio(struct officina_t *officina) {
    pthread_mutex_lock(&officina->mutex);
    printf("%ld > in attesa della fine del servizio\n", (long)pthread_self());
    pthread_cond_wait(&officina->attesa_riparazione, &officina->mutex);
    printf("%ld > Servizio finito... Vado a casa\n", (long)pthread_self());
    
    officina->dentro_ufficio = 0; // Libera l'ufficio

    if (officina->n_clienti_attesa > 0) {
        pthread_cond_signal(&officina->attesa_ufficio); // Sblocca un cliente in attesa
    }
    pthread_mutex_unlock(&officina->mutex);
}

void operaio_attesacliente(struct officina_t *officina, int r) {
    pthread_mutex_lock(&officina->mutex);
    
    // Attende finché non arriva un cliente che richiede il suo servizio
    while (officina->servizio_richiesto != r) {
        printf("Operaio %d in attesa di un cliente\n", r);
        pthread_cond_wait(&officina->operaio[r], &officina->mutex);
    }
    
    printf("L'operaio %d > Qualcuno ha chiesto la riparazione %d che svolgo solo io\n", r, r);
    pthread_mutex_unlock(&officina->mutex);
}

void operaio_fineservizio(struct officina_t *officina) {
    pthread_mutex_lock(&officina->mutex);
    printf("L'operaio %ld > Ho terminato il servizio richiesto, avviso il cliente\n", (long)pthread_self());
    officina->servizio_richiesto = -1; // Resetta il servizio richiesto
    pthread_cond_signal(&officina->attesa_riparazione);
    pthread_mutex_unlock(&officina->mutex);
}

void *cliente(void *arg) {
    int r = rand() % N;
    cliente_arrivo(&o, r);
    cliente_attesafineservizio(&o);
}

void *operaio(void *arg) {
    int r = (int)(intptr_t)arg;
    for(;;) {
        operaio_attesacliente(&o, r);
        sleep(1); // Simula il tempo per eseguire il servizio
        operaio_fineservizio(&o);
    }
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
        pthread_create(&p, &a, operaio, (void*)(intptr_t)i);
    }

    // Creazione clienti
    for (int i = 0; i < NCLIENTI; i++) {
        pthread_create(&p, &a, cliente, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(10);
    printf("Chiusura!\n");

    return 0;
}