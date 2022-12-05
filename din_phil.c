#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>

# define G ((i+N-1) %N)
# define D ( i )
int duree, N;

typedef enum etat etat;
enum etat { penser , faim , manger } ;
etat *Etat ;
sem_t **sem_phi ;
static sem_t *mutex ;
void philosophe(int p);
void verif(int p);

int testInt(char * arg){
    for (int i=0; i<strlen(arg); i++){
        if (!isdigit(arg[i])){

            return 0;
        }
    }
    return 1;
}
int main(int argc, char *argv[]){
    if(argc<3){
      fprintf(stderr, "%s <Nombre d'argumentfff invalide>\n", argv[0]);
      return 1;
    }
    if ( !testInt(argv[1]) || !testInt(argv[2]) ){
        printf("Problème d'argument\n" );
        return 0;
    }
    N=atoi(argv[1]);
    duree = atoi(argv[2]);
    Etat = malloc(N*sizeof(etat));
    sem_phi = malloc(N*sizeof(sem_t));
    int i, listPhi[N], c = 0;
    pid_t proc[N];
    //sem_init(&mutex, 0, 1);
    if(( mutex = sem_open("/mutex", O_CREAT| O_RDWR, 0600, 2) )==SEM_FAILED)
    {
         perror ("impossible de créer le semaphore mutex\n");
         exit (-1);
     }
     sem_unlink("/mutex");
    for (i=0; i<N; i++){
        Etat[i]= penser;
        listPhi[i] = i;
        char var = i + '0';
        char dest[20] ="/phil";
        char * nom = strcat(dest, &var);
        if(( sem_phi[i] = sem_open(dest, O_CREAT| O_RDWR, 0600, 0) )==SEM_FAILED)
        {
             perror ("impossible de créer le semaphore mutex\n");
             exit (-1);
         }
         sem_unlink(dest);
        //sem_init(sem_phi[i], 0, 0);
    }
    while (c<N){
        proc[c] = fork();
        if(proc[c] < 0){
          fprintf(stderr, "Erreur de création du processus philosophe (%d)\n", errno);
          return 1;
        } else if(proc[c] == 0){

            printf("Suis al = %d\n", listPhi[c] );
            philosophe(listPhi[c]);
            return 0;

        }else{
            fprintf(stderr, "Je suis le processus (%d) qui vient de créer le philosophe (%d)\n", getpid(), proc[c]);
        }
        c++;
    }
    int status;
    pid_t pid2;
    while( (pid2 = wait(&status)) > 0){
        fprintf(stderr, "Mon fils (%d) vient de se terminer (%d)\n", pid2, status);
    }
    free(sem_phi);
    free(Etat);
}
void philosophe(int p){
    int i = p, n = 2;
    while (n>0){
        sleep(duree); //il pense
        //Il essaie de manger
        printf("OK\n" );
        //sem_post(mutex);
        sem_wait(mutex);
        Etat [i]= faim ;
        verif (i) ;
        printf("OKpasse\n" );
        sem_post(mutex);
        sem_wait(sem_phi[i]);
        //manger
        printf("Je suis le philosophe %d et je mange.\n", i);
        sleep(duree);
        printf("Je suis le philosophe %d et j'ai fini de manger.\n", i);
        //il libère ses fouchettes
        sem_wait(mutex);
        Etat [i]= penser ;
        //Il vérifie si ses voisins sont en état de faim et peuvent manger
        verif(G);
        verif(D);
        sem_post(mutex);
        n--;
    }
}

void verif(int p){
    int i = p;
    if (Etat[i] == faim && Etat[G] != manger && Etat[D] != manger){
        Etat[i] = manger;
        sem_post(sem_phi[i]);
    }
}
