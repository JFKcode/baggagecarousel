#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct Cegla
{
    int index;
    int waga;
};

#define MAX_WAGA 10
struct Cegla tasmociag[MAX_WAGA];
int index_wstaw=0;
int index_usun=0;
int waga_cegiel=0;
int index_nastepnej_cegly=0;

#define LICZBA_UMIESZCZAJACYCH 2
#define LICZBA_SCIAGAJACYCH 3
#define UMIESZCZAJACY_SLEEP 100
#define SCIAGAJACY_SLEEP 200

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sa_cegly=PTHREAD_COND_INITIALIZER;
pthread_cond_t jest_wolne_miejsce=PTHREAD_COND_INITIALIZER;

void *robotnik_umieszczajacy(void *param)
{
    int index = (int)param;
    while(1) {
        pthread_mutex_lock (&mutex);
        int waga_nowej = 1 + rand() % 2;
        while  (MAX_WAGA - waga_cegiel < waga_nowej)
            pthread_cond_wait (&jest_wolne_miejsce, &mutex);

        struct Cegla cegla;
        cegla.index = index_nastepnej_cegly++;
        cegla.waga = waga_nowej;
        tasmociag[index_wstaw] = cegla;
        index_wstaw = (index_wstaw+1) % MAX_WAGA;
        waga_cegiel += cegla.waga;
        printf ("robotnik %d umiescil cegle o indeksie %d i wadze %d\n", index, cegla.index, cegla.waga);
        fflush (stdout);
        pthread_mutex_unlock (&mutex);
        pthread_cond_broadcast(&sa_cegly);
        usleep((UMIESZCZAJACY_SLEEP+ rand()%UMIESZCZAJACY_SLEEP)*1000);
    }

}

void *robotnik_sciagajacy(void *param)
{
    int index = (int)param;
    while (1) {
        pthread_mutex_lock (&mutex);
        while (waga_cegiel == 0)
            pthread_cond_wait (&sa_cegly, &mutex);

        struct Cegla cegla = tasmociag[index_usun];
        index_usun = (index_usun+1) % MAX_WAGA;
        waga_cegiel -= cegla.waga;
        printf ("Robotnik %d usunal cegle o indeksie %d i o wadze %d\n", index, cegla.index, cegla.waga);
        fflush(stdout);
        pthread_mutex_unlock (&mutex);
        pthread_cond_broadcast(&jest_wolne_miejsce);
        usleep((SCIAGAJACY_SLEEP+rand()%SCIAGAJACY_SLEEP)*1000);
    }
}

int main (int argc, char *argv[])

{
    pthread_t umieszczajacy[LICZBA_UMIESZCZAJACYCH];
    pthread_t sciagajacy[LICZBA_SCIAGAJACYCH];

    for (int i = 0; i < LICZBA_UMIESZCZAJACYCH; i++)
        pthread_create(&umieszczajacy[i],NULL,robotnik_umieszczajacy,i);

    for (int i = 0; i < LICZBA_SCIAGAJACYCH; i++)
        pthread_create(&sciagajacy[i],NULL,robotnik_sciagajacy,LICZBA_UMIESZCZAJACYCH + i);

    for (int i = 0; i < LICZBA_UMIESZCZAJACYCH; i++)
        pthread_join(umieszczajacy[i],NULL);

    for (int i = 0; i < LICZBA_SCIAGAJACYCH; i++)
        pthread_join(sciagajacy[i],NULL);

    return 0;
}