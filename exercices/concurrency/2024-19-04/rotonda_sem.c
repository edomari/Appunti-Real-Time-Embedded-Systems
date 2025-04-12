#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 7  // Numero di auto
#define S 6     // Numero di sezioni della rotonda

// Struttura per rappresentare la rotonda
typedef struct {
    sem_t mutex[S];           // Mutex globale per proteggere le sezioni
    sem_t condition[S];         // Semafori privati per ogni auto
    int sezioni[S];        // 0 = libera, 1 = occupata
    int posizioni_auto[N]; // Posizione corrente di ogni auto (-1 se non in rotonda)
} rotonda_t;

rotonda_t r;

// Funzione di inizializzazione della rotonda
void init_rotonda(rotonda_t *r) {
    for (int i = 0; i < N; i++) {
        sem_init(&r->mutex[i], 0, 1);
        sem_init(&r->condition[i], 0, 0);  // Semafori privati inizializzati a 0
        r->posizioni_auto[i] = -1;
    }
    for (int i = 0; i < S; i++) {
        r->sezioni[i] = 0;  // Tutte le sezioni inizialmente libere
    }
}

// Funzione per entrare nella rotonda
void entra(rotonda_t *r, int numeroauto, int sezione) {
    sem_wait(&r->mutex[sezione]);  // Acquisisco il mutex della sezione
    
    if (r->sezioni[sezione] == 1) {
        sem_post(&r->mutex[sezione]);
        printf("Sono %d e aspetto che la sezione %d sia libera.\n", numeroauto, sezione);
        sem_wait(&r->condition[sezione]);
    } 

    r->sezioni[sezione] = 1; // la sezione ora è occupata
    r->posizioni_auto[numeroauto] = sezione;
    printf("Sono %d ed entro nella sezione %d.\n", numeroauto, sezione);
    sem_post(&r->mutex[sezione]);
}

// Funzione per verificare se l'auto ha raggiunto la destinazione
int sonoarrivato(rotonda_t *r, int numeroauto, int destinazione) {
    int sezionecorrente = r->posizioni_auto[numeroauto];
    
    sem_wait(&r->mutex[sezionecorrente]);  // Blocco il mutex
    
    printf("Sono %d, sono in %d e devo uscire alla sezione %d. Sono arrivato?\n", numeroauto, sezionecorrente, destinazione);
    
    // Se è arrivato a destinazione, esce
    if (sezionecorrente == destinazione) {
        printf("SI! Sono %d e sono arrivato alla sezione %d. Esco...\n", numeroauto, destinazione);
        sem_post(&r->mutex[sezionecorrente]);  // Rilascio il mutex (l'uscita vera è gestita da `esci`)
        return 0;
    }
    // Se NON è arrivato, rilascia la sezione corrente e avanza
    int nuovasezione = (sezionecorrente + 1) % S;
    r->sezioni[sezionecorrente] = 0;
    sem_post(&r->condition[sezionecorrente]);
    sem_post(&r->mutex[sezionecorrente]); 

    sem_wait(&r->mutex[nuovasezione]); 
    printf("NO! Sono %d e chiedo di avanzare alla sezione %d.\n", numeroauto, nuovasezione); 
    if(r->sezioni[nuovasezione] == 1){
        printf("Sono %d e aspetto che la sezione %d si liberi.\n", numeroauto, nuovasezione);
        sem_wait(&r->condition[nuovasezione]);
    }
    printf("Sono %d e ho il permesso avanzare alla sezione %d.\n", numeroauto, nuovasezione); 
    r->sezioni[nuovasezione] = 1; // Occupa la nuova sezione
    r->posizioni_auto[numeroauto] = nuovasezione;
    sem_post(&r->mutex[nuovasezione]);
    return 1;     
    
}

// Funzione per uscire dalla rotonda
void esci(rotonda_t *r, int numeroauto) {
    sem_wait(&r->mutex[r->posizioni_auto[numeroauto]]);  // Acquisisco il mutex
    
    // Libero la sezione
    printf("----------Auto %d è uscita dalla sezione %d----------\n", numeroauto, r->posizioni_auto[numeroauto]);
    r->sezioni[r->posizioni_auto[numeroauto]] = 0;
    sem_post(&r->condition[r->posizioni_auto[numeroauto]]);
    sem_post(&r->mutex[r->posizioni_auto[numeroauto]]);
}

void pausetta(int quanto) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 100 + 1) * 100 + quanto;
    nanosleep(&t, NULL);
}

// Funzione per simulare il comportamento di un'auto
void *auto_thread(void *arg) {
    int numeroauto = *((int *)arg);
    int sezionediingresso = rand() % S;  // Sezione di ingresso casuale
    int destinazione = (sezionediingresso + rand()) % S; // Destinazione casuale
    printf("Auto %d, da %d a %d.\n", numeroauto, sezionediingresso, destinazione);

    // Entra nella rotonda
    entra(&r, numeroauto, sezionediingresso);
    do {
        // Simula il passaggio nella sezione corrente
        pausetta(10);
    } while (sonoarrivato(&r, numeroauto, destinazione));

    // Esce dalla rotonda
    esci(&r, numeroauto);

    return NULL;
}

int main() {
    pthread_t threads[N];
    int auto_id[N];
    pthread_attr_t p_attr;

    pthread_attr_init(&p_attr);
    srand(1000);

    // Inizializza la rotonda
    init_rotonda(&r);

    // Crea i thread per ogni auto
    for (int i = 0; i < N; i++) {
        auto_id[i] = i;
        pthread_create(&threads[i], &p_attr, auto_thread, &auto_id[i]);
    }

    // Unisce i thread
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&p_attr);
    return 0;
}