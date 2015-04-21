#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

struct node {
    char fileName[256];
    struct node *next;
};
int main(int argc, char**argv) {
   int s,n,ret, byte_count, time, tamanho;
   struct addrinfo hints, *res = NULL;
   struct timeval tv0, tv1;
   char sent[256];
   char received[256];
   char tamanho_string[10];
   DIR *d;
   char *dir = argv[3]; //nome do diretório
   struct node *root;
   struct node *position;

   if (argc != 4) {
      printf("Argumentos necessarios: host do servidor, porta e nome do diretorio.\n");
      exit(1);
   }


   root = malloc(sizeof(struct node));


   //identificação: ipv4 ou ipv6
   bzero(&hints, sizeof(hints)); //limpa a variável


   hints.ai_family = PF_UNSPEC; //tipo do endereço não especificado
   hints.ai_flags = AI_NUMERICHOST;

   ret = getaddrinfo(argv[1], argv[2], &hints, &res);

   if(ret) {
        printf("Endereço inválido.");
        exit(1);
   }

   s = socket(res->ai_family,SOCK_STREAM,0);
   puts("socket criado\n");


   connect(s, res->ai_addr, res->ai_addrlen);//I'm gonna do an Internet
   puts("conectado\n");

   strcpy(sent,"READY");

   //inicia contagem de bytes e do tempo
   byte_count = 0;
   gettimeofday(&tv0,0);
   n = send(s,sent, strlen(sent),0); //envia "READY"
   while (n==-1) {
        printf("Erro ao enviar mensagem.");
        n = send(s, sent, strlen(sent),0);
   }
   byte_count += n;

   n = recv(s,received,sizeof(received),0); //recebe mensagem do server
   received[n] = 0;
   byte_count += n;

   if (strcmp(received, "READY ACK") != 0) {
        printf("Mensagem inesperada recebida: %s",received);
        exit(1);
        n=recv(s,received,sizeof(received),0);
        received[n] = 0;
        byte_count += n;
   }

   d = opendir(dir); //abre o diretório
   if (!d) {
        printf("Erro ao abrir local dos arquivos '%s': %s.", dir, strerror(errno));
        exit(1);
   }

   position = root;
   root -> next = 0;
   while (1) {
        struct dirent *file;
        file = readdir(d);
        if(!file) { //fim do diretório
                position->next = 0;
                break;
        }
        char dirList[256];
        strcpy(dirList, file->d_name);
        if (dirList[0]=='b'&&dirList[1]=='y') { //evitar arquivos com nome "bye"
            int i;
            char aux[256];
            strcpy(aux,dirList);
            for (i=2; i<=strlen(dirList); i++) { //"bit stuffing" na string
                dirList[i+1] = aux[i];
            }
            dirList[2] = 'y';
        }
        strcpy(position->fileName,dirList);
        position->next = malloc(sizeof(struct node));
        position = position->next;
   }

   if (closedir(d)) {//fecha o diretório
        printf("Erro ao fechar local dos arquivos.");
        exit(1);
   }
   position = root;
   while (position->next!=0) {
        strcpy(sent, position->fileName);
        tamanho = strlen(sent);//tamanho da mensagem
        sprintf(tamanho_string, "%03d",tamanho); //concatena ao início de cada mensagem o seu tamanho
        strcat(tamanho_string, sent);
        strcpy(sent, tamanho_string);
        puts(sent);
        n = send(s,sent, strlen(sent),0); //envia tamanho da mensagem + nome do arquivo
        while (n==-1) {
            printf("Erro ao enviar mensagem.");
            n = send(s, sent, strlen(sent),0);
        }
        byte_count += n;
        position = position->next;
   }
   strcpy(sent, "bye");
   tamanho = strlen(sent);//tamanho da mensagem
    sprintf(tamanho_string, "%03d",tamanho);
    strcat(tamanho_string, sent);
    strcpy(sent, tamanho_string);
    puts(sent);
    n = send(s,sent, strlen(sent),0); //envia "bye"
   while (n==-1) {
        printf("Erro ao enviar mensagem.");
        n = send(s, sent, strlen(sent),0);
   }
   byte_count += n;

   close(s); //encerra a conexão

   gettimeofday(&tv1,0); //finaliza a contagem de tempo
   long total = (tv1.tv_sec - tv0.tv_sec)*1000000 + tv1.tv_usec - tv0.tv_usec; //tempo decorrido, em milissegundos
   printf("\nDesempenho: \nBytes enviados: %d \nTempo decorrido (milissegundos): %ld\n\n", byte_count, total);
   return 0;
}
