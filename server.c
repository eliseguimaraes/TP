#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char**argv) {
    int s, ret, len,n;
    struct sockaddr_in6 cliaddr;
    struct addrinfo hints, *res;
    char received[256], sent[256], string[256];
    FILE *arquivo;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    ret = getaddrinfo(NULL,argv[2],&hints,&res);
    if(ret) {
        printf("Endereço inválido.");
        puts(gai_sterror(ret));
        exit(1);
    }

    s=socket(res->ai_family,SOCK_STREAM,0);


    bind(s,res->ai_addr,res->ai_addrlen);

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
    strcpy(string, argv[1]); //host do servidor
    strcat(string, argv[3]); //nome do diretório
    strcat(string, ".txt");
    arquivo = fopen(string,"w"); //abre o arquivo

    while(1) {
        n = recv(conn,received,strlen(received),0);
        received[n] = 0;
        if(!strcmp(received, "READY")) {
            strcpy(sent,"READY ACK");
            send(conn,sent,strlen(sent),0);
            while (n = recv(conn,received,strlen(received),0)) {
                received[n] = 0;
                if (received[0]=='b'&&received[1]=='y') {
                    if (received[2]=='e') { //fim da comunicação, anunciado por "bye"
                        fclose(arquivo);
                    }
                    else { //bit stuffing. remover caracter inserido
                        for (int i = 2; i<strlen(received)-1;i++) {
                            received[i] = received[i+1];
                        }
                        received[n-1] = 0;
                    }
                }
                fprintf(arquivo, "%s",received);
            }

        }
    }
    close(conn); //fim da conexão
}
