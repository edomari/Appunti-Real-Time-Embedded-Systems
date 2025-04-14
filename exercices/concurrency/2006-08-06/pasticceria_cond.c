#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define N 10      // Numero massimo di torte che possono essere in pasticceria (buffer size).
#define CLIENTI 5 // Numero di thread cliente che simulano l'acquisto di torte.

/* Struttura condivisa per rappresentare lo stato della pasticceria. */
struct pasticceria_t{
    pthread_mutex_t mutex;        /* Mutex per proteggere l'accesso alle variabili condivise della pasticceria. */

    pthread_cond_t cuoco;     /* Variabile condizione per segnalare al cuoco che c'è spazio per produrre una nuova torta. */
    pthread_cond_t commesso;  /* Variabile condizione per segnalare al commesso che una torta è pronta per essere venduta. */
    pthread_cond_t richiesta; /* Variabile condizione per segnalare al commesso che un cliente ha fatto una richiesta. */
    pthread_cond_t consegna;  /* Variabile condizione per segnalare al cliente che la sua torta è pronta per la consegna. */

    int torta_richiesta;      /* Flag (booleano) che indica se un cliente ha fatto una richiesta (1) o meno (0). */
    int num_torte_da_vendere; /* Numero attuale di torte pronte per la vendita in pasticceria. */

} pasticceria;

/*
 * Funzione per inizializzare la struttura condivisa 'pasticceria'.
 * Inizializza il mutex e le variabili condizione.
 */
void init_pasticceria(struct pasticceria_t* p){
    pthread_mutexattr_t mutexattr; /* Struttura per definire gli attributi del mutex. */
    pthread_condattr_t condattr;  /* Struttura per definire gli attributi della variabile condizione. */

    pthread_mutexattr_init(&mutexattr); /* Inizializza gli attributi del mutex con i valori di default. */
    pthread_condattr_init(&condattr);  /* Inizializza gli attributi della variabile condizione con i valori di default. */

    pthread_mutex_init(&p->mutex, &mutexattr); /* Inizializza il mutex 'p->mutex' con gli attributi specificati. */
    pthread_cond_init(&p->cuoco, &condattr);     /* Inizializza la variabile condizione 'p->cuoco'. */
    pthread_cond_init(&p->commesso, &condattr);  /* Inizializza la variabile condizione 'p->commesso'. */
    pthread_cond_init(&p->richiesta, &condattr); /* Inizializza la variabile condizione 'p->richiesta'. */
    pthread_cond_init(&p->consegna, &condattr);  /* Inizializza la variabile condizione 'p->consegna'. */

    p->num_torte_da_vendere = 0; /* Inizializza il numero di torte da vendere a 0. */
    p->torta_richiesta = 0;      /* Inizializza il flag di richiesta torta a 0 (nessuna richiesta iniziale). */

    pthread_condattr_destroy(&condattr);  /* Libera le risorse associate agli attributi della variabile condizione. */
    pthread_mutexattr_destroy(&mutexattr); /* Libera le risorse associate agli attributi del mutex. */
}

/*
 * Funzione chiamata dal thread cuoco prima di iniziare a preparare una torta.
 * Attende se il magazzino è pieno (troppe torte invendute).
 */
void cuoco_inizio_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */

    /* Se il numero di torte da vendere è maggiore o uguale alla capacità massima (N), */
    if (p->num_torte_da_vendere >= N){
        printf("CUOCO: aspetto che ci siano torte da produrre.\n");
        pthread_cond_wait(&p->cuoco, &p->mutex); // Il cuoco si mette in attesa sulla variabile condizione 'p->cuoco'.
                                                // Il mutex 'p->mutex' viene rilasciato durante l'attesa e riacquisito al risveglio.
    }

    printf("CUOCO: inizio a produrre un altra torta, attualmente ce ne sono %d.\n", p->num_torte_da_vendere);

    pthread_mutex_unlock(&p->mutex); /* Rilascia il mutex dopo aver controllato la condizione. */
}

/*
 * Funzione chiamata dal thread cuoco dopo aver preparato una torta.
 * Incrementa il numero di torte disponibili e segnala un commesso.
 */
void cuoco_fine_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */

    p->num_torte_da_vendere++; // Incrementa il numero di torte pronte per la vendita.
    printf("CUOCO: inizio a produrre la torta numero %d.\n", p->num_torte_da_vendere);
    pthread_cond_signal(&p->commesso); // Segnala (risveglia) un thread commesso in attesa sulla variabile condizione 'p->commesso',
                                       // indicando che una nuova torta è disponibile.
    printf("CUOCO: Commesso! la torta numero %d è pronta!\n", p->num_torte_da_vendere);
    pthread_mutex_unlock(&p->mutex); /* Rilascia il mutex dopo aver aggiornato lo stato e segnalato il commesso. */
}

/*
 * Funzione chiamata dal thread commesso per prendere una torta.
 * Attende una richiesta da un cliente e poi una torta dal cuoco se non ce ne sono.
 */
void commesso_prendo_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */

    /* Se non c'è una torta richiesta (nessun cliente ha fatto una richiesta), */
    if (!p->torta_richiesta){
        printf("COMMESSO: aspetto che qualcuno chieda una torta.\n");
        pthread_cond_wait(&p->richiesta, &p->mutex); // Il commesso si mette in attesa sulla variabile condizione 'p->richiesta',
                                                    // aspettando che un cliente faccia una richiesta. Il mutex viene rilasciato durante l'attesa.
    }
    printf("COMMESSO: salve prenderò il suo ordine!.\n");
    /* Se non ci sono torte da vendere (il numero di torte da vendere non è maggiore di 0), */
    if (!(p->num_torte_da_vendere > 0)){
        printf("COMMESSO: aspetto che il cuoco prepari una torta.\n");
        pthread_cond_wait(&p->commesso, &p->mutex); // Il commesso si mette in attesa sulla variabile condizione 'p->commesso',
                                                    // aspettando che il cuoco produca una torta. Il mutex viene rilasciato durante l'attesa.
    }
    printf("COMMESSO: procedo a vendere la torta.\n");

    pthread_mutex_unlock(&p->mutex); /* Rilascia il mutex dopo aver gestito la richiesta e (eventualmente) atteso una torta. */
}

/*
 * Funzione chiamata dal thread commesso per vendere una torta a un cliente.
 * Resetta il flag di richiesta, decrementa il numero di torte e segnala il cliente e il cuoco.
 */
void commesso_vendo_torta(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */

    p->torta_richiesta = 0; // Resetta il flag di richiesta torta, indicando che la richiesta è stata gestita.
    p->num_torte_da_vendere--; // Decrementa il numero di torte disponibili per la vendita.
    printf("COMMESSO: la sua torta è pronta!.\n");
    pthread_cond_signal(&p->consegna); // Segnala (risveglia) il thread cliente in attesa sulla variabile condizione 'p->consegna',
                                       // indicando che la sua torta è pronta.
    printf("COMMESSO: cuoco produca altre torte!.\n");
    pthread_cond_signal(&p->cuoco); // Segnala (risveglia) il thread cuoco in attesa sulla variabile condizione 'p->cuoco',
                                    // indicando che c'è spazio per produrre una nuova torta.

    pthread_mutex_unlock(&p->mutex); /* Rilascia il mutex dopo aver aggiornato lo stato e segnalato cliente e cuoco. */
}

/*
 * Funzione chiamata dal thread cliente per simulare l'acquisto di una torta.
 * Imposta il flag di richiesta e attende la consegna.
 */
void cliente_acquisto(struct pasticceria_t* p){
    pthread_mutex_lock(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
    printf("CLIENTE %lu: salve vorrei ordinare una torta!.\n", pthread_self());
    p->torta_richiesta = 1; // Imposta il flag di richiesta torta a 1, indicando che un cliente ha fatto una richiesta.
    pthread_cond_signal(&p->richiesta); // Segnala (risveglia) un thread commesso in attesa sulla variabile condizione 'p->richiesta',
                                        // informandolo di una nuova richiesta.
    pthread_cond_wait(&p->consegna, &p->mutex); // Il cliente si mette in attesa sulla variabile condizione 'p->consegna',
                                                // aspettando che il commesso lo segnali con la torta pronta.
                                                // Il mutex viene rilasciato durante l'attesa.
    printf("CLIENTE %lu: arrivederci!.\n", pthread_self());
    pthread_mutex_unlock(&p->mutex); /* Rilascia il mutex dopo aver ricevuto la torta. */
}

/*
 * Funzione per simulare un breve periodo di inattività (pausa) per i thread.
 * Utilizza nanosleep per una pausa più precisa.
 */
void pausetta(int quanto)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%100+1)*1000000 + quanto; /* Genera un tempo di attesa casuale in nanosecondi. */
    nanosleep(&t,NULL);                           /* Mette il thread in pausa. */
}

/*
 * Funzione che rappresenta il comportamento del cuoco.
 * Il thread cuoco esegue questa funzione in un ciclo infinito.
 */
void* cuoco(void* arg){
    while (1){ // Ciclo infinito per la produzione continua di torte.
        cuoco_inizio_torta(&pasticceria); // Attende se il magazzino è pieno.
        pausetta(100000);                // Simula il tempo impiegato per preparare una parte della torta.
        cuoco_fine_torta(&pasticceria);   // Segnala che una torta è pronta.
        pausetta(100000);                // Simula il tempo impiegato per preparare un'altra parte della torta.
    }
}

/*
 * Funzione che rappresenta il comportamento del commesso.
 * Il thread commesso esegue questa funzione in un ciclo infinito.
 */
void* commesso(void* arg){
    while (1){ // Ciclo infinito per la gestione continua delle vendite.
        commesso_prendo_torta(&pasticceria); // Attende una richiesta e (se necessario) una torta.
        pausetta(100000);                 // Simula il tempo impiegato per interagire con il cliente.
        commesso_vendo_torta(&pasticceria);  // Vende la torta e segnala cliente e cuoco.
        pausetta(100000);                 // Simula il tempo tra una vendita e l'altra.
    }
}

/*
 * Funzione che rappresenta il comportamento di un cliente.
 * I thread cliente eseguono questa funzione in un ciclo infinito.
 */
void* un_cliente(void* arg){
    while (1){ // Ciclo infinito per simulare acquisti ripetuti.
        pausetta(100000);           // Simula il tempo tra un acquisto e l'altro.
        cliente_acquisto(&pasticceria); // Simula l'acquisto di una torta.
    }
}

/*
 * Funzione principale (entry point) del programma.
 * Crea i thread per il cuoco, il commesso e i clienti.
 */
int main(int argc, char* argv[]){
    pthread_attr_t a; /* Struttura per definire gli attributi dei thread. */
    pthread_t pa;     /* Variabile per memorizzare l'ID del thread creato. */

    init_pasticceria(&pasticceria); /* Inizializza la struttura condivisa della pasticceria. */

    pthread_attr_init(&a);                     /* Inizializza la struttura degli attributi del thread con i valori di default. */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED); /* Imposta lo stato del thread come 'detached',
                                                              in modo che le risorse del thread vengano rilasciate
                                                              automaticamente al termine dell'esecuzione. */

    pthread_create(&pa, &a, cuoco, NULL);    /* Crea un thread cuoco che esegue la funzione 'cuoco'. */
    pthread_create(&pa, &a, commesso, NULL); /* Crea un thread commesso che esegue la funzione 'commesso'. */
    /* Crea un numero di thread cliente definiti dalla costante CLIENTI. */
    for(int i = 0; i<CLIENTI; i++){
        pthread_create(&pa, &a, un_cliente, NULL); /* Crea un thread cliente che esegue la funzione 'un_cliente'. */
    }

    pthread_attr_destroy(&a); /* Libera le risorse associate alla struttura degli attributi del thread. */

    sleep(10); /* Il thread principale dorme per 10 secondi per permettere agli altri thread di eseguire.
               * Dato che i thread dei lavoratori e dei clienti sono in loop infiniti, il programma
               * continuerebbe a girare senza questa pausa se non venisse interrotto esternamente. */

    return 0; /* Il programma termina. */
}