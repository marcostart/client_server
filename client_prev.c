#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#define SIZE 1048576

int init();
int verifAdd(char *);
int port =0;
char ip[20];

int main(int argc, char *argv[]){
    if(argc<3){
      fprintf(stderr, "%s <Nombre d'argument invalide>\n", argv[0]);
      return 1;
    }
    for (int i=0; i<strlen(argv[2]); i++){
        if (!isdigit(argv[2][i])){
            printf("Problème d'argument\n" );
            return 0;
        }
    }
    port = atoi(argv[2]);
    char str[SIZE];
    sprintf(str, "%s", argv[1]);
    int compt=0, ver=0;
      char sep[] = ".";
      char *p = strtok(str, sep);
      while(p != NULL)
      {
          compt++;
          ver = verifAdd(p);
          if (ver < 0){
              compt=ver;
              break;
          }
        printf("'%s'\n", p);
        p = strtok(NULL, sep);
      }
      if (compt<4){
          printf("Erreur: Adresse incorrect !\n" );
          return 0;
      }
      sprintf(ip, "%s", argv[1]);
   int ret, sd;
   //fd_set readset, writeset;
   sd = init();
   /*fcntl(1, F_SETFL, O_NONBLOCK );
   fcntl(sd, F_SETFL, O_NONBLOCK );
   FD_ZERO(&readset);
   FD_ZERO(&writeset);*/
   // while (1){
   //     /*FD_SET(sd, &writeset);
   //     FD_SET(sd, &readset);
   //     ret = select(sd+1, &readset, &writeset, NULL, NULL);
   //     if (FD_ISSET(sd, &readset)){
   //        fprintf(stdout, "tentative de connexion\n");
   //        FD_CLR(sd, &readset);
   //     }*/
   //     //fprintf(stdout, "tentative de connexion\n");
   //  }
   //  fprintf(stderr , "La taille du buffer du clavier est %d", size_inbytes);
}
int verifAdd(char * chaine){
    for (int i=0; i<strlen(chaine); i++){
        if (!isdigit(chaine[i])){
            return -1;
        }
    }
    int numb = atoi(chaine);
    printf("%d\n", numb);
    if (numb >= 256)
        return -1;
    return 1;
}

int init(){
   struct sockaddr_in serv_addr;
   int sd;
   /* Creation du socket*/
   if((sd = socket(AF_INET, SOCK_STREAM, 0))< 0){
        printf("\n Erreur  de socket \n");
        return sd;
   }

   /* Initialisation du strucutre du socket */
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(port); // port
   serv_addr.sin_addr.s_addr = inet_addr(ip);


   if(connect(sd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        printf("\n Error: Connect Failed (%d) \n", errno);
        return -1;
   }
    printf("Connection établie avec le serveur avec succès\n" );
   char buffer[SIZE], buffer2[SIZE];
   int ret;
    ret = read(sd,buffer,SIZE);
    printf("%s  %d\n", buffer, ret);
    int fic = open(buffer, O_CREAT | O_RDWR ,0777);

   while((ret = read(sd, buffer2, SIZE)) >0 ) {
       printf("yoooo %d\n", ret );
       //printf("%c\n", buffer2);
       write(1,buffer2, ret);
       write(fic,buffer2, ret);
   }
   printf("Fichier téléchargé avec succès.\n");
   //int retour = recv(sd,buffer,sizeof(buffer),0 );
   //read(sd, buffer, 25);


   // for(;;){}
   // printf("end %d\n", sd);

   return sd;
}
