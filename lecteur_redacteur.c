#include "lecteur_redacteur.h"

void initialiser_lecteur_redacteur(lecteur_redacteur_t *lr){
	lr->compteur_lecteur = 0;
	lr->compteur_redacteur = 0;
	pthread_cond_init(&lr->redacteur_ok);
	pthread_mutex_init(&lr->mutex_donnee);
	pthread_mutex_init(&lr->mutex_compteur_lecteur);
	pthread_mutex_init(&lr->mutex_compteur_redacteur);
}

void detruire_lecteur_redacteur(lecteur_redacteur_t *lr){
	pthread_mutex_destroy(&lecteur_redacteur->mutex_donnee);
	pthread_mutex_destroy(&lecteur_redacteur->mutex_compteur_lecteur);
	pthread_mutex_destroy(&lr->mutex_compteur_redacteur);
}

void debut_lecture(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_compteur_redacteur);
	if(&lr->compteur_redacteur > 0){
		pthread_cond_wait(&lr->lecteur_ok);
	}
	pthread_mutex_lock(&lr->mutex_compteur_lecteur);
	lr->compteur_lecteur++;
	pthread_mutex_unlock(&lr->mutex_compteur_lecteur);
}

void fin_lecture(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_compteur_lecteur);
	lr->compteur_lecteur--;
	if(lr->compteur_lecteur == 0){
		pthread_cond_signal(&lr->redacteur_ok);
	}
	pthread_mutex_unlock(&lr->mutex_compteur_lecteur);
}

void debut_redaction(lecteur_redacteur_t *lr){
	pthread_mutex_lock(&lr->mutex_compteur_lecteur);
	while(lr->compteur_lecteur > 0){
		pthread_cond_wait(&lr->redacteur_ok);
	}

}

void fin_redaction(lecteur_redacteur_t *lr){
	pthread_mutex_unlock(&lr->mutex_compteur_lecteur);
}