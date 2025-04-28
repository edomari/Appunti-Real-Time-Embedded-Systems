#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define N 5
#define NCLIENTI 10

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
        printf("CLIENTE %ld: aspetto fuori l'ufficio %d.\n", pthread_self(), r);
        pthread_cond_wait(&officina->attesa_ufficio, &officina->mutex);
        officina->n_clienti_attesa--;
    }
    
    officina->dentro_ufficio = 1; // Occupa l'ufficio
    officina->servizio_richiesto = r; // Imposta il servizio richiesto

    printf("CLIENTE %ld: chiedo il servizio %d\n", pthread_self(), r);
    pthread_cond_signal(&officina->operaio[r]); // Notifica l'operaio specifico
    pthread_mutex_unlock(&officina->mutex);
}

void cliente_attesafineservizio(struct officina_t *officina) {
    pthread_mutex_lock(&officina->mutex);
    printf("CLIENTE %ld: aspetto la fine del servizio\n", pthread_self());
    pthread_cond_wait(&officina->attesa_riparazione, &officina->mutex);
    printf("CLIENTE %ld: vado a casa\n", pthread_self());
    
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
        printf("Operaio %d: attendo un cliente\n", r);
        pthread_cond_wait(&officina->operaio[r], &officina->mutex);
    }
    
    printf("Operaio %d: chiesto il mio tipo di riparazione.\n", r);
    pthread_mutex_unlock(&officina->mutex);
}

void operaio_fineservizio(struct officina_t *officina) {
    pthread_mutex_lock(&officina->mutex);
    printf("Operaio %ld: servizio terminato.\n", pthread_self());
    officina->servizio_richiesto = -1; // Resetta il servizio richiesto
    pthread_cond_signal(&officina->attesa_riparazione);
    pthread_mutex_unlock(&officina->mutex);
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
    sleep(50);
    printf("Chiusura!\n");

    return 0;
}