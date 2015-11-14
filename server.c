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
    char buffer[INET6_ADDRSTRLEN];
    FILE *arquivo;
    pid_t child;

    if (argc != 2) {
        printf("%d argumentos\n",argc);
        printf("Argumentos necessarios: porta.\n");
        exit(1);
    }

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    ret = getaddrinfo(NULL,argv[1],&hints,&res);
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

    while (1) {
        len=sizeof(cliaddr);
        conn = accept(s,(struct sockaddr *)&cliaddr,&len);

        if(conn == -1) {
            perror("Erro ao aceitar conexão.\n");
            exit(1);
        }
        puts("Conectado ao cliente.\n");

        if ((child = fork())==0) { //caso seja processo filho
            close(s);
            n = recv(conn,received,sizeof(received),0);
            received[n] = 0;
            if(!strcmp(received, "READY")) {
                strcpy(sent,"READY ACK");
                send(conn,sent,strlen(sent),0);
                //recebimendo do nome do diretório
                n = recv(conn, received, 3, 0);
                received[n]=0;
                tamanho = atoi(received);
                n = recv(conn, received, tamanho, 0);
                received[n]=0;
                //abrindo o arquivo
                if (getnameinfo((struct sockaddr*)&cliaddr,len,buffer,sizeof(buffer),0,0,NI_NUMERICHOST)) { //conversão do ip do cliente para string
                    printf("failed to convert address to string");
                    exit(1);
                }
                strcpy(string, buffer); //host do cliente
                strcat(string, received); //nome do diretório
                strcat(string, ".txt");
                arquivo = fopen(string,"w"); //abre o arquivo
                //início do recebimento da lista de arquivos
                while (n = recv(conn,received,3,0)) { //lê primeiro o tamanho da mensagem que seguirá
                    received[n] = 0;
                    tamanho = atoi(received); //converte a string para inteiro, que será o tamanho do buffer da próxima leitura
                    n = recv(conn,received,tamanho,0);
                    received[n] = 0;
                    if (received[0]=='t'&&received[1]=='c'&&received[2]=='h'&&received[3]=='a') {
                        if (received[4]=='u') { //fim da comunicação, anunciado por "tchau"
                            fclose(arquivo); //salva e fecha o arquivo
                            close(conn); //fim da conexão
                        }
                        else { //bit stuffing. remover caracter inserido
                            int i;
                            for (i = 4; i<strlen(received)-1;i++) {
                                received[i] = received[i+1];
                            }
                            received[n-1] = 0;
                        }
                    }
                    puts(received);
                    fprintf(arquivo, "%s\n",received);
                }

            }
        }
    }

    return 0;
}
