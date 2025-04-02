#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N 10

struct pasticceria_t{
    pthread_mutex_t mutex;

    pthread_cond_t cuoco;
    pthread_cond_t commesso;
    pthread_cond_t richiesta;
    pthread_cond_t consegna;

    int torta_richiesta;
    int num_torte_da_vendere;

} pasticceria;

void pasticceria_init(struct pasticceria_t* p){
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_condattr_init(&condattr);

    pthread_mutex_init(&p->mutex, &mutexattr);
    pthread_cond_init(&p->cuoco, &condattr);
    pthread_cond_init(&p->commesso, &condattr);
    pthread_cond_init(&p->richiesta, &condattr);

    p->num_torte_da_vendere = 0;
    p->torta_richiesta = 0;

    pthread_condattr_destroy(&condattr);
    pthread_mutexattr_destroy(&mutexattr);
}

/*
Il cuoco inizia a produrre una torta solo se il numero di torte invendute è minore di N
Altrimenti si blocca
*/
void cuoco_inizio_torta(struct pasticceria_t* p)
{
    pthread_mutex_lock(&p->mutex);

    if (p->num_torte_da_vendere >= N)
        pthread_cond_wait(&p->cuoco, &p->mutex);

    pthread_mutex_unlock(&p->mutex);
}

/*
Il cuoco una volta finita di produrre la torta, aumenta il counter delle torte prodotte
Inoltre sveglia un commesso
*/
void cuoco_fine_torta(struct pasticceria_t* p)
{
    pthread_mutex_lock(&p->mutex);
    
    p->num_torte_da_vendere++;
    pthread_cond_signal(&p->commesso);

    pthread_mutex_unlock(&p->mutex);
}

/*
Bloccante:
il commesso si blocca in attesa che il cuoco comunichi che ci sia una torta pronta
*/
void commesso_prendo_torta(struct pasticceria_t* p)
{
    pthread_mutex_lock(&p->mutex);
    
    if (!p->torta_richiesta)
        pthread_cond_wait(&p->richiesta, &p->mutex);

    if (p->num_torte_da_vendere == 0)
        pthread_cond_wait(&p->commesso, &p->mutex);

    pthread_mutex_unlock(&p->mutex);
}

void commesso_vendo_torta(struct pasticceria_t* p)
{
    pthread_mutex_lock(&p->mutex);

    p->torta_richiesta = 1;
    p->num_torte_da_vendere--;
    pthread_cond_signal(&p->consegna);
    pthread_cond_signal(&p->cuoco);

    pthread_mutex_unlock(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t* p)
{
    pthread_mutex_lock(&p->mutex);

    p->torta_richiesta = 1;
    pthread_cond_signal(&p->richiesta);
    pthread_cond_wait(&p->consegna, &p->mutex);

    pthread_mutex_unlock(&p->mutex);
}

void* cuoco(void* arg)
{
    while (1)
    {
        cuoco_inizio_torta(&pasticceria);
        printf("cuoco> Preparo la torta\n");
        cuoco_fine_torta(&pasticceria);
        
        sleep(rand() % 2);
    }
}

void* commesso(void* arg)
{
    while (1)
    {
        commesso_prendo_torta(&pasticceria);
        printf("commesso> Vendo la torta...\n");
        commesso_vendo_torta(&pasticceria);

        sleep(rand() % 2);
    }
}

void* un_cliente(void* arg)
{
    while (1)
    {
        printf("cliente> Acquisto una torta...\n");
        cliente_acquisto(&pasticceria);
        printf("cliente> Torta acquistata\n");

        sleep(rand() % 2);
    }
}

int main(int argc, char* argv[]){
    pthread_t thread_cuoco;
    pthread_t thread_commesso;
    pthread_t thread_cliente;

    pasticceria_init(&pasticceria);

    pthread_create(&thread_cuoco, NULL, cuoco, NULL);
    pthread_create(&thread_commesso, NULL, commesso, NULL);
    pthread_create(&thread_cliente, NULL, un_cliente, NULL);

    pthread_join(thread_cuoco, NULL);
    pthread_join(thread_commesso, NULL);
    pthread_join(thread_cliente, NULL);

    return 0;
}