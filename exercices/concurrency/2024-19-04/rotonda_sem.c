#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 20    // Numero di auto
#define S 6     // Numero di sezioni della rotonda

// Struttura per rappresentare la rotonda
typedef struct {
    sem_t mutex[S];       // Semafori per ciascuna sezione (mutex)
    sem_t condition[S];   // Semafori per l'attesa su ciascuna sezione
    int sezioni[S];       // 0 = sezione libera, 1 = sezione occupata
    int posizioni_auto[N]; // Posizione corrente di ciascuna auto
} rotonda_t;

rotonda_t r;

// Funzione di inizializzazione della rotonda
void init_rotonda(rotonda_t *r) {
    for (int i = 0; i < S; i++) {
        sem_init(&r->mutex[i], 0, 1);      // Inizializza i mutex per ogni sezione
        sem_init(&r->condition[i], 0, 0);  // Inizializza i semafori di attesa per ogni sezione
        r->sezioni[i] = 0;                 // Tutte le sezioni sono libere
    }
    for (int i = 0; i < N; i++) {
        r->posizioni_auto[i] = -1;         // Posizione iniziale non definita
    }
}

// Funzione per entrare nella rotonda
void entra(rotonda_t *r, int numeroauto, int sezione) {
    sem_wait(&r->mutex[sezione]);          // Acquisisci il mutex per la sezione

    // Attende finché la sezione è occupata
    while (r->sezioni[sezione]) {
        sem_post(&r->mutex[sezione]);      // Rilascia il mutex prima di bloccarsi
        sem_wait(&r->condition[sezione]);  // Blocca l'auto sulla sezione
        sem_wait(&r->mutex[sezione]);      // Ri-acquisisci il mutex dopo essere stato risvegliato
    }

    r->sezioni[sezione] = 1;               // Imposta la sezione come occupata (1)
    r->posizioni_auto[numeroauto] = sezione; // Aggiorna la posizione dell'auto nell'array posizioni_auto

    sem_post(&r->mutex[sezione]);          // Rilascia il mutex
}

// Funzione per verificare se l'auto ha raggiunto la destinazione
int sonoarrivato(rotonda_t *r, int numeroauto, int destinazione) {
    int sezionecorrente = r->posizioni_auto[numeroauto];

    sem_wait(&r->mutex[sezionecorrente]);  // Acquisisci il mutex per la sezione corrente
    printf("Sono %d e devo uscire alla sezione %d. Sono arrivato?\n", numeroauto, destinazione);

    // Verifica se la sezione corrente è la destinazione dell'auto
    if (sezionecorrente == destinazione) {
        printf("SI! Sono %d e sono arrivato alla sezione %d. Esco...\n", numeroauto, destinazione);
        sem_post(&r->mutex[sezionecorrente]); // Rilascia il mutex
        return 0; // L'auto è arrivata alla destinazione
    }

    // Non è arrivato
    // Quindi va avanti e rilascia la sezione corrente
    r->sezioni[sezionecorrente] = 0;
    int nuovasezione = (sezionecorrente + 1) % S;
    sem_post(&r->condition[sezionecorrente]); // Segnala che la sezione è libera
    sem_post(&r->mutex[sezionecorrente]);    // Rilascia il mutex per la sezione corrente

    // Entra nella nuova sezione
    sem_wait(&r->mutex[nuovasezione]);       // Acquisisci il mutex per la nuova sezione

    while (r->sezioni[nuovasezione]) {
        sem_post(&r->mutex[nuovasezione]);   // Rilascia il mutex prima di bloccarsi
        sem_wait(&r->condition[nuovasezione]); // Blocca l'auto sulla nuova sezione
        sem_wait(&r->mutex[nuovasezione]);   // Ri-acquisisci il mutex dopo essere stato risvegliato
    }

    r->sezioni[nuovasezione] = 1;           // Occupa la nuova sezione
    r->posizioni_auto[numeroauto] = nuovasezione;
    printf("NO! Sono %d ed avanzo alla sezione %d.\n", numeroauto, nuovasezione);

    sem_post(&r->mutex[nuovasezione]);      // Rilascia il mutex per la nuova sezione

    return 1; // L'auto non è ancora arrivata
}

// Funzione per uscire dalla rotonda
void esci(rotonda_t *r, int numeroauto) {
    int sezionecorrente = r->posizioni_auto[numeroauto];
    sem_wait(&r->mutex[sezionecorrente]);   // Acquisisci il mutex per la sezione corrente

    // Libera la sezione
    printf("----------Auto %d è uscita dalla sezione %d----------\n", numeroauto, sezionecorrente);
    r->sezioni[sezionecorrente] = 0;
    sem_post(&r->condition[sezionecorrente]); // Segnala che la sezione è libera
    sem_post(&r->mutex[sezionecorrente]);    // Rilascia il mutex per la sezione corrente
}

void pausetta(int quanto) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 100 + 1) * 1000000 + quanto;
    nanosleep(&t, NULL);
}

// Funzione per simulare il comportamento di un'auto
void *auto_thread(void *arg) {
    int numeroauto = *((int *)arg);
    int sezionediingresso = rand() % S;  // Sezione di ingresso casuale
    int destinazione = (sezionediingresso + rand()) % S; // Destinazione casuale

    // Entra nella rotonda
    entra(&r, numeroauto, sezionediingresso);
    printf("Auto %d, da %d a %d.\n", numeroauto, sezionediingresso, destinazione);
    do {
        // Simula il passaggio nella sezione corrente
        pausetta(1000000);
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
    srand(100);

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