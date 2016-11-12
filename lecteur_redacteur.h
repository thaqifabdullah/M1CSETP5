typedef enum{
	LECT, RED
}lecteur_redacteur_type;

typedef enum priority_type{
	LECTEUR, REDACTEUR, ORDRE_ARRIVE
}priority_t;

typedef struct fifo_type{
	pthread_cond_t cond;
	lecteur_redacteur_type LR;
	struct fifo_type *next;
	int id;
}fifo_t;

typedef struct{
	int disponible;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}semaphore_t;

typedef struct{
	int compteur_lecteur;
	int compteur_redacteur;
	priority_t priority;
	semaphore_t donnee;
	semaphore_t lecteur;
	semaphore_t redacteur;
	semaphore_t debut_lecteur;
	semaphore_t fifo;
	pthread_mutex_t mutex;
	fifo_t *file;
	fifo_t *iter;
	int disponible;
	int disponible_redacteur;
}lecteur_redacteur_t;

void initialiser_fifo(fifo_t *fifo);

void initialiser_semaphore(semaphore_t *sem, int token);

void P(semaphore_t *sem);

void V(semaphore_t *sem);

void detruire_semaphore(semaphore_t *sem);

/**
* L'initialisation des structures de synchronisation
**/
void initialiser_lecteur_redacteur(lecteur_redacteur_t *lr, priority_t prio);

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