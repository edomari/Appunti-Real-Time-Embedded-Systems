# La rotonda dell'UVA - 19/04/2024

Uscendo dal Dipartimento, N auto (numerate da 0 a N-1) prendono la rotonda di accesso alla tangenziale. La rotonda è costituita da S sezioni, numerate da 0 a S-1. La sezione 0 confina con le sezioni S-1 ed 1, la sezione 1 confina con le sezioni 0 e 2, e cosi' via. Ciascuna delle sezioni della rotonda ha una entrata, una uscita, e lo spazio per contenere una sola auto.

Il sistema è modellato con un thread per ogni auto. Le auto entrano da una sezione di ingresso, effettuano il giro della rotonda in ordine di sezione crescente (esempio: S-2, S-1, 0, 1, 2, ...), ed escono dalla sezione di uscita. Le Prima di occupare una sezione, un'auto deve attendere che tale sezione sia libera.

L'auto richiama la funzione entra richiedendo l'ingresso in rotonda ad una particolare entrata.

Dopo di che, l'auto richiama la funzione sonoarrivato per verificare se deve passare alla successiva sezione oppure deve uscire. La funzione ritorna 1 se l'auto deve percorrere una ulteriore sezione, 0, se la sezione corrente è uguale alla destinazione. La prima sezione verificata da un'auto è la sezione di entrata. Quando la funzione ritorna 1 l'auto effettua in quel momento il passaggio nella nuova sezione.

All'uscita dalla rotonda, l'auto richiama la funzione esci per uscire definitivamente dalla rotonda.
Non sono importanti eventuali ordini di accodamento. Non è importante considerare i problemi di starvation o deadlock (vedi domande finali).
```C
struct rotonda_t { ... } rotonda;
void init_rotonda (struct rotonda_t *s);
void entra(struct rotonda_t *r, int numeroauto, int sezione);
int sonoarrivato (struct rotonda_t *r, int numeroauto, int destinazione); 
void esci(struct rotonda_t *r, int numeroauto);
void auto(void *arg){
    entra(&r, numeroauto, sezionediingresso);
    do {
        <percorri la sezione corrente della rotonda>
    } while (sonoarrivato (&r, numeroauto, destinazione)); esci(&r, numeroauto);
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Scrivere il contenuto della struct rotonda e le funzioni init_rotonda, entra, sonoarrivato, esci.

Domande facoltative

1. La soluzione implementata ha problemi di deadlock? Motivare la risposta indicando come sia possibile evitare tale fenomeno (se presente), o come è stato evitato (se non presente).
2. La soluzione implementata ha problemi di starvation? Motivare la risposta indicando come sia possibile evitare tale fenomeno (se presente), o come è stato evitato (se non presente).