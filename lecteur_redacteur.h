typedef struct{
	int compteur_lecteur;
	int lecteur_attend;
	int donnee_disponible;
	pthread_cond_t redacteur_ok;
	pthread_cond_t redacteur_parti;
	pthread_mutex_t mutex_donnee;
	pthread_mutex_t mutex_compteur_lecteur;
}lecteur_redacteur_t;

/**
* L'initialisation des structures de synchronisation
**/
void initialiser_lecteur_redacteur(lecteur_redacteur_t *lr);

/**
* La fonction de destruction des structures de synchronisation
**/
void detruire_lecteur_redacteur(lecteur_redacteur_t *lr);

/**
*Les primitives éxécutées par chaque lecteur avant et après une lecture
**/
void debut_lecture(lecteur_redacteur_t *lr);

void fin_lecture(lecteur_redacteur_t *lr);

/**
*Les primitives éxécutées par chaque rédacteur avant et après une écriture
**/
void debut_redaction(lecteur_redacteur_t *lr);

void fin_redaction(lecteur_redacteur_t *lr);