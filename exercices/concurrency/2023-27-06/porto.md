# Il porto - 27/06/2023

Una città ha un porto che **puo' contenere al massimo N posti barca**, e da una imboccatura in cui possono passare al piu' due barche contemporaneamente. Hanno precedenza le barche che escono dal porto.

Il sistema è modellato tramite un thread per ogni barca. Il numero di barche non è specificato. Non è importante l'ordine di accodamento delle barche.
```C
struct porto_t{...} porto;
void init_porto(struct porto_t *porto);
void entrata_richiesta(struct porto_t *porto); void entrata_ok(struct porto_t *porto);
void uscita_richiesta(struct porto_t *porto);
void uscita_ok(struct porto_t *porto);

void *barca(void *arg){
    <arriva all imboccatura del porto> 
    entrata_richiesta(&porto); // BLOCCANTE
    <transito imboccatura>
    entrata_ok(&porto);        // NON BLOCCANTE
    <staziona dentro il porto> 
    uscita_richiesta(&porto);  // BLOCCANTE
    <transito imboccatura> 
    uscita_ok(&porto);         // NON BLOCCANTE
    <vai ad altro porto>
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Definire la struttura porto_t, e scrivere le funzioni: init_porto, entrata_richiesta, entrata_ok, uscita_richiesta, uscita_ok.