#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

/* la struttura condivisa */
struct bandierine_t{
	pthread_mutex_t mutex;
	pthread_cond_t attesa, via, fine;

	int n_giocatori;
	int giocatore_con_bandiera;
	int vincitore;
	bool salvo;

} bandierine;

void pausetta(void){
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand() % + 1) * 1000000;
	nanosleep(&t, NULL);
}

void init_bandierine(struct bandierine_t *b){
	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;

	pthread_mutexattr_init(&mutex_attr);
	pthread_condattr_init(&cond_attr);

	pthread_mutex_init(&b->mutex, &mutex_attr);
	pthread_cond_init(&b->attesa, &cond_attr);
	pthread_cond_init(&b->via, &cond_attr);
	pthread_cond_init(&b->fine, &cond_attr);

	pthread_mutexattr_destroy(&mutex_attr);
	pthread_condattr_destroy(&cond_attr);

	b->n_giocatori = 0;
	b->giocatore_con_bandiera = b->vincitore = -1;
	b->salvo = false;
}

void via(struct bandierine_t *b){
	pthread_cond_broadcast(&b->via);
}

void attendi_il_via(struct bandierine_t *b, int n){
	pthread_mutex_lock(&b->mutex);

	while (b->n_giocatori != 2) {
		b->n_giocatori++;
		if (b->n_giocatori == 2) {
			pthread_cond_signal(&b->attesa);
		}
		pthread_cond_wait(&b->via, &b->mutex);
	}

	pthread_mutex_unlock(&b->mutex);
}

int bandierina_presa(struct bandierine_t *b, int n){
	int res = 0;
	pthread_mutex_lock(&b->mutex);

	if (b->giocatore_con_bandiera == -1) {
		b->giocatore_con_bandiera = n;
		res = 1;
	}

	pthread_mutex_unlock(&b->mutex);

	return res;
}

int sono_salvo(struct bandierine_t *b, int n){
	int res = 0;

	pthread_mutex_lock(&b->mutex);

	if (b->salvo == false) {
		res = 1;
		b->vincitore = n;
		b->salvo = true;
	}

	pthread_cond_signal(&b->fine); // segnalo che il gioco è finito

	pthread_mutex_unlock(&b->mutex);

	return res;
}

int ti_ho_preso(struct bandierine_t *b, int n){
	int res = 0;

	pthread_mutex_lock(&b->mutex);

	if (b->salvo == false) {
		res = 1;
		b->vincitore = n;
		b->salvo = true;
	}
	pthread_cond_signal(&b->fine); // segnalo che il gioco è finito

	pthread_mutex_unlock(&b->mutex);

	return res;
}

int risultato_gioco(struct bandierine_t *b){
	int res;
	pthread_mutex_lock(&b->mutex);

	while (b->vincitore == -1) {
		printf("Giudice: aspetto il vincitore\n");
		pthread_cond_wait(&b->fine, &b->mutex);
	}

	res = b->vincitore;

	pthread_mutex_unlock(&b->mutex);

	return res;
}

void attendi_giocatori(struct bandierine_t *b){
	pthread_mutex_lock(&b->mutex);
	while (b->n_giocatori != 2) {
		pthread_cond_wait(&b->attesa, &b->mutex);
	}
	pthread_mutex_unlock(&b->mutex);
}

void *giocatore(void *arg){
	int numero_giocatore = (int) arg;
	attendi_il_via(&bandierine, numero_giocatore);
	if (bandierina_presa(&bandierine, numero_giocatore)){
		printf("P%d: Ho preso la bandierina!\n", numero_giocatore);
		pausetta();
		if (sono_salvo(&bandierine, numero_giocatore))
			printf("P%d: Sono Salvo!\n", numero_giocatore);
	} else {
		printf("P%d: Non sono riuscito a prendere la bandierina!\n", numero_giocatore);
		pausetta();
		if (ti_ho_preso(&bandierine, numero_giocatore))
			printf("P%d: Ti ho preso!\n", numero_giocatore);
	}

	pthread_exit(0);
}

void *giudice(void *arg){
	attendi_giocatori(&bandierine);
	via(&bandierine);
	printf("Giudice: il vincitore è %d\n", risultato_gioco(&bandierine));
	pthread_exit(0);
}

int main(){
	pthread_t t_giocatore1, t_giocatore2, t_giudice;
	pthread_attr_t p_attr;

	pthread_attr_init(&p_attr);

	init_bandierine(&bandierine);

	pthread_create(&t_giocatore1, &p_attr, giocatore, (void *) 1);
	pthread_create(&t_giocatore2, &p_attr, giocatore, (void *) 2);
	pthread_create(&t_giudice, &p_attr, giudice, NULL);

	pthread_join(t_giocatore1, NULL);
	pthread_join(t_giocatore2, NULL);
	pthread_join(t_giudice, NULL);

	pthread_attr_destroy(&p_attr);

	return 0;
}