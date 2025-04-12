#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10 // max. torte invendute
#define CLIENTI 5

struct pasticceria_t {
    sem_t mutex;        // Semaforo per l'accesso mutuamente esclusivo
    sem_t cuoco;        // Semaforo per il cuoco in attesa
    sem_t commesso;     // Semaforo per il commesso in attesa
    sem_t richiesta;    // Semaforo per la richiesta di una torta
    sem_t consegna;     // Semaforo per la consegna della torta

    int torta_richiesta;
    int num_torte_da_vendere;
} g_pasticceria;

// Inizializzazione della struttura pasticceria
void pasticceria_init(struct pasticceria_t* p) {
    sem_init(&p->mutex, 0, 1);       // Mutex inizializzato a 1 (libero)
    sem_init(&p->cuoco, 0, 0);       // Semaforo per il cuoco in attesa (inizialmente bloccato)
    sem_init(&p->commesso, 0, 0);    // Semaforo per il commesso in attesa (inizialmente bloccato)
    sem_init(&p->richiesta, 0, 0);   // Semaforo per la richiesta di una torta (inizialmente bloccato)
    sem_init(&p->consegna, 0, 0);    // Semaforo per la consegna della torta (inizialmente bloccato)

    p->num_torte_da_vendere = 0;     // Nessuna torta inizialmente disponibile
    p->torta_richiesta = 0;          // Nessuna richiesta iniziale
}

// Il cuoco inizia a produrre una torta
void cuoco_inizio_torta(struct pasticceria_t* p) {
    sem_wait(&p->mutex);             // Acquisisce il mutex per accedere alle risorse condivise

    // Aspetta se ci sono troppe torte invendute
    if (p->num_torte_da_vendere == N) {
        printf("CUOCO: aspetto che ci siano torte da produrre.\n");
        sem_wait(&p->cuoco);         // Blocca il cuoco finché un commesso lo sveglia
    }

    printf("CUOCO: inizio a produrre un'altra torta, attualmente ce ne sono %d.\n", p->num_torte_da_vendere);
    sem_post(&p->mutex);             // Rilascia il mutex
}

// Il cuoco finisce di produrre una torta
void cuoco_fine_torta(struct pasticceria_t* p) {
    sem_wait(&p->mutex);             // Acquisisce il mutex per accedere alle risorse condivise

    p->num_torte_da_vendere++;       // Produce una torta
    printf("CUOCO: ho prodotto la torta numero %d.\n", p->num_torte_da_vendere);
    sem_post(&p->commesso);          // Sveglia un commesso in attesa di una torta
    printf("CUOCO: Commesso! La torta numero %d è pronta!\n", p->num_torte_da_vendere);

    sem_post(&p->mutex);             // Rilascia il mutex
}

// Il commesso prende una torta
void commesso_prendo_torta(struct pasticceria_t* p) {
    sem_wait(&p->mutex);             // Acquisisce il mutex per accedere alle risorse condivise

    if (!p->torta_richiesta) {
        printf("COMMESSO: aspetto che qualcuno chieda una torta.\n");
        sem_post(&p->mutex); 
        sem_wait(&p->richiesta);     // Blocca il commesso finché un cliente fa una richiesta
        printf("1\n");
    }
    printf("COMMESSO: salve, prenderò il suo ordine!\n");

    if (p->num_torte_da_vendere == 0) {
        printf("COMMESSO: aspetto che il cuoco prepari una torta.\n");
        sem_post(&p->mutex); 
        sem_wait(&p->commesso);      // Blocca il commesso finché il cuoco produce una torta
        printf("2\n");
    }
    printf("COMMESSO: procedo a vendere la torta numero %d.\n", p->num_torte_da_vendere);

    sem_post(&p->mutex);             // Rilascia il mutex
}

// Il commesso vende una torta
void commesso_vendo_torta(struct pasticceria_t* p) {
    sem_wait(&p->mutex);             // Acquisisce il mutex per accedere alle risorse condivise

    p->torta_richiesta = 0;          // Resetta la richiesta
    p->num_torte_da_vendere--;       // Diminuisce il numero di torte invendute
    printf("COMMESSO: la sua torta è pronta!\n");
    sem_post(&p->consegna);          // Sveglia il cliente in attesa
    
    printf("COMMESSO: cuoco, produca altre torte!\n");
    if(p->num_torte_da_vendere < N){
        sem_post(&p->cuoco);             // Sveglia il cuoco per produrre una nuova torta
    }

    sem_post(&p->mutex);             // Rilascia il mutex
}

// Il cliente acquista una torta
void cliente_acquisto(struct pasticceria_t* p) {
    sem_wait(&p->mutex);             // Acquisisce il mutex per accedere alle risorse condivise
    printf("CLIENTE %ld: salve, vorrei ordinare una torta!\n", pthread_self());

    p->torta_richiesta = 1;          // Richiede una torta
    sem_post(&p->richiesta);         // Sveglia un commesso in attesa di una richiesta
    sem_post(&p->mutex);             // Rilascia il mutex prima di bloccarsi

    sem_wait(&p->consegna);          // Blocca il cliente finché la torta è pronta
    printf("CLIENTE %ld: arrivederci!\n", pthread_self());
}

void pausetta(int quanto)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%100+1)*1000000 + quanto;
  nanosleep(&t,NULL);
}

// Thread del cuoco
void* cuoco(void* arg) {
    while (1) {
        cuoco_inizio_torta(&g_pasticceria);
        pausetta(1000000);
        cuoco_fine_torta(&g_pasticceria);
    }
}

// Thread del commesso
void* commesso(void* arg) {
    while (1) {
        commesso_prendo_torta(&g_pasticceria);
        pausetta(1000000);
        commesso_vendo_torta(&g_pasticceria);
    }
}

// Thread del cliente
void* un_cliente(void* arg) {
    while (1) {
        pausetta(1000000);
        cliente_acquisto(&g_pasticceria);
    }
}

int main(int argc, char* argv[]) {
    pthread_t thread_cuoco;
    pthread_t thread_commesso;
    pthread_t thread_cliente[CLIENTI];

    pasticceria_init(&g_pasticceria);

    pthread_create(&thread_cuoco, NULL, cuoco, NULL);
    pthread_create(&thread_commesso, NULL, commesso, NULL);
    for(int i = 0; i<CLIENTI; i++){
        pthread_create(&thread_cliente[i], NULL, un_cliente, NULL);
    }

    pthread_join(thread_cuoco, NULL);
    pthread_join(thread_commesso, NULL);
    pthread_join(thread_cliente, NULL);

    return 0;
}