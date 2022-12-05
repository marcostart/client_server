#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#define SIZE 1048576

int init();
int port =0;

int main(int argc, char *argv[]){
    if(argc<3){
      fprintf(stderr, "%s <Nombre d'argument inalide>\n", argv[0]);
      return 1;
    }
    for (int i=0; i<strlen(argv[2]); i++){
        if (!isdigit(argv[2][i])){
            printf("Problème d'argument\n" );
            return 0;
        }
    }
    port = atoi(argv[2]);
   int ret, size_inbytes = 0, sd, csd;
   fd_set readset, writeset;
   struct sockaddr_in client_addr;
   int clen, fic;
   char nom_fic2[SIZE];
   sprintf(nom_fic2, "%s", argv[1]);
   printf("taille = %ld\n", strlen(nom_fic2));
   sd = init();
   /*fcntl(1, F_SETFL, O_NONBLOCK );
   fcntl(sd, F_SETFL, O_NONBLOCK );
   FD_ZERO(&readset);
   FD_ZERO(&writeset);*/
  while(1){
    clen = sizeof((struct sockaddr *)&client_addr);
    if((csd = accept(sd,(struct sockaddr *)&client_addr, &clen)) < 0){
          fprintf(stderr, "Un petit problème lors du accept %d\n", errno);
          close(csd);
          return -1;
      }
       fprintf(stdout, "tentative de connexion du %s:%d\n", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
      pid_t pid = fork();

       if (pid == 0){

           write(csd, nom_fic2, strlen(nom_fic2));
           fic = open(nom_fic2, O_CREAT | O_RDONLY ,0777);
           if (fic == -1){
               perror("open");
           }
           printf("Hello ...\n");
           int ret_w;
           char buf[SIZE];
           while((ret = read(fic, buf, SIZE)) >0 ){
               printf("entre\n");
                ret_w = write(1, buf, ret);
                if(ret_w == -1){
                    perror("write");
                }

               write(csd, buf, ret);
           }
           printf("\n[+]Fichier envoyé avec succès.\n");
           close(csd);
           //for (int i=0; i<500;i++);

      }
      else if(pid > 0){
          //printf("Le père\n" );
          close(csd);
           //close(sd);
      }

  }
  int status;
  pid_t pid2;
  while( (pid2 = wait(&status)) > 0);
       for(;;){}
       while (1){
           //FD_SET(sd, &writeset);
           /*FD_SET(sd, &readset);
           ret = select(sd+1, &readset, &writeset, NULL, NULL);
           if(ret < 0){
              printf("erreur de select\n");
              exit(1);
           }
           if (FD_ISSET(sd, &readset)){
              fprintf(stdout, "tentative de connexion\n");
              FD_CLR(sd, &readset);
              break;
           }*/
           fprintf(stdout, "tentative de connexion\n");
        }
        fprintf(stderr , "La taille du buffer du clavier est %d", size_inbytes);

}

int init(){
   int sd, ret;
   struct sockaddr_in serv_addr;
   sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0){
      printf("Error in socket creation\n");
      return sd;
   }
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   serv_addr.sin_port = htons(port);

   ret=bind(sd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
   if(ret<0){
      printf("Error in bind\n");
      perror("bind");
      close(sd);
      return ret;
    }

    if(listen(sd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }
        printf("En ecoute ...\n" );
    return sd;

}
