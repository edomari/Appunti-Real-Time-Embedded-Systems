#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 8
#define NCLIENTI 32

struct officina_t
{
    pthread_mutex_t mutex;
    pthread_cond_t attesa_ufficio;
    pthread_cond_t attesa_riparazione;
    pthread_cond_t operaio[N];

    int b_ufficio, dentro_ufficio;

} o;

void init_officina(struct officina_t *officina)
{
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_condattr_init(&cond_attr);

    pthread_mutex_init(&officina->mutex, &mutex_attr);
    pthread_cond_init(&officina->attesa_riparazione, &cond_attr);
    pthread_cond_init(&officina->attesa_ufficio, &cond_attr);

    for (int i = 0; i < N; i++)
    {
        pthread_cond_init(&officina->operaio[i], &cond_attr);
    }

    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_destroy(&cond_attr);

    officina->b_ufficio = 0;
    officina->dentro_ufficio = -1; // forse dentro ufficio può anche non servire....
}

// Funzione per simulare l'arrivo di un cliente nell'ufficio
void cliente_arrivo(struct officina_t *officina, int r)
{
    pthread_mutex_lock(&officina->mutex); // Acquisisce il mutex per accedere alle risorse condivise

    // Se l'ufficio è occupato, il cliente attende fuori
    while (officina->dentro_ufficio != -1)
    {
        officina->b_ufficio++; // Incrementa il contatore dei clienti in attesa
        printf("%ld > Aspetto fuori per il servizio %d\n", pthread_self(), r); // Stampa messaggio di attesa
        pthread_cond_wait(&officina->attesa_ufficio, &officina->mutex); // Rilascia il mutex e si blocca sulla condition variable
        officina->b_ufficio--; // Decrementa il contatore dopo essere stato risvegliato
    }
    officina->dentro_ufficio = r; // Imposta il cliente corrente come quello servito

    pthread_mutex_unlock(&officina->mutex); // Rilascia il mutex
    printf("%ld > Chiedo il servizio %d\n", pthread_self(), r); // Stampa messaggio di richiesta servizio
    pthread_cond_signal(&officina->operaio[r]); // Notifica all'operaio corrispondente che c'è un cliente
}

// Funzione per simulare l'attesa del cliente fino alla fine del servizio
void cliente_attesafineservizio(struct officina_t *officina)
{
    pthread_mutex_lock(&officina->mutex); // Acquisisce il mutex per accedere alle risorse condivise
    printf("%ld > in attesa della fine del servizio\n", pthread_self()); // Stampa messaggio di attesa
    pthread_cond_wait(&officina->attesa_riparazione, &officina->mutex); // Rilascia il mutex e si blocca sulla condition variable
    printf("%ld > Servizio finito... Vado a casa\n", pthread_self()); // Stampa messaggio di fine servizio
    officina->dentro_ufficio = -1; // Libera l'ufficio

    if (officina->b_ufficio) // Se ci sono clienti in attesa fuori
    {
        pthread_cond_signal(&officina->attesa_ufficio); // Sveglia uno di loro
    }
    pthread_mutex_unlock(&officina->mutex); // Rilascia il mutex
}

// Funzione per simulare l'attesa di un operaio per un cliente
void operaio_attesacliente(struct officina_t *officina, int r)
{
    pthread_mutex_lock(&officina->mutex); // Acquisisce il mutex per accedere alle risorse condivise
    printf("Operaio che si occupa di r --> %d in attesa\n", r); // Stampa messaggio di attesa

    // L'operaio attende finché non viene richiesto il suo servizio
    while (officina->dentro_ufficio != r) // Se nessun cliente richiede il servizio `r`, attendi
    {
        pthread_cond_wait(&officina->operaio[r], &officina->mutex); // Rilascia il mutex e si blocca sulla condition variable
    }
    printf("L'operaio %ld > Qualcuno ha chiesto la riparazione %d che svolgo solo io\n", pthread_self(), r); // Stampa messaggio di richiesta ricevuta
    pthread_mutex_unlock(&officina->mutex); // Rilascia il mutex
}

// Funzione per simulare la fine del servizio da parte dell'operaio
void operaio_fineservizio(struct officina_t *officina)
{
    pthread_mutex_lock(&officina->mutex); // Acquisisce il mutex per accedere alle risorse condivise
    printf("L'operaio %ld > Ho terminato il servizio richiesto, avviso il cliente\n", pthread_self()); // Stampa messaggio di fine servizio

    pthread_cond_signal(&officina->attesa_riparazione); // Notifica al cliente che il servizio è terminato
    pthread_mutex_unlock(&officina->mutex); // Rilascia il mutex
}

// Funzione eseguita dai thread dei clienti
void *cliente(void *arg)
{
    int r = rand() % N; // Genera casualmente il tipo di servizio richiesto
    cliente_arrivo(&o, r); // Simula l'arrivo del cliente nell'ufficio
    cliente_attesafineservizio(&o); // Simula l'attesa del cliente fino alla fine del servizio
    return NULL; // Termina il thread del cliente
}

// Funzione eseguita dai thread degli operai
void *operaio(void *arg)
{
    int r = (intptr_t)arg; // Recupera il tipo di servizio gestito dall'operaio
    while (1) // Ciclo infinito
    {
        operaio_attesacliente(&o, r); // Simula l'attesa dell'operaio per un cliente
        printf("Operaio che si occupa di r --> %d è arrivato un cliente\n", r); 

        operaio_fineservizio(&o); // Simula la fine del servizio
        printf("Operaio che si occupa di r --> %d ha finito il servizio\n", r); 
    }
}

int main()
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il mio sistema */
    init_officina(&o);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    int i = 0;
    while (i < NCLIENTI)
    {
        pthread_create(&p, &a, cliente, NULL);
        i++;
    }
    intptr_t x = 0;
    while (x < N)
    {
        pthread_create(&p, &a, operaio, (void *)x);
        x++;
    }

    pthread_attr_destroy(&a);
    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);
    printf("%s", "Chiusura!\n");

    return 0;
}