#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lecteur_redacteur.h"

void initialiser_fifo(fifo_t *file){
	pthread_cond_init(&file->cond, NULL);
	file->next = NULL;
}

void enfiler(lecteur_redacteur_t *lr, fifo_t *f){
	if(lr->file != NULL){
		lr->iter->next = f;
		lr->iter = f;
	}else{
		lr->file = f;
		lr->iter = f;
	}
}

void defiler(lecteur_redacteur_t *lr){
	if(lr->file != NULL){
		lr->file = lr->file->next;
	}
}

void initialiser_semaphore(semaphore_t *sem, int token){
	sem->disponible = token;
	pthread_mutex_init(&sem->mutex, NULL);
	pthread_cond_init(&sem->cond, NULL);
}

void detruire_semaphore(semaphore_t *sem){
	pthread_mutex_destroy(&sem->mutex);
	pthread_cond_destroy(&sem->cond);
}
//en attente jusqu'à ce qu'une ressource soit disponible
void P(semaphore_t *sem){
	pthread_mutex_lock(&sem->mutex);
	while(!sem->disponible){
		pthread_cond_wait(&sem->cond, &sem->mutex);
	}
	sem->disponible = 0;
	pthread_mutex_unlock(&sem->mutex);
}

//rend une ressource disponible
void V(semaphore_t *sem){
	pthread_mutex_lock(&sem->mutex);
	sem->disponible = 1;
	pthread_cond_broadcast(&sem->cond);
	pthread_mutex_unlock(&sem->mutex);
}

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lr, priority_t prio){
	lr->compteur_lecteur = 0;
	lr->compteur_redacteur = 0;
	lr->priority = prio;
	initialiser_semaphore(&lr->donnee, 1);
	initialiser_semaphore(&lr->lecteur, 1);
	initialiser_semaphore(&lr->redacteur, 1);
	initialiser_semaphore(&lr->debut_lecteur, 1);
	initialiser_semaphore(&lr->fifo, 1);
	lr->file = NULL;
	lr->iter = NULL;
	lr->disponible = 1;
	lr->disponible_redacteur = 1;
	pthread_mutex_init(&lr->mutex, NULL);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lr){
	detruire_semaphore(&lr->donnee);
	detruire_semaphore(&lr->lecteur);
	detruire_semaphore(&lr->redacteur);
	detruire_semaphore(&lr->debut_lecteur);
	detruire_semaphore(&lr->fifo);
}

void debut_lecture(lecteur_redacteur_t *lr){
	fifo_t f;
	pthread_cond_init(&f.cond, NULL);
	f.LR = LECT;
	f.next = NULL;
	f.id = (int)pthread_self();
	int est_enfile = 0;
	switch(lr->priority){
		case LECTEUR:
			P(&lr->lecteur);
			lr->compteur_lecteur++;
			if(lr->compteur_lecteur == 1){
				P(&lr->donnee);
			}
			V(&lr->lecteur);
			break;

		case REDACTEUR:
			P(&lr->debut_lecteur);
			P(&lr->lecteur);
			lr->compteur_lecteur++;
			if(lr->compteur_lecteur == 1){
				P(&lr->donnee);
			}
			V(&lr->lecteur);
			V(&lr->debut_lecteur);
			break;

		case ORDRE_ARRIVE:
			pthread_mutex_lock(&lr->mutex);
			if(lr->file != NULL){ //verifier s'il y a des threads en attente
				enfiler(lr, &f);
				est_enfile = 1;
				pthread_cond_wait(&f.cond, &lr->mutex);
			}
			while(!lr->disponible){
				if(!est_enfile){
					enfiler(lr, &f);
					est_enfile = 1;
					pthread_cond_wait(&f.cond, &lr->mutex);
				}else{
					pthread_cond_wait(&f.cond, &lr->mutex);
				}
			}
			defiler(lr);
			lr->disponible_redacteur = 0;
			lr->disponible = 0;
			pthread_mutex_unlock(&lr->mutex);
			
			break;

		default:
			break;
	}
}

void fin_lecture(lecteur_redacteur_t *lr){
	switch(lr->priority){
		case LECTEUR:
			P(&lr->lecteur);
			lr->compteur_lecteur--;
			if(lr->compteur_lecteur == 0){
				V(&lr->donnee);
			}
			V(&lr->lecteur);
			break;
		case REDACTEUR:
			P(&lr->lecteur);
			lr->compteur_lecteur--;
			if(lr->compteur_lecteur == 0){
				V(&lr->donnee);
			}
			V(&lr->lecteur);
			break;

		case ORDRE_ARRIVE:
			pthread_mutex_lock(&lr->mutex);
			lr->disponible_redacteur = 1;
			lr->disponible = 1;
			if(lr->file != NULL){
				pthread_cond_signal(&lr->file->cond);
			}
			pthread_mutex_unlock(&lr->mutex);
			break;

		default:
			break;
	}
	
}

void debut_redaction(lecteur_redacteur_t *lr){
	fifo_t f;
	pthread_cond_init(&f.cond, NULL);
	f.LR = RED;
	f.next = NULL;
	f.id = (int)pthread_self();
	int est_enfile = 0;
	switch(lr->priority){
		case LECTEUR:
			P(&lr->redacteur);
			P(&lr->donnee);
			break;
			
		case REDACTEUR:
			P(&lr->redacteur);
			lr->compteur_redacteur++;
			if(lr->compteur_redacteur == 1){
				P(&lr->debut_lecteur);
			}
			V(&lr->redacteur);
			P(&lr->donnee);
			break;

		case ORDRE_ARRIVE:
			pthread_mutex_lock(&lr->mutex);
			if(lr->file != NULL){
				enfiler(lr, &f);
				est_enfile = 1;
				pthread_cond_wait(&f.cond, &lr->mutex);
			}
			while(!lr->disponible || !lr->disponible_redacteur){
				if(!est_enfile){
					enfiler(lr, &f);
					est_enfile = 1;
					pthread_cond_wait(&f.cond, &lr->mutex);
				}else{
					pthread_cond_wait(&f.cond, &lr->mutex);
				}
			}
			defiler(lr);
			lr->disponible_redacteur = 0;
			lr->disponible = 0;
			pthread_mutex_unlock(&lr->mutex);
			break;

		default:
			break;
	}
}

void fin_redaction(lecteur_redacteur_t *lr){
	switch(lr->priority){
		case LECTEUR:
			V(&lr->donnee);
			V(&lr->redacteur);
			break;

		case REDACTEUR:
			V(&lr->donnee);
			P(&lr->redacteur);
			lr->compteur_redacteur--;
			if(lr->compteur_redacteur == 0){
				V(&lr->debut_lecteur);
			}
			V(&lr->redacteur);
			break;

		case ORDRE_ARRIVE:
			pthread_mutex_lock(&lr->mutex);
			lr->disponible_redacteur = 1;
			lr->disponible = 1;
			if(lr->file != NULL){
				pthread_cond_signal(&lr->file->cond);
			}
			pthread_mutex_unlock(&lr->mutex);
			break;

		default:
			break;
	}
}