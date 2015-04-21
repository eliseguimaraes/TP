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

int main(int argc, char**argv) {
    int s, ret, len, n, conn,tamanho;
    struct sockaddr_in6 cliaddr;
    struct addrinfo hints, *res;
    char received[256], sent[256], string[256];
    FILE *arquivo;

    if (argc != 4) {
        printf("%d argumentos\n",argc);
        printf("Argumentos necessarios: host do servidor, porta e nome do diretorio.\n");
        exit(1);
    }

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    ret = getaddrinfo(NULL,argv[2],&hints,&res);
    if(ret) {
        printf("Endereço inválido.");
        exit(1);
    }

    s=socket(res->ai_family,SOCK_STREAM,0);
    puts("socket criado.\n");


    bind(s,res->ai_addr,res->ai_addrlen);
    puts("bind\n");

    if (listen(s,10) == -1) {
        printf("\nErro ao aguardar conexão: %s", strerror(errno));
        exit(1);
    }


    len=sizeof(cliaddr);
    conn = accept(s,(struct sockaddr *)&cliaddr,&len);

    if(conn == -1) {
        perror("Erro ao aceitar conexão.\n");
        exit(1);
    }
    puts("Conectado ao cliente.\n");
    strcpy(string, argv[1]); //host do servidor
    strcat(string, argv[3]); //nome do diretório
    strcat(string, ".txt");
    arquivo = fopen(string,"w"); //abre o arquivo
    n = recv(conn,received,sizeof(received),0);
    received[n] = 0;
    if(!strcmp(received, "READY")) {
        strcpy(sent,"READY ACK");
        send(conn,sent,strlen(sent),0);
        //início do recebimento da lista de arquivos
        while (n = recv(conn,received,3,0)) { //lê primeiro o tamanho da mensagem que seguirá
            received[n] = 0;
            tamanho = atoi(received); //converte o tamanho para inteiro, que será o tamanho do buffer da próxima leitura
            n = recv(conn,received,tamanho,0);
            received[n] = 0;
            if (received[0]=='b'&&received[1]=='y') {
                if (received[2]=='e') { //fim da comunicação, anunciado por "bye"
                    fclose(arquivo); //salva e fecha o arquivo
                    close(conn); //fim da conexão
                    return 0;
                }
                else { //bit stuffing. remover caracter inserido
                    int i;
                    for (i = 2; i<strlen(received)-1;i++) {
                        received[i] = received[i+1];
                    }
                    received[n-1] = 0;
                }
            }
            puts(received);
            fprintf(arquivo, "%s",received);
        }

    }

    return 0;
}
