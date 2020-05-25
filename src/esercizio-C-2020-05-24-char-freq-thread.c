/*********** esercizio-C-2020-05-24-char-freq-thread ************/

/*
è data la string di caratteri content:

char * content = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Mattis rhoncus urna neque viverra justo nec ultrices. Pretium quam vulputate dignissim suspendisse in est ante. Vitae congue mauris rhoncus aenean. Blandit cursus risus at ultrices mi. Ut lectus arcu bibendum at varius vel pharetra vel. Etiam non quam lacus suspendisse faucibus interdum posuere. Eget sit amet tellus cras adipiscing enim eu turpis egestas. Lectus magna fringilla urna porttitor rhoncus dolor purus non. Sit amet consectetur adipiscing elit duis tristique sollicitudin nibh. Nec tincidunt praesent semper feugiat nibh. Sapien pellentesque habitant morbi tristique senectus et netus et malesuada.";

il programma utilizza due thread aggiuntivi, il primo thread analizza i caratteri nelle posizioni "pari" della stringa (content[0], content[2], content[4]...)
mentre il secondo considera i caratteri nelle posizioni "dispari" della stringa (content[1], content[3], content[5]...)

definire un array condiviso:

int counter[256];

ciascun thread incrementa il contatore (counter) corrispondente al valore di ogni carattere incontrato:

counter[content[i]]++;

usare un solo semaforo per risolvere il problema (ci sono più modi di risolverlo...)

il thread principale aspetta il termine dei due thread e poi scrive su stdout la frequenza di ogni carattere.

"frequenza %d = %d\n"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

char * content = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Mattis rhoncus urna neque viverra justo nec ultrices. Pretium quam vulputate dignissim suspendisse in est ante. Vitae congue mauris rhoncus aenean. Blandit cursus risus at ultrices mi. Ut lectus arcu bibendum at varius vel pharetra vel. Etiam non quam lacus suspendisse faucibus interdum posuere. Eget sit amet tellus cras adipiscing enim eu turpis egestas. Lectus magna fringilla urna porttitor rhoncus dolor purus non. Sit amet consectetur adipiscing elit duis tristique sollicitudin nibh. Nec tincidunt praesent semper feugiat nibh. Sapien pellentesque habitant morbi tristique senectus et netus et malesuada.";
int counter[256];
sem_t mutex;

#define THREAD_ERR(a,msg) {if ((a) != 0) { perror((msg)); exit(EXIT_FAILURE); } }
#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

void * thread_function_1(void * arg) {

	printf("[thread_1]start\n");

	char * str = (char *) arg;

	int ch;

	for (int i = 0; str[i] < strlen(str); i+=2) {
		if (sem_wait(&mutex) == -1) {
				perror("sem_wait");
				exit(EXIT_FAILURE);
		}

		ch = (int) str[i];
		counter[ch]++;

		if (sem_post(&mutex) == -1) {
				perror("sem_post");
				exit(EXIT_FAILURE);
		}

	}

	return NULL;
}

void * thread_function_2(void * arg) {

	char * str = (char *) arg;
	int ch;

	for (int i = 1; str[i] < strlen(str); i+=2) {
		if (sem_wait(&mutex) == -1) {
				perror("sem_wait");
				exit(EXIT_FAILURE);
		}

		ch = (int) str[i];
		counter[ch]++;

		if (sem_post(&mutex) == -1) {
				perror("sem_post");
				exit(EXIT_FAILURE);
		}

	}

	return NULL;
}

int main(int argc, char * argv[]) {

	pthread_t t1, t2;
	void * res1, * res2;

	int res = sem_init(&mutex,
				1, 	// 1 => il semaforo è condiviso tra processi, 0 => il semaforo è condiviso tra threads del processo
				1 	// valore iniziale del semaforo (se mettiamo 0 che succede?)
				);

	CHECK_ERR(res, "sem_init")

	int s;
	// create threads
	s = pthread_create(&t1, NULL, thread_function_1, content);

	THREAD_ERR(s, "pthread_create")

	s = pthread_create(&t2, NULL, thread_function_2, content);

	THREAD_ERR(s, "pthread_create")

	// join threads
	s = pthread_join(t1, &res1);

	THREAD_ERR(s, "pthread_join")

	s = pthread_join(t2, &res2);

	THREAD_ERR(s, "pthread_join")

	for (int i = 0; i < 256; i++)
		printf("frequenza %d = %d\n", i, counter[i]);
		//printf("frequenza %d (%c) = %d\n", i, i, counter[i]);

	exit(EXIT_SUCCESS);
}

