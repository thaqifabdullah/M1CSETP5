#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lecteur_redacteur.h"

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lr){
	lr->compteur_lecteur = 0;
	lr->donnee_disponible = 1;
	lr->lecteur_attend = 0;
	pthread_cond_init(&lr->redacteur_ok, NULL);
	pthread_cond_init(&lr->redacteur_parti, NULL);
	pthread_mutex_init(&lr->mutex_donnee, NULL);
	pthread_mutex_init(&lr->mutex_compteur_lecteur, NULL);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lr){
	pthread_mutex_destroy(&lr->mutex_donnee);
	pthread_mutex_destroy(&lr->mutex_compteur_lecteur);
}

void debut_lecture(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_compteur_lecteur);
	lr->compteur_lecteur++;
	if(lr->compteur_lecteur == 1){
		pthread_mutex_lock(&lr->mutex_donnee);
		while(!lr->donnee_disponible){
			lr->lecteur_attend = 1;
			pthread_cond_wait(&lr->redacteur_parti, &lr->mutex_donnee);
		}
		lr->lecteur_attend = 0;
		lr->donnee_disponible = 0;
		pthread_mutex_unlock(&lr->mutex_donnee);
	}
	pthread_mutex_unlock(&lr->mutex_compteur_lecteur);
}

void fin_lecture(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_compteur_lecteur);
	lr->compteur_lecteur--;
	if(lr->compteur_lecteur == 0){
		pthread_mutex_lock(&lr->mutex_donnee);
		lr->donnee_disponible = 1;
		pthread_cond_signal(&lr->redacteur_ok);
		pthread_mutex_unlock(&lr->mutex_donnee);
	}
	pthread_mutex_unlock(&lr->mutex_compteur_lecteur);
}

void debut_redaction(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_donnee);
	while(!lr->donnee_disponible){
		pthread_cond_wait(&lr->redacteur_ok, &lr->mutex_donnee);
	}
	lr->donnee_disponible = 0;
	pthread_mutex_unlock(&lr->mutex_donnee);
}

void fin_redaction(lecteur_redacteur_t *lr){
	pthread_mutex_unlock(&lr->mutex_donnee);
	lr->donnee_disponible = 1;
	if(lr->lecteur_attend){
		pthread_cond_signal(&lr->redacteur_parti);
	}else{
		pthread_cond_signal(&lr->redacteur_ok);
	}
	pthread_mutex_unlock(&lr->mutex_donnee);
}