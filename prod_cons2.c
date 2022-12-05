#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <ctype.h>


int shmid, duree, produit = 0, consom =0, nb_vide, dispo=0; /* id de la mémoire partagée*/
void *production ();
void *consommation ();
void *functionClock ();

short *adresse = NULL;
sem_t mutex, plein , vide;
key_t shm_key; /*clé de la mémoire partagée*/

clock_t startTime;

void delay ()
{
    int ms = 10;
    clock_t begin = clock();
    while ((double)((clock()-begin)/CLOCKS_PER_SEC) < 0.01);
}
int init(int memSize){
  shm_key = ftok("prod_cons.key", 256); // création de la clé
  if(shm_key < 0){ // la clé n'a pas pu être créée
    fprintf(stderr, "Impossible de créer la clé (%s)\n", strerror(errno));
    return shm_key;
  }
  /* Création de la mémoire partagée */
  fprintf(stderr, "key %d\n", shm_key);
  shmid = shmget(shm_key, memSize, IPC_EXCL | IPC_CREAT | 0666);
  if (shmid<0){
    fprintf(stderr, "Impossible de créer la mémoire partagée (%s), %d\n", strerror(errno), errno);
    if(errno!=EEXIST)
      return shmid;
    /* Si la mémoire partagée existe déjà , il faut obtenir son id*/
    shmid = shmget(shm_key, memSize, 0666);
    if(shmid < 0)
      return shmid;
  }
  fprintf(stderr, "shmid %d\n", shmid);
  return 1;
}

void mem_attach(int shmid,void **adresse){
  /* Le processus attache une adresse dans son
   *  espace mémoire à la mémoire partagée
   */
  *adresse=(short *)shmat(shmid, NULL,0);
  if (*adresse==(void *)-1) exit(1);
}

void mem_detach(void **adresse){
  /* Le processus détache l'adresse de son
   *  espace mémoire de la mémoire partagée
   */
  shmdt(*adresse);
}

int testInt(char * arg){
    for (int i=0; i<strlen(arg); i++){
        if (!isdigit(arg[i])){

            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]){
    srand(time(NULL));
    duree = rand()%10;
    startTime = clock();
    if(argc<4){
      fprintf(stderr, "%s <Nombre d'argument invalide>\n", argv[0]);
      return 1;
    }
    if (argc == 5 && testInt(argv[4]))
        duree = atoi(argv[4]);
    if ( !testInt(argv[1]) || !testInt(argv[2]) || !testInt(argv[3]) || !testInt(argv[4])){
        printf("Problème d'argument\n" );
        return 0;
    }
    int memS = atoi(argv[3]), prod = atoi(argv[1]), cons = atoi(argv[2]);

    int ret;
    ret = init(memS);
    nb_vide = atoi(argv[3]);
    if(ret < 0)
        return -1;
    sem_init(&plein , 0 , 0 ) ;
    sem_init(&vide , 0 , atoi(argv[3]) ) ;
    sem_init(&mutex , 0 , 1 ) ;
    pthread_t threadPro[prod], threadCons[cons], threadPrinc;
    int rc1, rc2, rc3;
    for (int i=0; i<prod; i++){
        if( (rc1=pthread_create( &threadPro[i], NULL, &production, NULL)) ){
           printf("Thread creation failed: %d\n", rc1);
        }
    }
    for (int i=0; i<cons; i++){
        if( (rc2=pthread_create( &threadCons[i], NULL, &consommation, NULL)) ){
           printf("Thread creation failed: %d\n", rc2);
        }
    }
   if( (rc3=pthread_create( &threadPrinc, NULL, &functionClock, NULL)) ){
      printf("Thread creation failed: %d\n", rc3);
   }
   for (int i=0; i<prod; i++){
       pthread_join( threadPro[i], NULL);
   }
    for (int i=0; i<cons; i++){
        pthread_join( threadCons[i], NULL);
    }
    pthread_join( threadPrinc, NULL);

}
void *production(){
    mem_attach(shmid, (void**)&adresse);
    assert(adresse != NULL);
    while (1){
        sem_wait(&vide);
        sem_wait(&mutex);
        //production
        produit++;
        *(adresse+ nb_vide) = 'a';
        nb_vide--;
        dispo++;
        printf("Production !!!%d\n", produit );
        sleep(duree);
        //fin production
        sem_post(&mutex);
        sem_post(&plein);
    }
}

void *consommation(void * cons){
    mem_attach(shmid, (void**)&adresse);
    assert(adresse != NULL);
    while (1){
        sem_wait(&plein);
        sem_wait(&mutex);
        //Consommation
        consom++;
        nb_vide++;
        dispo--;
        printf("Consommation !!!%d\n",consom );
        sleep(duree);
        //fin consommation
        sem_post(&mutex);
        sem_post(&vide);
    }
}

void* functionClock() {
    /* code */
    unsigned long temps;
    while (1){
        delay();
        printf("Affichage de l'etat de production :\n" );
        clock_t finish = clock();
        // int conv;
        // conv = sem_getvalue(&vide, produ);

        double temps = (double) (finish - startTime)/CLOCKS_PER_SEC;
        printf("t = %f\n", temps);
        printf("X(total produit) = %d\n", produit );
        printf("Y(total consommé) = %d\n", consom );
        printf("produit dispo = %d\n", dispo );
        printf("Z = %d\n", nb_vide );
    }
}
