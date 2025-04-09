# Ufficio riparazioni - 21 Febbraio 2019

Un ufficio riparazioni dispone di N operai ciascuno dei quali può effettuare un tipo diverso di riparazione (numerare le riparazioni da 0 a N-1). L'ufficio può contenere (gestire) **massimo un cliente alla volta**. Eventuali ulteriori clienti devono attendere all'esterno (non è importante l'ordine di accodamento). 

Comportamenti di clienti e operai:
- Il singolo cliente arriva in ufficio, richiede un servizio, e si mette in attesa. Il cliente può lasciare l'ufficio **non appena la riparazione è stata effettuata**.
- Il singolo operaio ha il seguente comportamento: attende l'arrivo di un nuovo cliente; verifica se è lui a dover gestire la richiesta: se si, effettua la riparazione e fa una pausa.

Il sistema è modellato tramite un thread per ogni cliente (il numero di clienti **non** è specificato), ed un thread per ogni operaio.
```C
struct officina_t (...) o;
void init_officina(struct officina_t *officina);
void cliente_arrivo(struct officina_t *officina, int riparazione); // il parametro è la riparazione richiesta 
void cliente_attesafineservizio(struct officina_t *officina);
void operaio_attesacliente(struct officina_t *officina, int riparazione);
void operaio_fineservizio(struct officina_t *officina);
void *cliente(void *arg)
{
    <arriva nell ufficio per effettuare una riparazione r> 
    cliente_arrivo(&o, r);              // può essere bloccante
    <leggo un giornale>
    cliente_attesa_fineservizio(&o);    // può essere bloccante
    <torno a casa>
    return 0;
}

void *operaio(void *arg)
{
    <r indica la riparazione che l operaio è in grado di fare>
    for (;;) {
        operaio_attesacliente(&o, r);   // può essere bloccante
        <riparo>
        operaio_fineservizio(&o);       // NON bloccante
        <pausa>
    }
    return 0;
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Nota: La funzione operaio_attesacliente riceve come parametro la riparazione che il particolare operaio è in grado di fare. La funzione fa bloccare l'operaio fino all'arrivo di un cliente che richieda la riparazione r. Le riparazioni vengono fornite da un singolo operaio (N tipi di riparazioni = N operai)