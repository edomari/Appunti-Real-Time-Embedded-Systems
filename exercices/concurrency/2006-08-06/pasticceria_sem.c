#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define N 3       // Numero massimo di torte che possono essere prodotte e tenute in pasticceria.
#define CLIENTI 5 // Numero di thread cliente che simulano l'acquisto di torte.

/* Struttura condivisa per rappresentare lo stato della pasticceria. */
struct pasticceria_t
{
    sem_t mutex;                  /* Semaforo binario (mutex) per proteggere l'accesso alle variabili condivise della pasticceria. */
    sem_t cuoco;                  /* Semaforo per segnalare al cuoco che può iniziare a produrre una nuova torta (quando il buffer non è pieno). Inizializzato a 0. */
    sem_t commesso;               /* Semaforo per segnalare al commesso che una torta è pronta per essere venduta o che c'è un cliente in attesa. Inizializzato a 0. */
    sem_t clienti;                /* Semaforo per segnalare a un cliente che la sua torta è pronta per la consegna. Inizializzato a 0. */

    int clienti_in_attesa;        /* Contatore del numero di clienti attualmente in attesa di essere serviti. */
    int n_torte_pronte;           /* Numero attuale di torte pronte per la vendita in pasticceria. */
    bool commesso_aspetta_cliente; /* Flag booleano che indica se il commesso sta aspettando un cliente (true) o meno (false). */
    bool commesso_aspetta_torte;   /* Flag booleano che indica se il commesso sta aspettando che il cuoco prepari una torta (true) o meno (false). */

} pasticceria;

/*
 * Funzione per inizializzare la struttura condivisa 'pasticceria'.
 * Inizializza i semafori e le variabili di stato.
 */
void init_pasticceria(struct pasticceria_t *p)
{
    sem_init(&p->mutex, 0, 1);    /* Inizializza il mutex con valore 1 (risorsa disponibile). */
    sem_init(&p->cuoco, 0, 0);    /* Inizializza il semaforo del cuoco a 0 (nessuna segnalazione iniziale). */
    sem_init(&p->commesso, 0, 0); /* Inizializza il semaforo del commesso a 0 (nessuna segnalazione iniziale). */
    sem_init(&p->clienti, 0, 0);  /* Inizializza il semaforo dei clienti a 0 (nessuna segnalazione iniziale). */

    p->clienti_in_attesa = 0;          /* Inizializza il numero di clienti in attesa a 0. */
    p->n_torte_pronte = 0;             /* Inizializza il numero di torte pronte a 0. */
    p->commesso_aspetta_cliente = false; /* Inizializza il flag di attesa del commesso per i clienti a false. */
    p->commesso_aspetta_torte = false;   /* Inizializza il flag di attesa del commesso per le torte a false. */
}

/*
 * Funzione chiamata dal thread cuoco prima di iniziare a preparare una torta.
 * Verifica se c'è spazio per produrre e, in caso contrario, attende.
 */
void cuoco_inizio_torta(struct pasticceria_t *p)
{
    sem_wait(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */

    printf("Cuoco: ora ci sono %d torte prodotte.\n", p->n_torte_pronte);

    /* Se il numero di torte pronte è inferiore alla capacità massima (N), */
    if (p->n_torte_pronte < N){
        printf("CUOCO: inizio a produrre un altra torta, attualmente ce ne sono %d.\n", p->n_torte_pronte);
        sem_post(&p->cuoco); /* Segnala al cuoco stesso che può procedere con la produzione (usato per la sincronizzazione interna del ciclo di produzione). */
    }
    /* Altrimenti (se il magazzino è pieno), */
    else{
        printf("CUOCO: aspetto che ci siano torte da produrre.\n");
    }
    sem_post(&p->mutex); /* Rilascia il mutex. */
    sem_wait(&p->cuoco); /* Il cuoco attende il proprio segnale per iniziare la produzione (viene dato solo se c'è spazio). */
}

/*
 * Funzione chiamata dal thread cuoco dopo aver preparato una torta.
 * Incrementa il numero di torte pronte e segnala un commesso se sta aspettando.
 */
void cuoco_fine_torta(struct pasticceria_t *p)
{
    sem_wait(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
    p->n_torte_pronte++; /* Incrementa il numero di torte pronte. */
    printf("CUOCO: Commesso! la torta numero %d è pronta!\n", p->n_torte_pronte);

    /* Se il commesso sta aspettando una torta, */
    if (p->commesso_aspetta_torte){
        p->commesso_aspetta_torte = false; /* Resetta il flag di attesa del commesso. */
        sem_post(&p->commesso);            /* Segnala al commesso che una torta è pronta. */
    }

    sem_post(&p->mutex); /* Rilascia il mutex. */
}

/*
 * Funzione chiamata dal thread commesso per prendere una torta.
 * Attende se non ci sono torte pronte.
 */
void commesso_prendo_torta(struct pasticceria_t *p)
{
    sem_wait(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
    /* Se non ci sono torte pronte, */
    if (p->n_torte_pronte == 0)
    {
        p->commesso_aspetta_torte = true; /* Imposta il flag per indicare che il commesso sta aspettando una torta. */
        printf("COMMESSO: aspetto che il cuoco prepari una torta.\n");
        sem_post(&p->cuoco);              /* Segnala al cuoco che è necessario produrre una torta. */
    }
    /* Altrimenti (se ci sono torte pronte), */
    else
    {
        sem_post(&p->commesso); /* Segnala al commesso stesso che può procedere (usato per la sincronizzazione con la vendita). */
    }
    sem_post(&p->mutex); /* Rilascia il mutex. */
    sem_wait(&p->commesso); /* Il commesso attende di essere segnalato quando una torta è disponibile (o se ce n'era già una). */
}

/*
 * Funzione chiamata dal thread commesso per vendere una torta a un cliente.
 * Verifica se ci sono clienti in attesa o si mette in attesa di un cliente.
 */
void commesso_vendo_torta(struct pasticceria_t *p)
{
    sem_wait(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
    /* Se ci sono clienti in attesa, */
    if (p->clienti_in_attesa){
        printf("COMMESSO: c'è un cliente in attesa.\n");
        p->clienti_in_attesa--; /* Decrementa il numero di clienti in attesa. */
        sem_post(&p->commesso); /* Segnala al commesso stesso che può procedere con la vendita. */
        sem_post(&p->clienti);  /* Segnala al cliente in attesa che può essere servito. */
    }
    /* Altrimenti (se non ci sono clienti in attesa), */
    else{
        printf("COMMESSO: aspetto che qualcuno chieda una torta.\n");
        p->commesso_aspetta_cliente = true; /* Imposta il flag per indicare che il commesso sta aspettando un cliente. */
    }
    sem_post(&p->mutex); /* Rilascia il mutex. */
    sem_wait(&p->commesso); /* Il commesso attende di essere segnalato quando c'è un cliente (o se ce n'era già uno). */
}

/*
 * Funzione chiamata dal thread cliente per simulare l'acquisto di una torta.
 * Si mette in coda se il commesso è occupato e attende di essere servito.
 */
void cliente_acquisto(struct pasticceria_t *p)
{
    sem_wait(&p->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
    printf("CLIENTE %lu: salve vorrei ordinare una torta!.\n", pthread_self());
    printf("CLIENTE %lu: %d persone davanti a me.\n", pthread_self(), p->clienti_in_attesa);
    /* Se il commesso sta aspettando un cliente, */
    if (p->commesso_aspetta_cliente){
        p->commesso_aspetta_cliente = false; /* Resetta il flag di attesa del commesso. */
        sem_post(&p->commesso);            /* Segnala al commesso che c'è un cliente. */
    }
    /* Altrimenti (se il commesso non sta aspettando), */
    else{
        p->clienti_in_attesa++; /* Incrementa il numero di clienti in attesa. */
    }
    sem_post(&p->mutex); /* Rilascia il mutex. */
    sem_wait(&p->clienti); /* Il cliente attende di essere segnalato dal commesso quando è il suo turno. */
    printf("CLIENTE %lu: arrivederci!.\n", pthread_self());
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
        cuoco_inizio_torta(&pasticceria); // Verifica se può iniziare a produrre e attende se necessario.
        pausetta(100000);                // Simula il tempo impiegato per preparare una torta.
        cuoco_fine_torta(&pasticceria);   // Incrementa il contatore delle torte e segnala il commesso.
        pausetta(100000);                // Simula un breve intervallo tra la preparazione delle torte.
    }
}

/*
 * Funzione che rappresenta il comportamento del commesso.
 * Il thread commesso esegue questa funzione in un ciclo infinito.
 */
void* commesso(void* arg){
    while (1){ // Ciclo infinito per la gestione continua delle vendite.
        commesso_prendo_torta(&pasticceria); // Attende che ci sia una torta pronta.
        pausetta(100000);                 // Simula il tempo impiegato per servire un cliente.
        commesso_vendo_torta(&pasticceria);  // Serve un cliente (se presente) o attende un cliente.
        pausetta(100000);                 // Simula un breve intervallo tra le vendite.
    }
}

/*
 * Funzione che rappresenta il comportamento di un cliente.
 * I thread cliente eseguono questa funzione in un ciclo infinito.
 */
void* un_cliente(void* arg){
    while (1){ // Ciclo infinito per simulare acquisti ripetuti.
        pausetta(100000);           // Simula un tempo tra un acquisto e l'altro.
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