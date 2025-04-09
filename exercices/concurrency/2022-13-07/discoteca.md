# La discoteca - 13/07/2022

Una discoteca ha la seguente organizzazione per far entrare i clienti nella sala da ballo:

- esiste una cassiera che emette UN BIGLIETTO ALLA VOLTA. Emissione del biglietto e riscossione
partono assieme, ma alla fine sono non bloccanti.

- per evitare confusione alla cassa esiste una porta che fa attendere i clienti fuori dalla discoteca. La porta all'inizio del sistema è aperta. All'arrivo del primo cliente la porta si chiude dietro di lui. 

- Quando l'ULTIMO cliente in coda alla cassa ha ritirato il biglietto (ovvero alla fine di cliente_esco_coda), se ci sono dei clienti in attesa fuori, si apre la porta, si fanno entrare (a vostra scelta se farli entrare tutti o solo parte di essi), dopo di che la porta si richiude di nuovo. Se non ci sono clienti in attesa fuori la porta viene aperta.

Il sistema è modellato tramite un thread per la cassiera, ed uno per ogni cliente. Il numero di clienti non è specificato. Non è importante l'ordine di accodamento dei clienti.
```C
struct discoteca_t { ... } discoteca;
void init_discoteca(struct discoteca_t *d);
void cliente_coda_fuori(struct discoteca_t *d);
void cliente_coda_dentro(struct discoteca_t *d);
void cliente_esco_coda(struct discoteca_t *d); 
void cassiera_attesa_cliente(struct discoteca_t *d);
void cassiera_cliente_servito(struct discoteca_t *d);

void *cliente(void *arg)
{
    <arriva all ingresso della discoteca> 
    cliente_coda_fuori(&discoteca);    // BLOCCANTE se la porta è chiusa
    <vado a fare biglietto> 
    cliente_coda_dentro(&discoteca);   // BLOCCANTE se la cassa è occupata
    <pagamento e ritiro biglietto> 
    cliente_esco_coda(&discoteca);     // NON BLOCCANTE
    <si balla!>
}

void *cassiera(void *arg)
{
    while (<la discoteca è aperta>) {
        cassiera_attesa_cliente(&discoteca); // BLOCCANTE
        <emissione biglietto e riscossione di un SINGOLO cliente>
        cassiera_cliente_servito(&discoteca); // NON BLOCCANTE
        <metto a posto i $$$>
    }
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [ ] semafori [ ] mutex + variabili condition 

Definire la struttura discoteca_t, e scrivere le funzioni: init_discoteca, cliente_coda_fuori, cliente_coda_dentro, cliente_esco_coda, cassiera_attesa_cliente, cassiera_cliente_servito