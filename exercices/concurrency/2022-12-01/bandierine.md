# Le bandierine 12/01/2022

Il sistema modella un turno del gioco delle bandierine. Ci sono due giocatori che al "via!" devono tentare di prendere la bandierina tenuta dal giudice. Il primo che riesce a prendere la bandierina deve ritornare alla base, mentre l'altro deve cercare di raggiungerlo.

Il sistema è modellato da tre thread: uno per il giudice, ed uno per ogni giocatore.
```C
struct bandierine t... bandierine;
void init bandierine(struct bandierine t *b)
void via(struct bandierine t *b);
void attendi il via(struct bandierine t *b, int n):
int bandierina presa(struct bandierine_t *b. int n); 
int sono salvo(struct bandierine t *b, int n); 
int ti_ho_preso(struct bandierine_t b, int n); 
int risultato gioco(struct bandierine_t *b):
void *giocatore(void *arg)
{
    int numerogiocatore (int)arg:
    attendi_il_via(&bandierine. numerogiocatore); 
    <corri e tenta di prendere la bandierina>>
    if (bandierina presa(&bandierine, numerogiocatore)) {
        <corri alla base>
        if (sono salvo(&bandierine, numerogiocatore)) printf("salvo!");
    }
    else{
        <cerca di acchiappare l altro giocatore>
        if (ti_ho_presot(&bandierine, mumerogiocatore)) printf("ti ho preso!");
    }
    return 0;
}
void *giudice(void *arg){
    attendi_giocatori(&bandierine);
    <pronti, attenti....
    via(&bandierine);
    printf("il vincitore è:"%d", risultatogioco(&bandierine)");
    return 0;
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: []semafori [] mutex + variabili condition
- _attendi_ giocatori attende che entrambe i giocatori siano pronti in attesa prima di dare il via
- _via_ da il via al gioco sbloccando i giocatori in attesa
- _attendi_il_via_ è sempre bloccante
- _bandierina_presa_ prende la bandierina (se non è già stata presa), e ritorna 1 se il giocatore ha preso la bandierina. O altrimenti
- _sono_salvo_ ritorna 1 se il giocatore non è stato ancora preso, 0 altrimenti
- _ti_ho_preso_ ritorna 1 se il giocatore non si è ancora salvato, 0 altrimenti
- _risultato_gioco_ ritorna il vincitore non appena il risultato della partita è certo