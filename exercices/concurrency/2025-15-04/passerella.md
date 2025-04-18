# La passerella - 15/04/2025

Una città di mare ha una passerella che attraversa un canale vicino ad un porto. La passerella, gestita da un guardiano, è un ponte levatoio che serve per far passare eventuali barche nel canale sottostante. La passerella è un ponte pedonale, dove i pedoni possono passare da una parte all'altra della città. La passerella è inizialmente chiusa, e viene aperta dal guardiano al suo arrivo. Il guardiano controlla costantemente il canale: quando si avvicina una barca troppo alta, il guardiano blocca gli ingressi, fa uscire di corsa tutti i pedoni, alza la passerella e fa passare la barca. Quando la barca è passata, il guardiano abbassa la passerella ed i pedoni possono passare di nuovo. Le barche hanno la precedenza sui pedoni.

Se il pedone ha fretta (parametro hofretta=1) e la passerella è chiusa, la funzione pedone_entra_passerella ritorna 0, altrimenti 1. Se il pedone non ha fretta (parametro hofretta=0), allora la funzione pedone_entra_passerella è bloccante.

Il sistema è modellato tramite un thread per ogni pedone (il numero di pedoni non è specificato), ed un thread per il guardiano.
```C
struct passerella_t { } passerella;
void init_passerella (struct passerella_t *passerella);
void pedone_entro_passerella (struct passerella_t *passerella, int hofretta); void pedone_esco_passerella (struct passerella_t *passerella);
void guardiano_abbasso_passerella (struct passerella_t *passerella);
void guardiano_alzo_passerella (struct passerella_t *passerella);

void *pedone (void *arg)
{
    while (1) {
        <arrivo alla passerella>
        if (pedone_entro_passerella(&passerella, <ho fretta>)) {    
            <attraversa passerella (guardo il panorama)> 
            pedone_esco_passerella(&passerella);
            <vado via>
        } else <cambio strada>
    }
    return 0;
}

void *guardiano (void *arg)
{
    while(1) {
        guardiano_abbasso_passerella(&passerella);
        <attendi barca>
        guardiano_alzo_passerella(&passerella); 
        <lascio passare la barca>
    }
}
```
Modellare il sistema in linguaggio C con la libreria pthread nel seguente modo utilizzando: [] semafori [] mutex + variabili condition

Scrivere il contenuto della struct passerella e le funzioni init_passerella pedone_entro_passerella, pedone_esco_passerella, guardiano_abbasso_passerella guardiano_alzo_passerella,