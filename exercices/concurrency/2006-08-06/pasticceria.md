# La pasticceria - 8/6/2006

Una pasticceria produce e vende al dettaglio delle torte. La pasticceria è gestita da un cuoco, che cucina le torte con arte, e da un commesso, che prende le torte prodotte, le incarta e le vende al dettaglio. Il pasticcere evita di iniziare a produrre torte se in pasticceria ci sono piu' di N torte invendute. I clienti acquistano una torta alla volta. La vendita di una torta da parte del commesso coincide con l'acquisto da parte del cliente.

Il sistema è modellato tramite un thread per il cuoco, uno per il commesso, ed uno per ogni cliente. Il numero di clienti non è specificato, e non è importante il loro ordine di accodamento.

```C
struct pasticceria_t { ... } pasticceria;
void init_pasticceria(struct pasticceria_t *p); 
void cuoco_inizio_torta(struct pasticceria_t *p); 
void cuoco_fine_torta(struct pasticceria_t *p); 
void commesso_prendo_torta(struct pasticceria_t *p); 
void commesso_vendo_torta(struct pasticceria_t *p); 
void cliente_acquisto(struct pasticceria_t *p);
void *cuoco(void *arg)
{
    while(1) {
        cuoco_inizio_torta(&pasticceria); // 1 torta; può essere bloccante
        <preparo torta> 
        cuoco_fine_torta(&pasticceria);   // 1 torta; non bloccante
    }
}

void *commesso(void *arg)
{
    while(1) {
        commesso_prendo_torta(&pasticceria);            // 1 torta; bloccante
        <incarto la torta in una confezione colorata> 
        commesso_vendo_torta(&pasticceria);             // 1 torta; bloccante
    }
}
void *un_cliente(void *arg)
{
    while (1) {
        <vado in pasticceria per comprare una torta sopraffina>
        cliente_acquisto(&pasticceria);     // 1 torta; bloccante
        <torno a casa a mangiare la torta>
    }
}

```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Scrivere il contenuto della struct pasticceria e le funzioni cuoco_inizio_torta, cuoco_fine_torta, commesso_prendo_torta, commesso_vendo_torta, e cliente_acquisto.
