#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NBARCHE 10
#define NPOSTI_LIBERI 3

int nanosleep(const struct timespec *req, struct timespec *rem);

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 100000;
    nanosleep(&t, NULL);
}

struct porto_t
{

    sem_t mutex;
    sem_t entrata;
    sem_t uscita;

    int b_entrata, b_uscita, in_transito, in_sosta;

} porto;

void init_porto(struct porto_t *porto)
{

    sem_init(&porto->mutex, 0, 1);
    sem_init(&porto->entrata, 0, 0);
    sem_init(&porto->uscita, 0, 0);

    porto->b_entrata = porto->b_uscita = porto->in_sosta = 0;
    porto->in_transito = 2; // al massimo due in transito
}

void entrata_richiestaporto(struct porto_t *porto)
{

    sem_wait(&porto->mutex);
    printf("%ld -> Richiesta di entrare nel porto\n", pthread_self());

    if (porto->in_sosta == NPOSTI_LIBERI || porto->in_transito == 0 || porto->b_uscita)
    {
        // non posso entrare nel porto perchè il canale è pieno e non ci sono posti
        //  così non occupo il canale inutilmente
        printf("Barca %ld -> Non posso entrare nel porto.\n");
        porto->b_entrata++;
        sem_post(&porto->mutex);
        sem_wait(&porto->entrata); // quando mi svegliano avrò già il mutex.
    }
    porto->in_transito--;
    printf("%ld -> Barca nel passaggio (entrata). Barche nel passaggio: %d\n", pthread_self(), porto->in_transito);

    sem_post(&porto->mutex);
}

void entrata_ok(struct porto_t *porto)
{
    // ho finito di transitare, mi parcheggio
    sem_wait(&porto->mutex);
    porto->in_transito++;
    porto->in_sosta++;
    printf("%ld -> Barca entrata nel porto. Barche nel porto: %d\n", pthread_self(), porto->in_transito);


    if (porto->b_uscita)
    {
        porto->b_uscita--;
        sem_post(&porto->uscita);
    }
    else if (porto->b_entrata)
    {
        porto->b_entrata--;
        sem_post(&porto->entrata);
    }
    else
    {
        sem_post(&porto->mutex);
    }
}

void uscita_richiesta(struct porto_t *porto)
{

    sem_wait(&porto->mutex);

    if (porto->in_transito == 0)
    {
        printf("%ld -> Richiesta di uscire dal porto (in attesa: %d)\n", pthread_self(), porto->in_transito);

        porto->b_uscita++;
        sem_post(&porto->mutex);
        sem_wait(&porto->uscita);
    }

    porto->in_transito--;
    sem_post(&porto->mutex);
}

void uscita_ok(struct porto_t *porto)
{

    sem_wait(&porto->mutex);

    porto->in_transito++;
    porto->in_sosta--;
    printf("%ld -> Barca uscita dal porto. Barche nel porto: %d\n", pthread_self(), porto->in_sosta);

    if (porto->b_uscita)
    {
        porto->b_uscita--;
        sem_post(&porto->uscita);
    }
    else if (porto->b_entrata)
    {
        porto->b_entrata--;
        sem_post(&porto->entrata);
    }
    else
    {
        sem_post(&porto->mutex);
    }
}

void *barca(void *arg)
{

    entrata_richiestaporto(&porto);
    pausetta();
    entrata_ok(&porto);
    pausetta();
    uscita_richiesta(&porto);
    pausetta();
    uscita_ok(&porto);
    return 0;
}

int main(void)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il mio sistema */
    init_porto(&porto);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    int i = 0;
    while (i < NBARCHE)
    {
        pthread_create(&p, &a, barca, NULL);
        i++;
    }

    pthread_attr_destroy(&a);
    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);
    printf("%s", "Chiusura!\n");

    return 0;
}