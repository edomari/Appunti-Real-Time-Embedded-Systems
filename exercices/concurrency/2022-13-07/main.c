#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define CLIENTI 10 // il numero di thread
#define CAPIENZA_MAX 4   // la capacità interna della biglietteria

struct discoteca_t
{
    sem_t mutex;        // per l'accesso mutua esclusivo delle risorse manipolate
    sem_t porta;        // per l'eventuale attesa dovuta al fatto che dentro ci sono persone = capacità
    sem_t cassa;        // per l'eventuale attesa dovuta al fatto che c'è la fila alla cassa
    sem_t cassiera;     // per l'attesa della cassiera di eventuali clienti

    int coda_dentro, coda_fuori, bcassa, cassa_disponibile;
    int cassiera_b;
    int cliente_presente;

} discoteca;

void init_discoteca(struct discoteca_t *d)
{
    sem_init(&d->mutex, 0, 1);         // mutex inizializzato a 1 (libero)
    sem_init(&d->porta, 0, 0);       // semaforo per i clienti in attesa fuori
    sem_init(&d->cassa, 0, 0);         // semaforo per i clienti in attesa alla cassa
    sem_init(&d->cassiera, 0, 0);      // semaforo per la cassiera in attesa di clienti

    d->coda_dentro = d->coda_fuori = d->bcassa = d->cassa_disponibile = 0;
    d->cliente_presente = 0;
    d->cassiera_b = 0;
}

void cliente_coda_fuori(struct discoteca_t *d)
{
    sem_wait(&d->mutex);
    // devo controllare se posso entrare all'interno della discoteca o se è stata raggiunta la massima capacità
    while (d->coda_dentro == CAPIENZA_MAX)
    {
        // NON POSSO ENTRARE MI BLOCCO SULLA CODA FUORI
        d->coda_fuori++;
        printf("Cliente %ld > Aspetto fuori\n", pthread_self());
        sem_post(&d->mutex); // rilascio il mutex prima di bloccarmi
        sem_wait(&d->porta); // mi blocco sulla coda fuori
        sem_wait(&d->mutex); // ri-acquisisco il mutex dopo essere stato risvegliato
        d->coda_fuori--;
    }
    // arrivati a questo punto posso entrare dentro
    printf("Cliente %ld > Sono entrato\n", pthread_self());
    d->coda_dentro++;
    sem_post(&d->mutex);
}

void cliente_coda_dentro(struct discoteca_t *d)
{
    sem_wait(&d->mutex);
    // devo controllare se c'è già gente in coda --> mi blocco, se in coda non c'è nessuno devo occuparmi di svegliare la cassiera in attesa dei clienti
    printf("Cliente %ld > appena arrivato la cassa è %d\n", pthread_self(), d->cassa_disponibile);
    while (!d->cassa_disponibile)
    {
        d->bcassa++;
        sem_post(&d->mutex); // rilascio il mutex prima di bloccarmi
        sem_wait(&d->cassa); // mi blocco sulla coda della cassa
        sem_wait(&d->mutex); // ri-acquisisco il mutex dopo essere stato risvegliato
        d->bcassa--;
    }
    d->cliente_presente = 1;
    if (d->cassiera_b)
    {
        printf("Cliente %ld > Sveglio la cassiera \n", pthread_self());
        sem_post(&d->cassiera); // sveglio la cassiera
    }
    d->cassa_disponibile = 0;
    sem_post(&d->mutex);
}

void cliente_esco_coda(struct discoteca_t *d)
{
    sem_wait(&d->mutex);
    d->coda_dentro--;
    // devo uscire fuori, mi devo occupare di far entrare gli altri solo se sono l'ultimo in cassa.
    if (!d->coda_dentro)
    {
        for (int i = 0; i < d->coda_fuori; i++)
        {
            sem_post(&d->porta); // sveglio tutti i clienti in attesa fuori
        }
    }
    sem_post(&d->mutex);
}

void cassiera_attesa_cliente(struct discoteca_t *d)
{
    // quando si blocca la cassiera? se nessun cliente è presente
    sem_wait(&d->mutex);
    d->cassa_disponibile = 1;
    sem_post(&d->cassa); // segnalo che la cassa è disponibile
    while (!d->cliente_presente)
    {
        d->cassiera_b++;
        sem_post(&d->mutex); // rilascio il mutex prima di bloccarmi
        sem_wait(&d->cassiera); // mi blocco in attesa di un cliente
        sem_wait(&d->mutex); // ri-acquisisco il mutex dopo essere stato risvegliato
        d->cassiera_b--;
    }
    d->cliente_presente = 0;
    sem_post(&d->mutex);
}

void cassiera_cliente_servito(struct discoteca_t *d)
{
    sem_wait(&d->mutex);

    printf("Cassiera > Cliente servito\n");
    if (d->bcassa)
    {
        sem_post(&d->cassa); // sveglio un cliente in attesa alla cassa
    }

    sem_post(&d->mutex);
}

void pausetta(int quanto)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 100 + 1) * 1000000 + quanto;
    nanosleep(&t, NULL);
}

void *cliente(void *arg)
{
    pausetta(100);
    // ARRIVA ALL'INGRESSO DELLA DISCO
    printf("Il cliente %ld arriva fuori dalla discoteca\n", pthread_self());
    cliente_coda_fuori(&discoteca);

    printf("Il cliente %ld arriva dentro la discoteca\n", pthread_self());
    // VADO A FARE IL BIGLIETTO DALLA CASSIERA
    cliente_coda_dentro(&discoteca);
    printf("Il cliente %ld è in cassa e sta pagando il biglietto\n", pthread_self());

    // PAGO E RITIRO IL BIGLIETTO
    cliente_esco_coda(&discoteca);
    // A BALLARE
    printf("Il cliente %ld è in pista!!!\n", pthread_self());

    return NULL;
}

void *cassiera(void *arg)
{
    while (1)
    {
        cassiera_attesa_cliente(&discoteca);
        // EMISSIONE BIGLIETTO E RISCOSSIONE SINGOLO CLIENTE
        cassiera_cliente_servito(&discoteca);
        // METTO A POSTO I SOLDINI
    }
    return NULL;
}

int main()
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il mio sistema */
    init_discoteca(&discoteca);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, cassiera, (void *)"1");

    int i = 0;
    while (i < CLIENTI)
    {
        pthread_create(&p, &a, cliente, (void *)"1");
        i++;
    }

    pthread_attr_destroy(&a);
    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);
    printf("%s", "Chiusura!\n");

    return 0;
}