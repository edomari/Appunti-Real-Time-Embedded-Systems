#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5  // Numero di tipologie di attrezzi
#define M 3  // Numero di copie di ciascun attrezzo
#define P 10 // Numero di persone
#define E 10 // Numero di esercizi per persona

typedef struct {
    int attrezzi_liberi[N]; // Numero di attrezzi liberi per ciascun tipo
    int attrezzi_prenotati[N]; // Numero di attrezzi prenotati per ciascun tipo
    pthread_mutex_t mutex[N];  // Mutex per ciascun tipo di attrezzo
    pthread_cond_t cond[N];    // Condition variable per ciascun tipo di attrezzo
} palestra_t ;

palestra_t palestra;

void init_palestra(palestra_t *p) {
    for (int i = 0; i < N; i++) {
        p->attrezzi_liberi[i] = M;      // Tutti gli attrezzi sono inizialmente liberi
        p->attrezzi_prenotati[i] = 0;   // Nessun attrezzo è inizialmente prenotato
        pthread_mutex_init(&p->mutex[i], NULL);
        pthread_cond_init(&p->cond[i], NULL);
    }
}

void usaattrezzo(palestra_t *p, int numeropersona, int tipoattrezzo) {
    pthread_mutex_lock(&p->mutex[tipoattrezzo]);

    while (p->attrezzi_liberi[tipoattrezzo] == 0 && p->attrezzi_prenotati[tipoattrezzo] == 0) {
        // Aspetta che un attrezzo del tipo richiesto sia disponibile
        pthread_cond_wait(&p->cond[tipoattrezzo], &p->mutex[tipoattrezzo]);
    }

    if (p->attrezzi_prenotati[tipoattrezzo] > 0) {
        // Se c'è un attrezzo prenotato dalla persona
        p->attrezzi_prenotati[tipoattrezzo]--;
    } else {
        // Altrimenti usa un attrezzo libero
        p->attrezzi_liberi[tipoattrezzo]--;
    }

    printf("Persona %d sta usando attrezzo tipo %d\n", numeropersona, tipoattrezzo);

    pthread_mutex_unlock(&p->mutex[tipoattrezzo]);
}

void prenota(palestra_t *p, int numeropersona, int tipoattrezzo) {
    pthread_mutex_lock(&p->mutex[tipoattrezzo]);

    if (p->attrezzi_liberi[tipoattrezzo] > 0) {
        // Se c'è un attrezzo libero, prenotalo
        p->attrezzi_liberi[tipoattrezzo]--;
        p->attrezzi_prenotati[tipoattrezzo]++;
        printf("Persona %d ha prenotato attrezzo tipo %d\n", numeropersona, tipoattrezzo);
    } else {
        printf("Persona %d non ha potuto prenotare attrezzo tipo %d\n", numeropersona, tipoattrezzo);
    }

    pthread_mutex_unlock(&p->mutex[tipoattrezzo]);
}

void fineuso(palestra_t *p, int numeropersona, int tipoattrezzo) {
    pthread_mutex_lock(&p->mutex[tipoattrezzo]);

    // Rilascia l'attrezzo e segnala agli altri thread che è libero
    p->attrezzi_liberi[tipoattrezzo]++;
    printf("Persona %d ha finito di usare attrezzo tipo %d\n", numeropersona, tipoattrezzo);

    pthread_cond_signal(&p->cond[tipoattrezzo]); // Segnala che un attrezzo si è liberato

    pthread_mutex_unlock(&p->mutex[tipoattrezzo]);
}

void *persona(void *arg) {
    int numeropersona = *(int *)arg;
    int attrezzocorrente = rand() % N;
    int prossimoattrezzo = rand() % N;

    for (int i = 0; i < E; i++) {
        usaattrezzo(&palestra, numeropersona, attrezzocorrente);

        if (i < E - 1) {
            prenota(&palestra, numeropersona, prossimoattrezzo);
        }

        fineuso(&palestra, numeropersona, attrezzocorrente);

        if (i < E - 1) {
            attrezzocorrente = prossimoattrezzo;
            prossimoattrezzo = rand() % N;
        }
    }

    return NULL;
}


int main() {
    pthread_t persone[P];
    int numeri[P];
    srand(time(NULL));

    // Inizializza la palestra
    init_palestra(&palestra);

    // Crea i thread per le persone
    for (int i = 0; i < P; i++) {
        numeri[i] = i;
        pthread_create(&persone[i], NULL, persona, &numeri[i]);
    }

    // Attende la terminazione di tutti i thread
    for (int i = 0; i < P; i++) {
        pthread_join(persone[i], NULL);
    }

    return 0;
}
