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
#include <ctype.h>
#include <fcntl.h>

void produire (short *);
void consommer (short *);
sem_t mutex, plein , vide, *plein2, *mutex2, *vide2;
key_t shm_key; /*clé de la mémoire partagée*/
int shmid, duree, produit = 0, consom =0, nb_vide; /* id de la mémoire partagée*/

int testInt(char * arg){
    for (int i=0; i<strlen(arg); i++){
        if (!isdigit(arg[i])){

            return 0;
        }
    }
    return 1;
}
void delay ()
{
    int ms = 10;
    clock_t begin = clock();
    while ((double)((clock()-begin)/CLOCKS_PER_SEC) < 0.01);
}
int init(int memSize){
  shm_key = ftok("prod_con.key", 256); // création de la clé
  if(shm_key < 0){ // la clé n'a pas pu être créée
    fprintf(stderr, "Impossible de créer la clé (%s)\n", strerror(errno));
    return shm_key;
  }
  /* Création de la mémoire partagée */
  fprintf(stderr, "key %d\n", shm_key);
  shmid = shmget(shm_key, 20, IPC_EXCL | IPC_CREAT | 0666);
  if (shmid<0){
    fprintf(stderr, "Impossible de créer la mémoire partagée (%s), %d\n", strerror(errno), errno);
    if(errno!=EEXIST)
      return shmid;
    /* Si la mémoire partagée existe déjà , il faut obtenir son id*/
    shmid = shmget(shm_key, 20, 0666);
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

int main(int argc, char *argv[]){
    srand(time(NULL));
    duree = rand()%10;
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
    short *adresse = NULL;
    int ret;
    ret = init(memS);
    nb_vide = atoi(argv[3]);
    if(ret < 0)
        return -1;

    // sem_init(&plein , 0 , 0 ) ;
    // sem_init(&vide , 0 , atoi(argv[3]) ) ;
    // sem_init(&mutex , 0 , 1 ) ;
    if(( mutex2 = sem_open("/mutex2", O_CREAT, 0600, 1) )==SEM_FAILED)
    {
         perror ("impossible de créer le semaphore mutex\n");
         exit (-1);
     }
     sem_unlink("/mutex2");
     if(( vide2 = sem_open("/vide2", O_CREAT, 0600, atoi(argv[3])) )==SEM_FAILED)
     {
          perror ("impossible de créer le semaphore vide\n");
          exit (-1);
      }
      sem_unlink("/vide2");
      if(( plein2 = sem_open("/plein2", O_CREAT, 0600, 0) )==SEM_FAILED)
      {
           perror ("impossible de créer le semaphore plein\n");
           exit (-1);
       }
       sem_unlink("/plein2");
    pid_t pidp[prod], pidc[cons];
    int p =0, c=0, compt=0;
    mem_attach(shmid, (void**)&adresse);
    *(adresse) = 0;
    *(adresse + 4) = 0;
    *(adresse + 8) = atoi(argv[3]);
    while (p<prod || c<cons) {
        if (p<prod){
            pidp[p] = fork();

            if(pidp[p] < 0){
              fprintf(stderr, "Erreur de création du processus producteur (%d)\n", errno);
              return 1;
          } else if(pidp[p] == 0){
                mem_attach(shmid, (void**)&adresse);
                // if (compt == 0){
                //     *(adresse) = 0;
                //     *(adresse + 4) = 0;
                //     *(adresse + 8) = 0;
                //     compt++;
                // }
                assert(adresse != NULL);
                produire (adresse);
                return 0;
            }else{
                fprintf(stderr, "Je suis le processus (%d) qui vient de créer le producteur (%d)\n", getpid(), pidp[p]);
            }
            p++;
        }
        if (c<cons){
            pidc[c] = fork();

            if(pidc[c] < 0){
              fprintf(stderr, "Erreur de création du processus consommateur (%d)\n", errno);
              return 1;
            }
            if(pidc[c] == 0){
                mem_attach(shmid, (void**)&adresse);
                assert(adresse != NULL);
                consommer (adresse);
                return 0;
            }else{
                fprintf(stderr, "Je suis le processus (%d) qui vient de créer le consommateur (%d)\n", getpid(), pidc[c]);
            }
            c++;
        }
    }
    int status, i=4;
    pid_t pid2;
    while (1){
        delay();
        printf("Affichage de l'etat de production :\n" );
        double temps = (double) clock()/CLOCKS_PER_SEC;
        produit = *(adresse) ;
        consom = *(adresse +4);
        nb_vide = *(adresse + 8);
        printf("t = %f\n", temps);
        printf("X = %d\n", produit );
        printf("Y = %d\n", consom );
        printf("Z = %d\n", nb_vide );
        //i--;
    }

    while( (pid2 = wait(&status)) > 0){
        fprintf(stderr, "Mon fils (%d) vient de se terminer (%d)\n", pid2, status);
    }
}

void produire(short * addr){
    int i=5;
    while (i>0){
        sem_wait(vide2);
        sem_wait(mutex2);
        produit = *(addr) ;
        nb_vide = *(addr + 8);
        //production
        produit ++;
        nb_vide --;
        *(addr) = produit;
        *(addr + 8) = nb_vide;
        printf("Production !!!%d\n",produit );
        sleep(duree);
        //fin production
        sem_post(mutex2);
        sem_post(plein2);
    }

}

void consommer(short * addr){
    int i=5;
    while (i>0){
        sem_wait(plein2);
        sem_wait(mutex2);
        consom = *(addr + 4);
        nb_vide = *(addr + 8);
        //production
        consom ++;
        nb_vide++;
        *(addr + 4) = consom;
        *(addr + 8) = nb_vide;
        printf("Consommation !!!%d\n",consom );
        sleep(duree);
        //fin production
        sem_post(mutex2);
        sem_post(vide2);
    }

}
