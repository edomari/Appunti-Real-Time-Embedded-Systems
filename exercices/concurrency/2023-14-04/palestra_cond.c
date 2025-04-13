#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5  // Numero di tipologie di attrezzi
#define M 3  // Numero di copie di ciascun attrezzo
#define P 10 // Numero di persone
#define E 10 // Numero di esercizi per persona

struct palestra_t{
    int attrezzi_liberi[N]; // Numero di attrezzi liberi per ciascun tipo
    int attrezzi_prenotati[N]; // Numero di attrezzi prenotati per ciascun tipo
    pthread_mutex_t mutex[N];  // Mutex per ciascun tipo di attrezzo
    pthread_cond_t cond[N];    // Condition variable per ciascun tipo di attrezzo
} palestra;

void init_palestra(struct palestra_t *p) {
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    for (int i = 0; i < N; i++) {
        p->attrezzi_liberi[i] = M;      // Tutti gli attrezzi sono inizialmente liberi
        p->attrezzi_prenotati[i] = 0;   // Nessun attrezzo è inizialmente prenotato
        pthread_mutex_init(&p->mutex[i], NULL);
        pthread_cond_init(&p->cond[i], NULL);
    }
}

void usaattrezzo(struct palestra_t *p, int numeropersona, int tipoattrezzo) {
    pthread_mutex_lock(&p->mutex[tipoattrezzo]);
    printf("Persona %d cerca di usare attrezzo di tipo %d\n", numeropersona, tipoattrezzo);

    while (p->attrezzi_liberi[tipoattrezzo] == 0 && p->attrezzi_prenotati[tipoattrezzo] == 0) {
        // Aspetta che un attrezzo del tipo richiesto sia disponibile
        printf("Persona %d attende l'attrezzo di tipo %d\n", numeropersona, tipoattrezzo);
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

void prenota(struct palestra_t *p, int numeropersona, int tipoattrezzo) {
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

void fineuso(struct palestra_t *p, int numeropersona, int tipoattrezzo) {
    pthread_mutex_lock(&p->mutex[tipoattrezzo]);

    // Rilascia l'attrezzo e segnala agli altri thread che è libero
    p->attrezzi_liberi[tipoattrezzo]++;
    printf("Persona %d ha finito di usare attrezzo tipo %d\n", numeropersona, tipoattrezzo);

    pthread_cond_signal(&p->cond[tipoattrezzo]); // Segnala che un attrezzo si è liberato

    pthread_mutex_unlock(&p->mutex[tipoattrezzo]);
}

void *persona(void *arg) {
    int numeropersona = (int)arg;
    int attrezzocorrente = rand() % N;
    int prossimoattrezzo = rand() % N;

    for (int i = E; i > 0; i--) {
        usaattrezzo(&palestra, numeropersona, attrezzocorrente);

        if (i != 0) {
            prenota(&palestra, numeropersona, prossimoattrezzo);
        }

        fineuso(&palestra, numeropersona, attrezzocorrente);

        if (i != 0) {
            attrezzocorrente = prossimoattrezzo;
            prossimoattrezzo = rand() % N;
        }
    }
    return NULL;
}


int main() {
    int i=0;
    pthread_attr_t a;
    pthread_t pa;

    init_palestra(&palestra);

    srand(55);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i = 0; i < P; i++) {
        pthread_create(&pa, &a, persona, (void *)i);
    }
    pthread_attr_destroy(&a);

    sleep(1);

    return 0;
}
