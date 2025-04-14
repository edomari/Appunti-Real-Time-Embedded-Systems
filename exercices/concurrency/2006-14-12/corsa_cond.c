#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define N 6

/* la struttura condivisa */
struct corsa_t {
    pthread_mutex_t mutex;        /* Mutex per proteggere l'accesso alle variabili condivise della corsa. */
    pthread_cond_t scorridore;   /* Variabile condizione per segnalare ai corridori il via della corsa. */
    int ccorridore;             /* Contatore del numero di corridori arrivati alla partenza. */

    int corridoriarrivati;      /* Contatore del numero di corridori arrivati al traguardo. */
    int primo, ultimo;          /* Identificatori (numero) del primo e dell'ultimo corridore arrivato. */

    pthread_cond_t sarbitro_via;  /* Variabile condizione per segnalare all'arbitro che tutti i corridori sono pronti per la partenza. */
    pthread_cond_t sarbitro_fine; /* Variabile condizione per segnalare all'arbitro che tutti i corridori sono arrivati al traguardo. */
    int carbitro_via;           /* Flag per indicare se l'arbitro sta aspettando i corridori alla partenza. */
    int carbitro_fine;          /* Flag per indicare se l'arbitro sta aspettando i corridori all'arrivo. */

  } corsa;

  /*
   * Funzione per inizializzare la struttura condivisa 'corsa'.
   * Inizializza il mutex e le variabili condizione.
   */
  void init_corsa(struct corsa_t *in){
    pthread_mutexattr_t m_attr; /* Struttura per definire gli attributi del mutex. */
    pthread_condattr_t c_attr;  /* Struttura per definire gli attributi della variabile condizione. */

    pthread_mutexattr_init(&m_attr); /* Inizializza gli attributi del mutex con i valori di default. */
    pthread_condattr_init(&c_attr);  /* Inizializza gli attributi della variabile condizione con i valori di default. */

    pthread_mutex_init(&in->mutex, &m_attr); /* Inizializza il mutex 'in->mutex' con gli attributi specificati. */

    pthread_cond_init(&in->scorridore, &c_attr);   /* Inizializza la variabile condizione 'in->scorridore'. */
    pthread_cond_init(&in->sarbitro_via, &c_attr); /* Inizializza la variabile condizione 'in->sarbitro_via'. */
    pthread_cond_init(&in->sarbitro_fine, &c_attr);/* Inizializza la variabile condizione 'in->sarbitro_fine'. */

    in->ccorridore = in->carbitro_via = in->carbitro_fine = 0; /* Inizializza i contatori e i flag a 0. */
    in->primo = in->ultimo = -1;                             /* Inizializza gli identificatori del primo e dell'ultimo a -1. */
  }

  /*
   * Funzione chiamata da ogni thread corridore quando arriva alla partenza.
   * Attende il segnale di via dall'arbitro usando una variabile condizione.
   */
  void corridore_attendivia(struct corsa_t *corsa, int numerocorridore)
  {
    pthread_mutex_lock(&corsa->mutex); /* Acquisisce il mutex per proteggere le variabili condivise. */
    corsa->ccorridore++;              /* Incrementa il contatore dei corridori arrivati alla partenza. */

    printf("%2d : attendivia\n", numerocorridore);

    /* Se tutti i corridori (N) sono arrivati alla partenza (corsa->ccorridore == N)
     * e l'arbitro sta aspettando il via (corsa->carbitro_via è true), */
    if (corsa->ccorridore == N && corsa->carbitro_via) {
      printf("%2d : sveglio arbitro\n", numerocorridore);
      pthread_cond_signal(&corsa->sarbitro_via); /* Segnala all'arbitro che tutti i corridori sono pronti. */
    }

    /* Il corridore si mette in attesa sulla variabile condizione 'corsa->scorridore'.
     * Il mutex 'corsa->mutex' viene rilasciato durante l'attesa e riacquisito al risveglio. */
    pthread_cond_wait(&corsa->scorridore, &corsa->mutex);

    printf("%2d : svegliato!\n", numerocorridore);

    pthread_mutex_unlock(&corsa->mutex); /* Rilascia il mutex dopo essere stato svegliato. */
  }


  /*
   * Funzione chiamata da ogni thread corridore quando arriva al traguardo.
   * Aggiorna lo stato della corsa (primo e ultimo) e, se tutti sono arrivati, sveglia l'arbitro.
   */
  void corridore_arrivo(struct corsa_t *corsa, int numerocorridore)
  {
    pthread_mutex_lock(&corsa->mutex); /* Acquisisce il mutex per proteggere le variabili condivise. */
    printf("%2d : arrivo", numerocorridore);
    /* Se è il primo corridore ad arrivare (corsa->corridoriarrivati == 0), */
    if (corsa->corridoriarrivati == 0) {
      printf("...primo!!!");
      corsa->primo = numerocorridore; /* Imposta l'identificatore del primo corridore. */
    }

    corsa->corridoriarrivati++; /* Incrementa il contatore dei corridori arrivati. */

    /* Se tutti i corridori (N) sono arrivati al traguardo (corsa->corridoriarrivati == N), */
    if (corsa->corridoriarrivati == N) {
      printf("...ultimo :-(");
      corsa->ultimo = numerocorridore; /* Imposta l'identificatore dell'ultimo corridore. */
    }
    printf("\n");

    /* Se l'arbitro sta aspettando la fine della corsa (corsa->carbitro_fine è true)
     * e tutti i corridori sono arrivati (corsa->corridoriarrivati == N), */
    if (corsa->carbitro_fine && corsa->corridoriarrivati == N) {
      printf("%2d : sveglio arbitro\n", numerocorridore);
      pthread_cond_signal(&corsa->sarbitro_fine); /* Segnala all'arbitro che la corsa è finita. */
    }

    pthread_mutex_unlock(&corsa->mutex); /* Rilascia il mutex. */
  }

  /*
   * Funzione chiamata dal thread arbitro per attendere che tutti i corridori siano pronti alla partenza.
   */
  void arbitro_attendicorridori(struct corsa_t *corsa)
  {
    pthread_mutex_lock(&corsa->mutex); /* Acquisisce il mutex. */
    /* Mentre il numero di corridori arrivati alla partenza (corsa->ccorridore) è diverso dal numero totale (N), */
    while (corsa->ccorridore != N) {
      printf("A  : aspetto corridori\n");
      corsa->carbitro_via++; /* Indica che l'arbitro sta aspettando. */
      /* L'arbitro si mette in attesa sulla variabile condizione 'corsa->sarbitro_via'.
       * Il mutex 'corsa->mutex' viene rilasciato durante l'attesa e riacquisito al risveglio. */
      pthread_cond_wait(&corsa->sarbitro_via, &corsa->mutex);
      corsa->carbitro_via--; /* Decrementa il flag dopo essere stato svegliato. */
    }

    printf("A  : corridori arrivati alla partenza\n");

    pthread_mutex_unlock(&corsa->mutex); /* Rilascia il mutex. */
  }

  /*
   * Funzione chiamata dal thread arbitro per dare il via alla corsa.
   * Risveglia tutti i thread corridore in attesa.
   */
  void arbitro_via(struct corsa_t *corsa)
  {
    pthread_mutex_lock(&corsa->mutex); /* Acquisisce il mutex. */

    printf("A  : via!!!\n");

    /* Risveglia tutti i thread in attesa sulla variabile condizione 'corsa->scorridore'. */
    pthread_cond_broadcast(&corsa->scorridore);
    corsa->ccorridore = 0; /* Resetta il contatore dei corridori alla partenza per eventuali future corse. */

    printf("A  : sveglio corridori!!!\n");

    pthread_mutex_unlock(&corsa->mutex); /* Rilascia il mutex. */
  }

  /*
   * Funzione chiamata dal thread arbitro per attendere l'arrivo di tutti i corridori e annunciare il risultato.
   */
  void arbitro_risultato(struct corsa_t *corsa, int *primo, int *ultimo)
  {
    pthread_mutex_lock(&corsa->mutex); /* Acquisisce il mutex. */

    /* Mentre il numero di corridori arrivati (corsa->corridoriarrivati) è diverso dal numero totale (N), */
    while (corsa->corridoriarrivati != N) {
      printf("A  : aspetto corridori al traguardo\n");
      corsa->carbitro_fine++; /* Indica che l'arbitro sta aspettando. */
      /* L'arbitro si mette in attesa sulla variabile condizione 'corsa->sarbitro_fine'.
       * Il mutex 'corsa->mutex' viene rilasciato durante l'attesa e riacquisito al risveglio. */
      pthread_cond_wait(&corsa->sarbitro_fine, &corsa->mutex);
      corsa->carbitro_fine--; /* Decrementa il flag dopo essere stato svegliato. */
    }

    *primo = corsa->primo;   /* Ottiene l'identificatore del primo corridore. */
    *ultimo = corsa->ultimo; /* Ottiene l'identificatore dell'ultimo corridore. */

    printf("A  : risultato: %d primo, %d ultimo\n", *primo, *ultimo);

    pthread_mutex_unlock(&corsa->mutex); /* Rilascia il mutex. */
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

  /* la creazione dei thread */

  int main()
  {
    int i=0;
    pthread_attr_t a; /* Struttura per definire gli attributi dei thread. */
    pthread_t pa;     /* Variabile per memorizzare l'ID del thread creato. */

    /* inizializzo il mio sistema */
    init_corsa(&corsa);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(55);

    pthread_attr_init(&a);                     /* Inizializza la struttura degli attributi del thread. */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED); /* Imposta lo stato del thread come 'detached',
                                                              in modo che le risorse del thread vengano rilasciate
                                                              automaticamente al termine dell'esecuzione. */

    /* Cicla per il numero totale di corridori (N). */
    for (i=0; i<N; i++)
      pthread_create(&pa, &a, corridore, (void *)(i)); /* Crea un nuovo thread che esegue la funzione 'corridore',
                                                        passando l'indice 'i' come argomento (numero del corridore). */

    /* Crea un thread per l'arbitro. */
    pthread_create(&pa, &a, arbitro, NULL); /* Crea un nuovo thread che esegue la funzione 'arbitro'. */

    pthread_attr_destroy(&a); /* Libera le risorse associate alla struttura degli attributi del thread. */

    sleep(1); /* Il thread principale dorme per 1 secondo per dare il tempo agli altri thread di eseguire. */

    return 0; /* Il programma termina. */
  }