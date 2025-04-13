#define N 6

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/* la struttura condivisa */
struct corsa_t {
  sem_t mutex;

  sem_t scorridore;
  int ccorridore;

  int corridoriarrivati;
  int primo, ultimo;

  sem_t sarbitro_via, sarbitro_fine;
  int carbitro_via, carbitro_fine;

} corsa;

void init_corsa(struct corsa_t *in)
{
  sem_init(&in->mutex,0,1);
  sem_init(&in->scorridore,0,0);
  sem_init(&in->sarbitro_via,0,0);
  sem_init(&in->sarbitro_fine,0,0);

  in->ccorridore = in->carbitro_via = in->carbitro_fine = 0;
  in->primo = in->ultimo = -1;
}

void corridore_attendivia(struct corsa_t *corsa, int numerocorridore)
{
  sem_wait(&corsa->mutex);
  corsa->ccorridore++;

  printf("%2d : Sono arrivato e attendo il via dall'arbitro.\n", numerocorridore);

  if (corsa->ccorridore == N && corsa->carbitro_via) {
    printf("%2d : sveglio arbitro\n", numerocorridore);
    corsa->carbitro_via--;
    sem_post(&corsa->sarbitro_via);
  }
  sem_post(&corsa->mutex);

  sem_wait(&corsa->scorridore);
}

void corridore_arrivo(struct corsa_t *corsa, int numerocorridore)
{
  sem_wait(&corsa->mutex);
  printf("%2d : arrivo", numerocorridore);
  if (corsa->corridoriarrivati == 0) {
    printf("...primo!!!");
    corsa->primo = numerocorridore;
  }

  corsa->corridoriarrivati++;

  if (corsa->corridoriarrivati == N) {
    printf("...ultimo :-(");
    corsa->ultimo = numerocorridore;
  }
  printf("\n");

  if (corsa->carbitro_fine && corsa->corridoriarrivati == N) {
    printf("%2d : sveglio arbitro\n", numerocorridore);
    sem_post(&corsa->sarbitro_fine);
  }
  else 
    sem_post(&corsa->mutex);
}

void arbitro_attendicorridori(struct corsa_t *corsa)
{
  sem_wait(&corsa->mutex);
  if (corsa->ccorridore == N) {
    printf("A  : corridori arrivati alla partenza\n");
    sem_post(&corsa->sarbitro_via);
  }
  else {
    printf("A  : aspetto corridori\n");
    corsa->carbitro_via++;
  }
  sem_post(&corsa->mutex);
  sem_wait(&corsa->sarbitro_via);
}

void arbitro_via(struct corsa_t *corsa)
{
  int i;

  sem_wait(&corsa->mutex);

  printf("A  : via!!!\n");

  for (i=0; i<N; i++)
    sem_post(&corsa->scorridore);
  corsa->ccorridore = 0;

  printf("A  : sveglio corridori!!!\n");
  sem_post(&corsa->mutex);
}

void arbitro_risultato(struct corsa_t *corsa, int *primo, int *ultimo)
{
  sem_wait(&corsa->mutex);

  if (corsa->corridoriarrivati != N) {
    printf("A  : aspetto corridori al traguardo\n");
    corsa->carbitro_fine++;
    sem_post(&corsa->mutex);
    sem_wait(&corsa->sarbitro_fine);
    corsa->carbitro_fine--;
  }

  *primo = corsa->primo;
  *ultimo = corsa->ultimo;

  printf("A  : risultato: %d primo, %d ultimo\n", *primo, *ultimo);

  sem_post(&corsa->mutex);
}

void pausetta(int quanto)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%100+1)*1000000 + quanto;
  nanosleep(&t,NULL);
}

void *corridore(void *arg)
{
  int mionumero = (int)arg;

  pausetta(100000);
  corridore_attendivia(&corsa, mionumero);
  pausetta(100000);
  corridore_arrivo(&corsa, mionumero);

  return 0;
}

void *arbitro(void *arg)
{
  int primo, ultimo;

  pausetta(100000);
  arbitro_attendicorridori(&corsa);
  pausetta(100000);
  arbitro_via(&corsa);
  pausetta(100000);
  arbitro_risultato(&corsa, &primo, &ultimo);

  return 0;
}

/* la creazione dei thread */

int main()
{
  int i=0;
  pthread_attr_t a;
  pthread_t pa;
  
  /* inizializzo il mio sistema */
  init_corsa(&corsa);

  /* inizializzo i numeri casuali, usati nella funzione pausetta */
  srand(55);

  pthread_attr_init(&a);
  pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

  for (i=0; i<N; i++)
    pthread_create(&pa, &a, corridore, (void *)(i));
  
  pthread_create(&pa, &a, arbitro, NULL);

  pthread_attr_destroy(&a);

  sleep(1);

  return 0;
}