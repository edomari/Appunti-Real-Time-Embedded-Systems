#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 20    // Numero massimo barche nel porto
#define BARCHE 200 // Numero totale di barche

struct porto_t {
    pthread_mutex_t mutex;    
    pthread_cond_t cond;      // Condition variable per gestire l'accesso
    int numero_barche_porto;  // Numero di barche attualmente nel porto
    int numero_barche_passaggio;  // Numero di barche attualmente nel passaggio
    int portopieno;           // Indica se il porto è pieno
    int passaggiopieno;       // Indica se il passaggio è pieno
} porto;

// Inizializza la struttura del porto
void init_porto(struct porto_t *porto) {
    pthread_mutexattr_t m_attr;
	pthread_condattr_t c_attr;
	pthread_mutexattr_init(&m_attr);
	pthread_condattr_init(&c_attr);

    pthread_mutex_init(&porto->mutex, NULL);
    pthread_cond_init(&porto->cond, NULL);
    porto->numero_barche_porto = 0;
    porto->numero_barche_passaggio = 0;
    porto->portopieno = 0;
    porto->passaggiopieno = 0;
}

// Simula una piccola pausa
void pausetta() {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 100 + 1) * 1000000;
    nanosleep(&t, NULL);
}

// Gestisce la richiesta di entrata nel porto
void entrata_richiesta(struct porto_t *porto, int numeroauto) {
    pthread_mutex_lock(&porto->mutex);
    printf("%d -> Richiesta di entrare nel porto\n", numeroauto);

    // Aspetta che ci sia spazio nel passaggio e nel porto
    while (porto->portopieno || porto->passaggiopieno) {
        pthread_cond_wait(&porto->cond, &porto->mutex);
    }

    porto->numero_barche_passaggio++;
    
    printf("%d -> Barca nel passaggio. Numero di barche nel passaggio: %d\n", numeroauto, porto->numero_barche_passaggio);

    if (porto->numero_barche_passaggio == 2) {
        porto->passaggiopieno = 1;
        printf("%d -> Il passaggio è pieno\n", numeroauto);
    }

    pthread_mutex_unlock(&porto->mutex);
}

// Conferma l'entrata nel porto
void entrata_ok(struct porto_t *porto, int numeroauto) {
    pthread_mutex_lock(&porto->mutex);
    porto->numero_barche_porto++;
    porto->numero_barche_passaggio--;
    printf("%d -> Barca entrata nel porto. Numero di barche nel porto: %d\n", numeroauto, porto->numero_barche_porto);

    if (porto->numero_barche_porto == N) {
        porto->portopieno = 1;
        printf("%d -> Il porto è pieno\n", numeroauto);
    }

    if (porto->numero_barche_passaggio < 2) {
        porto->passaggiopieno = 0;
        pthread_cond_signal(&porto->cond);  // Libera altre barche in attesa
    }

    pthread_mutex_unlock(&porto->mutex);
}

// Gestisce la richiesta di uscita dal porto
void uscita_richiesta(struct porto_t *porto, int numeroauto) {
    pthread_mutex_lock(&porto->mutex);
    printf("%d -> Richiesta di uscire dal porto\n", numeroauto);

    // Aspetta che ci sia spazio nel passaggio
    while (porto->passaggiopieno) {
        pthread_cond_wait(&porto->cond, &porto->mutex);
    }

    porto->numero_barche_passaggio++;
    printf("%d -> Barca nel passaggio per uscire. Numero di barche nel passaggio: %d\n", numeroauto, porto->numero_barche_passaggio);

    if (porto->numero_barche_passaggio == 2) {
        porto->passaggiopieno = 1;
        printf("%d -> Il passaggio è pieno per uscita\n", numeroauto);
    }

    pthread_mutex_unlock(&porto->mutex);
}

// Conferma l'uscita dal porto
void uscita_ok(struct porto_t *porto, int numeroauto) {
    pthread_mutex_lock(&porto->mutex);

    porto->numero_barche_porto--;
    porto->numero_barche_passaggio--;
    printf("%d -> Barca uscita dal porto. Numero di barche nel porto: %d\n", numeroauto, porto->numero_barche_porto);

    if (porto->numero_barche_porto < N) {
        porto->portopieno = 0;
        pthread_cond_signal(&porto->cond);  // Libera altre barche in attesa
    }

    if (porto->numero_barche_passaggio < 2) {
        porto->passaggiopieno = 0;
        pthread_cond_signal(&porto->cond);  // Libera altre barche in attesa
    }

    pthread_mutex_unlock(&porto->mutex);
}

// Funzione per simulare il comportamento di una barca
void *barca(void *arg) {
    int numeroauto = *((int *)arg);
    pausetta();
    entrata_richiesta(&porto, numeroauto);
    pausetta();
    entrata_ok(&porto, numeroauto);
    pausetta();
    uscita_richiesta(&porto, numeroauto);
    pausetta();
    uscita_ok(&porto, numeroauto);
    pausetta();
    return NULL;
}

int main() {
    pthread_t threads[BARCHE];
    pthread_attr_t p_attr;
    int barca_id[BARCHE];
    
    pthread_attr_init(&p_attr);
    srand(100);

    // Inizializza il porto
    init_porto(&porto);

    // Crea i thread per ogni barca
    for (int i = 0; i < BARCHE; i++) {
        barca_id[i] = i;
        pthread_create(&threads[i], &p_attr, barca, &barca_id[i]);
    }

    // Unisce i thread
    for (int i = 0; i < BARCHE; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&p_attr);

    sleep(1);

    return 0;
}