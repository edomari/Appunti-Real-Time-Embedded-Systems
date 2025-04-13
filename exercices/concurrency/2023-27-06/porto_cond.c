#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5          // Numero massimo barche nel porto
#define BARCHE 10    // Numero totale di barche

struct porto_t {
    pthread_mutex_t mutex;    
    pthread_cond_t cond_entrata;      // Condition variable per barche in entrata
    pthread_cond_t cond_uscita;      // Condition variable per barche in uscita
    int numero_barche_porto;         // Numero di barche attualmente nel porto
    int numero_barche_passaggio;     // Numero di barche attualmente nel passaggio
    int barche_in_attesa_uscita;     // Numero di barche in attesa di uscire
    int portopieno;                  // Indica se il porto è pieno
    int passaggiopieno;              // Indica se il passaggio è pieno
} porto;

void init_porto(struct porto_t *porto) {
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&porto->mutex, NULL);
    pthread_cond_init(&porto->cond_entrata, NULL);
    pthread_cond_init(&porto->cond_uscita, NULL);
    porto->numero_barche_porto = 0;
    porto->numero_barche_passaggio = 0;
    porto->barche_in_attesa_uscita = 0;
    porto->portopieno = 0;
    porto->passaggiopieno = 0;
}

void pausetta() {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 100 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void entrata_richiesta(struct porto_t *porto) {
    pthread_mutex_lock(&porto->mutex);
    printf("%ld -> Richiesta di entrare nel porto\n", pthread_self());

    // Aspetta che:
    // 1. Ci sia spazio nel passaggio
    // 2. Non ci siano barche in attesa di uscire
    // 3. Ci sia spazio nel porto
    while (porto->passaggiopieno || porto->barche_in_attesa_uscita > 0 || porto->portopieno) {
        pthread_cond_wait(&porto->cond_entrata, &porto->mutex);
    }

    porto->numero_barche_passaggio++;
    
    printf("%ld -> Barca nel passaggio (entrata). Barche nel passaggio: %d\n", pthread_self(), porto->numero_barche_passaggio);

    if (porto->numero_barche_passaggio == 2) {
        porto->passaggiopieno = 1;
        printf("%ld -> Il passaggio è pieno\n", pthread_self());
    }

    pthread_mutex_unlock(&porto->mutex);
}

void entrata_ok(struct porto_t *porto) {
    pthread_mutex_lock(&porto->mutex);
    porto->numero_barche_porto++;
    porto->numero_barche_passaggio--;
    
    printf("%ld -> Barca entrata nel porto. Barche nel porto: %d\n", pthread_self(), porto->numero_barche_porto);

    if (porto->numero_barche_porto == N) {
        porto->portopieno = 1;
        printf("%ld -> Il porto è pieno\n", pthread_self());
    }

    // Se il passaggio non è più pieno, sveglia le barche in uscita prima
    if (porto->numero_barche_passaggio < 2) {
        porto->passaggiopieno = 0;
        if (porto->barche_in_attesa_uscita > 0) {
            pthread_cond_signal(&porto->cond_uscita);
        } else {
            pthread_cond_signal(&porto->cond_entrata);
        }
    }

    pthread_mutex_unlock(&porto->mutex);
}

void uscita_richiesta(struct porto_t *porto) {
    pthread_mutex_lock(&porto->mutex);
    porto->barche_in_attesa_uscita++;
    printf("%ld -> Richiesta di uscire dal porto (in attesa: %d)\n", pthread_self(), porto->barche_in_attesa_uscita);

    // Aspetta che ci sia spazio nel passaggio
    while (porto->passaggiopieno) {
        pthread_cond_wait(&porto->cond_uscita, &porto->mutex);
    }

    porto->barche_in_attesa_uscita--;
    porto->numero_barche_passaggio++;
    porto->numero_barche_porto--;
    
    printf("%ld -> Barca nel passaggio (uscita). Barche nel passaggio: %d\n", pthread_self(), porto->numero_barche_passaggio);

    if (porto->numero_barche_passaggio == 2) {
        porto->passaggiopieno = 1;
        printf("%ld -> Il passaggio è pieno\n", pthread_self());
    }

    pthread_mutex_unlock(&porto->mutex);
}

void uscita_ok(struct porto_t *porto) {
    pthread_mutex_lock(&porto->mutex);

    porto->numero_barche_passaggio--;
    printf("%ld -> Barca uscita dal porto. Barche nel porto: %d\n", pthread_self(), porto->numero_barche_porto);

    // Il porto non è più pieno
    if (porto->numero_barche_porto < N) {
        porto->portopieno = 0;
    }

    // Se il passaggio non è più pieno, da priorità alle uscite
    if (porto->numero_barche_passaggio < 2) {
        porto->passaggiopieno = 0;
        if (porto->barche_in_attesa_uscita > 0) {
            pthread_cond_signal(&porto->cond_uscita);
        } else {
            pthread_cond_signal(&porto->cond_entrata);
        }
    }

    pthread_mutex_unlock(&porto->mutex);
}

void *barca(void *arg) {
    pausetta();
    entrata_richiesta(&porto);
    pausetta();
    entrata_ok(&porto);
    pausetta();
    uscita_richiesta(&porto);
    pausetta();
    uscita_ok(&porto);
    pausetta();
    return NULL;
}

int main() {
    int i=0;
    pthread_attr_t a;
    pthread_t pa;

    init_porto(&porto);

    srand(55);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i = 0; i < BARCHE; i++) {
        pthread_create(&pa, &a, barca, (void *)i);
    }
    pthread_attr_destroy(&a);

    sleep(1);

    return 0;
}