#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#define CLIENTI 20   // il numero di clienti
#define CAPACITA 4   // la capacità interna della biglietteria

struct discoteca_t
{
    pthread_mutex_t mutex;   // per l'accesso esclusivo delle risorse manipolate
    pthread_cond_t porta;    // per l'eventuale attesa dovuta al fatto che dentro ci sono persone = capacità
    pthread_cond_t cassa;    // per l'eventuale attesa dovuta al fatto che c'è la fila alla cassa
    pthread_cond_t cassiera; // per l'attesa della cassiera di eventuali clienti

    int n_clienti_cassa, cassa_bloccata, cassa_disponibile;
    int cassiera_bloccata;
    int cliente_presente;

} discoteca;

void init_discoteca(struct discoteca_t *d)
{
    pthread_mutexattr_t m;
    pthread_condattr_t c;

    pthread_mutexattr_init(&m);
    pthread_condattr_init(&c);

    pthread_mutex_init(&d->mutex, &m);
    pthread_cond_init(&d->porta, &c);
    pthread_cond_init(&d->cassa, &c);
    pthread_cond_init(&d->cassiera, &c);

    d->n_clienti_cassa = d->cassa_bloccata = d->cassa_disponibile = 0;
    d->cliente_presente = 0;
    d->cassiera_bloccata = 0;

    pthread_mutexattr_destroy(&m);
    pthread_condattr_destroy(&c);
}

void cliente_coda_fuori(struct discoteca_t *d)
{
    printf("Il cliente %ld arriva fuori dalla discoteca\n", pthread_self());
    pthread_mutex_lock(&d->mutex);
    // devo controllare se posso entrare all'interno della discoteca o se è stata raggiunta la massima capacità-
    while (d->n_clienti_cassa == CAPACITA)
    {
        // NON POSSO ENTRARE MI BLOCCO SULLA CODA FUORI
        printf("Cliente %ld > Aspetto fuori\n", pthread_self());
        pthread_cond_wait(&d->porta, &d->mutex);
    }
    // arrivati a questo punto posso entrare dentro
    printf("Cliente %ld > Sono entrato\n", pthread_self());
    d->n_clienti_cassa++;
    pthread_mutex_unlock(&d->mutex);
}

void cliente_coda_dentro(struct discoteca_t *d)
{
    printf("Il cliente %ld arriva dentro la discoteca\n", pthread_self());

    pthread_mutex_lock(&d->mutex);
    // devo controllare se c'è già gente in coda --> mi blocco, se in coda non c'è nessuno devo occuparmi di svegliare la cassiera in attesa dei clienti
    printf("Cliente %ld > appena arrivato la cassa è %d\n", pthread_self(), d->cassa_disponibile);
    while (!d->cassa_disponibile)
    {
        d->cassa_bloccata++;
        pthread_cond_wait(&d->cassa, &d->mutex); // qui mi sveglia la cassiera che controlla se c'è qualcuno bloccato in cassa prima di sospendersi.
        d->cassa_bloccata--;
    }
    d->cliente_presente = 1;
    if (d->cassiera_bloccata)
    {
        printf("Cliente %ld > Sveglio la cassiera \n", pthread_self());
        pthread_cond_signal(&d->cassiera);
    }
    d->cassa_disponibile = 0;
    printf("Il cliente %ld è in cassa e sta pagando il biglietto\n", pthread_self());
    pthread_mutex_unlock(&d->mutex);
}

void cliente_esco_coda(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);
    d->n_clienti_cassa--;
    // devo uscire fuori, mi devo occupare di far entrare gli altri solo se sono l'ultimo in cassa.
    if (d->n_clienti_cassa == 0)
    {
        pthread_cond_broadcast(&d->porta);
    }
    printf("Il cliente %ld è in pista!!!\n", pthread_self());

    pthread_mutex_unlock(&d->mutex);
}

void cassiera_attesa_cliente(struct discoteca_t *d)
{
    // qundo si blocca la cassiera? se nessun cliente è presente
    pthread_mutex_lock(&d->mutex);
    d->cassa_disponibile = 1;
    pthread_cond_signal(&d->cassa);
    while (d->cliente_presente == 0)
    {
        d->cassiera_bloccata++;
        pthread_cond_wait(&d->cassiera, &d->mutex);
        d->cassiera_bloccata--;
    }
    d->cliente_presente = 0;
    pthread_mutex_unlock(&d->mutex);
}

void cassiera_cliente_servito(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    printf("Cassiera > Cliente servito\n");
    if (d->cassa_bloccata != 0)
    {
        pthread_cond_signal(&d->cassa);
    }

    pthread_mutex_unlock(&d->mutex);
}

void pausetta(int quanto)
{
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand()%100+1)*1000000 + quanto;
	nanosleep(&t,NULL);
}

void *cliente(void *arg)
{
    pausetta(10000);
    cliente_coda_fuori(&discoteca);

    cliente_coda_dentro(&discoteca);

    cliente_esco_coda(&discoteca);
}

void *cassiera(void *arg)
{
    while (1)
    {
        cassiera_attesa_cliente(&discoteca);
        pausetta(10000);
        cassiera_cliente_servito(&discoteca);
        pausetta(10000);
    }
}

int main() {
    pthread_attr_t a;
    pthread_t p;

    init_discoteca(&discoteca);
    srand(555);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, cassiera, NULL);

    for(int i = 0; i < CLIENTI; i++) {
        pthread_create(&p, &a, cliente, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(20);
    printf("Chiusura!\n");

    return 0;
}