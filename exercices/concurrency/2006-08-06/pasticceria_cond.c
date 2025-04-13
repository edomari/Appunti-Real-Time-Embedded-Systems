// FUNZIONANTE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define N 10 // max. torte invendute
#define CLIENTI 5

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

void cuoco_inizio_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex);

    // aspetta se ci sono troppe torte invendute
    if (p->num_torte_da_vendere >= N){
        printf("CUOCO: aspetto che ci siano torte da produrre.\n");
        pthread_cond_wait(&p->cuoco, &p->mutex); // aspetto che un commesso mi svegli
    }

    printf("CUOCO: inizio a produrre un altra torta, attualmente ce ne sono %d.\n", p->num_torte_da_vendere);

    pthread_mutex_unlock(&p->mutex);
}

void cuoco_fine_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex);
    
    p->num_torte_da_vendere++; // produco una torta
    printf("CUOCO: inizio a produrre la torta numero %d.\n", p->num_torte_da_vendere);
    pthread_cond_signal(&p->commesso); // sveglio un commesso che aspetta una torta
    printf("CUOCO: Commesso! la torta numero %d è pronta!\n", p->num_torte_da_vendere);
    pthread_mutex_unlock(&p->mutex);
}

void commesso_prendo_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex);
    
    if (!p->torta_richiesta){
        printf("COMMESSO: aspetto che qualcuno chieda una torta.\n");
        pthread_cond_wait(&p->richiesta, &p->mutex); // aspetto che qualcuno chieda una torta, e ...
    }
    printf("COMMESSO: salve prenderò il suo ordine!.\n");
    if (!p->num_torte_da_vendere > 0){
        printf("COMMESSO: aspetto che il cuoco prepari una torta.\n");
        pthread_cond_wait(&p->commesso, &p->mutex); // ... aspetto che il cuoco produca una torta
    }
    printf("COMMESSO: procedo a vendere la torta.\n");

    pthread_mutex_unlock(&p->mutex);
}

void commesso_vendo_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex);

    p->torta_richiesta = 0; // non c'è più una richiesta
    p->num_torte_da_vendere--; // diminuisco le torte invendute
    printf("COMMESSO: la sua torta è pronta!.\n");
    pthread_cond_signal(&p->consegna); // sveglio il cliente in attesa
    printf("COMMESSO: cuoco produca altre torte!.\n");
    pthread_cond_signal(&p->cuoco); // sveglio il cuoco che deve produrre una torta

    pthread_mutex_unlock(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex);
    printf("CLIENTE %lu: salve vorrei ordinare una torta!.\n", pthread_self());
    p->torta_richiesta = 1; // richiedo una torta
    pthread_cond_signal(&p->richiesta); // sveglio un commesso in attesa di una richiesta
    pthread_cond_wait(&p->consegna, &p->mutex); // aspetto che la torta sua pronta
    printf("CLIENTE %lu: arrivederci!.\n", pthread_self());
    pthread_mutex_unlock(&p->mutex);
}

void pausetta(int quanto)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%100+1)*1000000 + quanto;
  nanosleep(&t,NULL);
}

void* cuoco(void* arg){
    while (1){
        cuoco_inizio_torta(&pasticceria);
        pausetta(100000);
        cuoco_fine_torta(&pasticceria);
        pausetta(100000);
    }
}

void* commesso(void* arg){
    while (1){
        commesso_prendo_torta(&pasticceria);
        pausetta(100000);
        commesso_vendo_torta(&pasticceria);
        pausetta(100000);
    }
}

void* un_cliente(void* arg){
    while (1){
        pausetta(100000);
        cliente_acquisto(&pasticceria);
    }
}

int main(int argc, char* argv[]){
    pthread_attr_t a;
    pthread_t pa;

    pasticceria_init(&pasticceria);

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&pa, &a, cuoco, NULL);
    pthread_create(&pa, &a, commesso, NULL);
    for(int i = 0; i<CLIENTI; i++){
        pthread_create(&pa, &a, un_cliente, NULL);
    }

    pthread_attr_destroy(&a);

    sleep(10);

    return 0;
}