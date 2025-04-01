#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 20    // Numero di auto
#define S 6     // Numero di sezioni della rotonda

// Struttura per rappresentare la rotonda
typedef struct {
    pthread_mutex_t mutex[S];    // Mutex per ciascuna sezione
    pthread_cond_t condition[S]; // Condition variable per ciascuna sezione
    int sezioni[S];              // 0 = sezione libera, 1 = sezione occupata
    int posizioni_auto[N];       // Posizione corrente di ciascuna auto
} rotonda_t;

rotonda_t r;

// Funzione di inizializzazione della rotonda
void init_rotonda(rotonda_t *r) {
    for (int i = 0; i < S; i++) {
        pthread_mutex_init(&r->mutex[i], NULL);
        pthread_cond_init(&r->condition[i], NULL);
        r->sezioni[i] = 0; // tutte le sezioni sono libere
    }
    for (int i = 0; i < N; i++) {
        r->posizioni_auto[i] = -1; // Posizione iniziale non definita
    }
}

// Funzione per entrare nella rotonda
void entra(rotonda_t *r, int numeroauto, int sezione) {
    pthread_mutex_lock(&r->mutex[sezione]);

    // Aspetta che la sezione sia libera
    while (r->sezioni[sezione]) {
        pthread_cond_wait(&r->condition[sezione], &r->mutex[sezione]);
    }

    // Occupa la sezione
    r->sezioni[sezione] = 1;
    r->posizioni_auto[numeroauto] = sezione; // Aggiorna la posizione dell'auto
    //printf("Sono %d ed entro nella sezione %d\n", numeroauto, sezione);

    pthread_mutex_unlock(&r->mutex[sezione]);
}

// Funzione per verificare se l'auto ha raggiunto la destinazione
int sonoarrivato(rotonda_t *r, int numeroauto, int destinazione) {
    int sezionecorrente = r->posizioni_auto[numeroauto];

    pthread_mutex_lock(&r->mutex[sezionecorrente]);
    printf("Sono %d e devo uscire alla sezione %d. Sono arrivato?\n", numeroauto, destinazione);

    // Verifica se la sezione corrente è la destinazione dell'auto
    if (sezionecorrente == destinazione) {
        printf("SI! Sono %d e sono arrivato alla sezione %d. Esco...\n", numeroauto, destinazione);
        pthread_mutex_unlock(&r->mutex[sezionecorrente]);
        return 0; // L'auto è arrivata alla destinazione
    }

    // Non è arrivato, quindi rilascia la sezione corrente
    r->sezioni[sezionecorrente] = 0;
    pthread_cond_signal(&r->condition[sezionecorrente]);  // Segnala che la sezione è libera
    pthread_mutex_unlock(&r->mutex[sezionecorrente]);

    // Sposta l'auto alla sezione successiva
    int nuovasezione = (sezionecorrente + 1) % S;
    
    // Entra nella nuova sezione
    pthread_mutex_lock(&r->mutex[nuovasezione]);
    
    while (r->sezioni[nuovasezione]) {
        pthread_cond_wait(&r->condition[nuovasezione], &r->mutex[nuovasezione]);
    }
    
    r->sezioni[nuovasezione] = 1; // Occupa la nuova sezione
    r->posizioni_auto[numeroauto] = nuovasezione;
    printf("NO! Sono %d ed avanzo alla sezione %d.\n", numeroauto, nuovasezione);

    pthread_mutex_unlock(&r->mutex[nuovasezione]);

    return 1; // L'auto non è ancora arrivata
}


// Funzione per uscire dalla rotonda
void esci(rotonda_t *r, int numeroauto) {
    pthread_mutex_lock(&r->mutex[r->posizioni_auto[numeroauto]]);

    // Libera la sezione
    printf("----------Auto %d è uscita dalla sezione %d----------\n", numeroauto, r->posizioni_auto[numeroauto]);
    r->sezioni[r->posizioni_auto[numeroauto]] = 0;
    // Segnala che la sezione è libera
    pthread_cond_signal(&r->condition[r->posizioni_auto[numeroauto]]);
    pthread_mutex_unlock(&r->mutex[r->posizioni_auto[numeroauto]]);
}

void pausetta(int quanto)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%100+1)*1000000 + quanto;
  nanosleep(&t,NULL);
}

// Funzione per simulare il comportamento di un'auto
void *auto_thread(void *arg) {
    int numeroauto = *((int *)arg);
    int sezionediingresso = rand()%S;  // Sezione di ingresso casuale
    int destinazione = (sezionediingresso + rand())%S;       // Destinazione casuale

    // Entra nella rotondas
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
