# Il bagno - 01/07/2025

Un bar ha un bagno utilizzato da uomini e donne. Nel bagno non possono mai trovarsi contemporaneamente persone di entrambe i sessi.
Gli uomini utilizzano il bagno da soli, mentre le donne lo utilizzano da sole o, quando possibile, in coppia. Se una donna entra da sola non se ne può aggiungere un'altra in un secondo momento.
La precedenza per l'utilizzo del bagno è data generalmente alle donne, con la seguente eccezione: se c'è in coda un uomo, ogni N donne deve passare almeno un uomo.
Non è importante l'ordine di accodamento di uomini e donne (a differenza di quello che succede nella vita reale).

Uomini e donne sono modellati da thread. Non è specificato il numero di uomini e donne nel sistema.

```c
struct bagno_t { ... } bagno;
void init_bagno(struct bagno_t *s);
void uomo_entra(struct bagno_t *bagno);
void uomo_esci(struct bagno_t *bagno);
void donna_entra(struct bagno_t *bagno);
void donna_esci(struct bagno_t *bagno);
```

```c
void *uomo(void *arg)
{
        uomo_entra(&bagno);
        <...>
        uomo_esci(&bagno);
}
```

```c
void *donna(void *arg)
{
        donna_entra(&bagno);
        <...>
        donna_esci(&bagno);
}
```

Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Scrivere il contenuto della struct bagno e le funzioni init_bagno, uomo_entra, uomo_esci, donna_entra, donna_esci.

Domanda obbligatoria: Il sistema soffre di problemi di starvation?