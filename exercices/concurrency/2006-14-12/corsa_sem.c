#define N 6

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Struttura condivisa per gestire lo stato della corsa.
 * Contiene semafori per la sincronizzazione e variabili per tenere traccia dei corridori e dell'arbitro.
 */
struct corsa_t {
  sem_t mutex;        /* Semaforo binario (mutex) per proteggere l'accesso alle variabili condivise della corsa. */

  sem_t scorridore;   /* Semaforo per segnalare ai corridori il via della corsa. Inizializzato a 0. */
  int ccorridore;     /* Contatore del numero di corridori arrivati alla partenza. */

  int corridoriarrivati; /* Contatore del numero di corridori arrivati al traguardo. */
  int primo, ultimo;   /* Identificatori (numero) del primo e dell'ultimo corridore arrivato. */

  sem_t sarbitro_via;  /* Semaforo per segnalare all'arbitro che tutti i corridori sono pronti per la partenza. Inizializzato a 0. */
  sem_t sarbitro_fine; /* Semaforo per segnalare all'arbitro che tutti i corridori sono arrivati al traguardo. Inizializzato a 0. */
  int carbitro_via;   /* Flag per indicare se l'arbitro sta aspettando i corridori alla partenza (usato per evitare risvegli spuri). */
  int carbitro_fine;  /* Flag per indicare se l'arbitro sta aspettando i corridori all'arrivo (usato per evitare risvegli spuri). */
} corsa;

/*
 * Funzione per inizializzare la struttura condivisa 'corsa'.
 * Inizializza i semafori e le variabili a valori di default.
 */
void init_corsa(struct corsa_t *in)
{
  sem_init(&in->mutex, 0, 1);      /* Inizializza il mutex con valore 1 (risorsa disponibile). */
  sem_init(&in->scorridore, 0, 0); /* Inizializza il semaforo dei corridori a 0 (nessun via dato). */
  sem_init(&in->sarbitro_via, 0, 0); /* Inizializza il semaforo dell'arbitro per il via a 0 (nessun corridore pronto). */
  sem_init(&in->sarbitro_fine, 0, 0);/* Inizializza il semaforo dell'arbitro per la fine a 0 (nessun corridore arrivato). */

  in->ccorridore = 0;              /* Inizializza il contatore dei corridori alla partenza a 0. */
  in->carbitro_via = 0;           /* Inizializza il flag dell'arbitro per l'attesa alla partenza a 0. */
  in->carbitro_fine = 0;          /* Inizializza il flag dell'arbitro per l'attesa all'arrivo a 0. */
  in->primo = -1;                 /* Inizializza l'identificatore del primo corridore a -1 (nessuno ancora arrivato). */
  in->ultimo = -1;                /* Inizializza l'identificatore dell'ultimo corridore a -1 (nessuno ancora arrivato). */
  in->corridoriarrivati = 0;       /* Inizializza il contatore dei corridori arrivati al traguardo a 0. */
}

/*
 * Funzione chiamata da ogni thread corridore quando arriva alla partenza.
 * Attende il segnale di via dall'arbitro.
 */
void corridore_attendivia(struct corsa_t *corsa, int numerocorridore)
{
  sem_wait(&corsa->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
  corsa->ccorridore++;    /* Incrementa il contatore dei corridori arrivati alla partenza. */

  printf("%2d : Sono arrivato e attendo il via dall'arbitro.\n", numerocorridore);

  /* Se tutti i corridori (N) sono arrivati alla partenza (corsa->ccorridore == N)
   * e l'arbitro sta aspettando i corridori (corsa->carbitro_via), */
  if (corsa->ccorridore == N && corsa->carbitro_via) {
    printf("%2d : sveglio arbitro\n", numerocorridore);
    corsa->carbitro_via--;      /* Decrementa il flag dell'arbitro. */
    sem_post(&corsa->sarbitro_via); /* Segnala all'arbitro che i corridori sono pronti. */
  }
  sem_post(&corsa->mutex); /* Rilascia il mutex. */
  
  sem_wait(&corsa->scorridore); /* Il corridore si blocca in attesa del segnale di via dall'arbitro. */
}

/*
 * Funzione chiamata da ogni thread corridore quando arriva al traguardo.
 * Aggiorna lo stato della corsa (primo e ultimo) e, se tutti sono arrivati, sveglia l'arbitro.
 */
void corridore_arrivo(struct corsa_t *corsa, int numerocorridore)
{
  sem_wait(&corsa->mutex); /* Acquisisce il mutex per accedere alle variabili condivise. */
  printf("%2d : arrivo", numerocorridore);
  /* Se è il primo corridore ad arrivare (corsa->corridoriarrivati == 0), */
  if (corsa->corridoriarrivati == 0) {
    printf("...primo!!!");
    corsa->primo = numerocorridore; /* Imposta il primo corridore arrivato. */
  }

  corsa->corridoriarrivati++; /* Incrementa il contatore dei corridori arrivati al traguardo. */

  /* Se tutti i corridori (N) sono arrivati al traguardo (corsa->corridoriarrivati == N), */
  if (corsa->corridoriarrivati == N) {
    printf("...ultimo :-(");
    corsa->ultimo = numerocorridore; /* Imposta l'ultimo corridore arrivato. */
  }
  printf("\n");

  /* Se l'arbitro sta aspettando la fine della corsa (corsa->carbitro_fine)
   * e tutti i corridori sono arrivati (corsa->corridoriarrivati == N), */
  if (corsa->carbitro_fine && corsa->corridoriarrivati == N) {
    printf("%2d : sveglio arbitro\n", numerocorridore);
    sem_post(&corsa->sarbitro_fine); /* Segnala all'arbitro che tutti i corridori sono arrivati. */
  }
  else
    sem_post(&corsa->mutex); /* Rilascia il mutex. */
}

/*
 * Funzione chiamata dal thread arbitro per attendere che tutti i corridori siano pronti alla partenza.
 */
void arbitro_attendicorridori(struct corsa_t *corsa)
{
  sem_wait(&corsa->mutex); /* Acquisisce il mutex. */
  /* Se il numero di corridori arrivati alla partenza è uguale al numero totale di corridori (N), */
  if (corsa->ccorridore == N) {
    printf("A  : corridori arrivati alla partenza\n");
    sem_post(&corsa->sarbitro_via); /* Segnala il via (anche se nessuno lo sta aspettando, per coerenza). */
  }
  /* Altrimenti (se non tutti i corridori sono arrivati), */
  else {
    printf("A  : aspetto corridori\n");
    corsa->carbitro_via++; /* Incrementa il flag per indicare che l'arbitro sta aspettando. */
  }
  sem_post(&corsa->mutex); /* Rilascia il mutex. */
  sem_wait(&corsa->sarbitro_via); /* L'arbitro si blocca in attesa che tutti i corridori siano pronti. */
}

/*
 * Funzione chiamata dal thread arbitro per dare il via alla corsa.
 * Risveglia tutti i thread corridore in attesa.
 */
void arbitro_via(struct corsa_t *corsa)
{
  int i;

  sem_wait(&corsa->mutex); /* Acquisisce il mutex. */

  printf("A  : via!!!\n");

  /* Cicla per il numero totale di corridori (N). */
  for (i = 0; i < N; i++)
    sem_post(&corsa->scorridore); /* Risveglia un thread corridore bloccato sul semaforo 'scorridore' per ogni iterazione. */
  corsa->ccorridore = 0; /* Resetta il contatore dei corridori alla partenza per eventuali future corse. */

  printf("A  : sveglio corridori!!!\n");
  sem_post(&corsa->mutex); /* Rilascia il mutex. */
}

/*
 * Funzione chiamata dal thread arbitro per attendere l'arrivo di tutti i corridori e annunciare il risultato.
 */
void arbitro_risultato(struct corsa_t *corsa, int *primo, int *ultimo)
{
  sem_wait(&corsa->mutex); /* Acquisisce il mutex. */

  /* Se il numero di corridori arrivati non è uguale al numero totale di corridori (N), */
  if (corsa->corridoriarrivati != N) {
    printf("A  : aspetto corridori al traguardo\n");
    corsa->carbitro_fine++; /* Incrementa il flag per indicare che l'arbitro sta aspettando. */
    sem_post(&corsa->mutex); /* Rilascia il mutex prima di bloccarsi. */
    sem_wait(&corsa->sarbitro_fine); /* L'arbitro si blocca in attesa che tutti i corridori arrivino. */
    sem_wait(&corsa->mutex); /* Riacquisisce il mutex dopo essere stato svegliato. */
    corsa->carbitro_fine--; /* Decrementa il flag. */
  }

  /* Ottiene l'identificatore del primo e dell'ultimo corridore. */
  *primo = corsa->primo;
  *ultimo = corsa->ultimo;

  printf("A  : risultato: %d primo, %d ultimo\n", *primo, *ultimo);

  sem_post(&corsa->mutex); /* Rilascia il mutex. */
}

/*
 * Funzione per simulare un breve periodo di inattività (pausa) per i thread.
 * Utilizza nanosleep per una pausa più precisa.
 */
void pausetta(int quanto)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand() % 100 + 1) * 1000000 + quanto; /* Genera un tempo di attesa casuale in nanosecondi. */
  nanosleep(&t, NULL);                               /* Mette il thread in pausa. */
}

/*
 * Funzione che rappresenta il comportamento di un singolo corridore.
 * Ogni thread corridore esegue questa funzione.
 */
void *corridore(void *arg)
{
  int mionumero = (int)arg; /* Ottiene il numero del corridore passato come argomento. */

  pausetta(100000);                     /* Simula un tempo di preparazione prima di arrivare alla partenza. */
  corridore_attendivia(&corsa, mionumero); /* Arriva alla partenza e attende il via. */
  pausetta(100000);                     /* Simula il tempo impiegato per correre. */
  corridore_arrivo(&corsa, mionumero);    /* Arriva al traguardo. */

  return 0; /* Il thread corridore termina. */
}

/*
 * Funzione che rappresenta il comportamento dell'arbitro.
 * Il thread arbitro esegue questa funzione.
 */
void *arbitro(void *arg)
{
  int primo, ultimo;

  pausetta(100000);                 /* Simula un tempo di preparazione dell'arbitro. */
  arbitro_attendicorridori(&corsa); /* Attende che tutti i corridori siano alla partenza. */
  pausetta(100000);                 /* Simula un breve intervallo prima di dare il via. */
  arbitro_via(&corsa);              /* Dà il via alla corsa. */
  pausetta(100000);                 /* Simula l'attesa dell'arrivo dei corridori. */
  arbitro_risultato(&corsa, &primo, &ultimo); /* Attende l'arrivo di tutti e annuncia il risultato. */

  return 0; /* Il thread arbitro termina. */
}

/*
 * Funzione principale (entry point) del programma.
 * Crea i thread per i corridori e l'arbitro.
 */
int main()
{
  int i = 0;
  pthread_attr_t a; /* Struttura per definire gli attributi dei thread. */
  pthread_t pa;     /* Variabile per memorizzare l'ID del thread creato. */

  /* Inizializza la struttura condivisa per la corsa. */
  init_corsa(&corsa);

  /* Inizializza il generatore di numeri casuali, usato nella funzione pausetta.
   * L'uso di un seme fisso (55) rende la sequenza di numeri casuali deterministica per ogni esecuzione. */
  srand(55);

  pthread_attr_init(&a);                     /* Inizializza la struttura degli attributi del thread. */
  pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED); /* Imposta lo stato del thread come 'detached',
                                                              in modo che le risorse del thread vengano rilasciate
                                                              automaticamente al termine dell'esecuzione, senza
                                                              necessità di una pthread_join. */

  /* Cicla per il numero totale di corridori (N). */
  for (i = 0; i < N; i++)
    pthread_create(&pa, &a, corridore, (void *)(i)); /* Crea un nuovo thread che esegue la funzione 'corridore',
                                                      passando l'indice 'i' come argomento (numero del corridore). */

  /* Crea un thread per l'arbitro. */
  pthread_create(&pa, &a, arbitro, NULL); /* Crea un nuovo thread che esegue la funzione 'arbitro'. */

  pthread_attr_destroy(&a); /* Libera le risorse associate alla struttura degli attributi del thread. */

  sleep(1); /* Il thread principale dorme per 1 secondo per dare il tempo agli altri thread di eseguire.
             * In un'applicazione reale, si userebbero meccanismi di sincronizzazione più robusti
             * per attendere la terminazione dei thread. */

  return 0; /* Il programma termina. */
}