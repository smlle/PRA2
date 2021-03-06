/***************************************************************************
Copyright (c) 2015 Bruna Oliveira, Leonardo Rosa Rodrigues, Rafael Rizzatti

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree.h"

/***************************************************************************************/
/* Equipe: Bruna de Oliveira    */
/*         Leonardo Rosa        */
/*         Rafael Rizzatti      */
/***************************************************************************************/

typedef struct{
    char nome_entidade[100];
    char **campos;
    char **tipos;
    int *tamanhos;
    int qtd_campos;
    int tamanho_header;
} tabela;

typedef struct{
    int id;
    int pos;
} heap;

FILE *arqGeral;
FILE *arqIndice;

tabela entidadeGeral;

void createFiles();//FUNCAO QUE CRIA OS ARQUIVOS
void writeEntity(char *nome);//FUNCAO QUE GRAVA ENTIDADES NOS ARQUIVOS
void readFiles();//FUNCAO QUE LE TODO O ARQUIVO E PRINTA NA TELA
void menuCRUD(char *nome); //SUBMENU
void carrega_entidadeGeral(char *nome); //CARREGA A ENTIDADE GERAL DE ACORDO COM O HEADER DO ARQUIVO
tabela findOne(char *nome, int ID);//FUNCAO QUE RETORNA A ENTIDADE CASO ACHE A CHAVE PRIMARIA
tabela *chargeEntity(char *nome);//FUNCAO QUE CARREGA O VETOR DE ENTIDADES
void changeEntity(char *nome); //FUNCAO QUE ALTERA O VALOR DO ID DE UM ELEMENTO
void removeEntity(char *nome);//FUNCAO QUE DELETA UMA ENTIDADE ATRAVES DO ID
void heapsort(heap a[], int n);//ORDENA OS ELEMENTOS ATRAVES DO INDICE
void ordena(char *nome);
void mostraIndex(char *nome);

int main(){
    createFiles();
    return 0;
}

void menu(char **vet_nome,int tam_vet){
    int opc,j;
    while(1){
        puts("\t\tMENU DE ENTIDADES\t\t\n");
        for(j=0;j<tam_vet;j++){
            printf("%i- %s\n",j+1,vet_nome[j]);
        }
        printf("0- Sair\n");
        printf(":>> ");
        scanf("%i",&opc);
        if(opc==0){
            break;
        }
        else if(opc>0 && opc<=tam_vet){
            menuCRUD(vet_nome[opc-1]);
        }
        else{
            puts("Opcao Invalida!\n");
        }
    }
}

void menuCRUD(char *nome){
    int opc;
    carrega_entidadeGeral(nome);
    puts("\t\tMENU CRUD\t\t\n");
    puts("1- Inserir um novo elemento\n");
    puts("2- Mostrar todos os elementos da entidade\n");
    puts("3- Alterar um elemento\n");
    puts("4- Remover um elemento\n");
    puts("5- Mostrar indices ordenados\n");
    puts("0- Sair");
    printf(":>> ");
    scanf("%i",&opc);
    if(opc==1){
        writeEntity(nome);
    }
    else if(opc==2){
        readFiles(nome);
    }
    else if(opc==3){
        changeEntity(nome);
    }
    else if(opc==4){
        removeEntity(nome);
    }
    else if(opc==5){
        mostraIndex(nome);
    }
    else if(opc==0)
        return;
    else{
        puts("Opcao Invalida!\n");
    }
}

void writeEntity(char *nome){
    char *string,aux[1000];
    int i,rowid;
    tabela auxTabela;
    string=aux;
    arqGeral=fopen(nome,"ab");
    rowid=ftell(arqGeral);
    sprintf(string,"%i",rowid);
    __fpurge(stdin);
    fwrite(string,sizeof(char),4,arqGeral);
    rowid=ftell(arqGeral);
    for(i=1;i<entidadeGeral.qtd_campos;i++){
        printf("Digite o %s:\n", entidadeGeral.campos[i]);
        if(strcmp(entidadeGeral.tipos[i],"int")==0){
            __fpurge(stdin);
            scanf("%[^\n]s",string);
            //fwrite(entidadeGeral.campos[i],sizeof(entidadeGeral.campos[i]),1,arqGeral);
            if(strcmp(entidadeGeral.campos[i],"id")==0){
                fclose(arqGeral);
                auxTabela=findOne(nome,atoi(string));
                arqGeral=fopen(nome,"ab");
                if(auxTabela.qtd_campos!=-1){
                    puts("Entidade com este ID ja existe!\nProcure digitar outro ID\n");
                    rowid=ftell(arqGeral);
                    rowid-=4;// desconta a gravacao do rowid que contem o id duplicado!
                    rewind(arqGeral);
                    fclose(arqGeral);
                    arqGeral=fopen(nome,"rb");
                    __fpurge(stdin);
                    fread(string,sizeof(char),rowid,arqGeral);
                    fclose(arqGeral);
                    arqGeral=fopen(nome,"wb");
                    fwrite(string,sizeof(char),rowid,arqGeral);
                    fclose(arqGeral);
                    return;
                }
            }
            fwrite(string,entidadeGeral.tamanhos[i],1,arqGeral);
            string[0]='\0';
        }
        else if(strcmp(entidadeGeral.tipos[i],"char")==0){
            __fpurge(stdin);
            scanf("%[^\n]s",string);
            if(strcmp(entidadeGeral.campos[i],"id")==0){
                auxTabela=findOne(nome,atoi(string));
                if(auxTabela.qtd_campos!=-1){
                    puts("Entidade com este ID ja existe!\nProcure digitar outro ID\n");
                }
                return;
            }
            //fwrite(entidadeGeral.campos[i],sizeof(entidadeGeral.campos[i]),1,arqGeral);
            fwrite(string,entidadeGeral.tamanhos[i],1,arqGeral);
            string[0]='\0';
        }
    }
    string[0]='\0';
    fclose(arqGeral);
    arqGeral=fopen(nome,"rb");
    fseek(arqGeral,0,SEEK_END);
    rowid=ftell(arqGeral);
    rewind(arqGeral);
    fread(string,sizeof(char),rowid,arqGeral);
    ordena(nome);
}

void createFiles(){//Funcao para criar os arquivos casa nao exista, caso exista abre para leitura
    char buffer[500],string[500],entidade[100],campos[500],tipo[500],tamanhos[500],relacao[500],localizacao[5],versao[10],aux[5],**vet_entidades;
    int qnt_campos,tamanho_header,i=0;
    FILE *fp;
    vet_entidades=(char **)malloc(sizeof(char *));
    arqGeral=fopen("configDB","r");
    if(!arqGeral){
        puts("Erro na abertura do arquivo do DataBase");
    }
    else{
        while(fgets(buffer,sizeof(buffer),arqGeral)){
            i++;
            strcat(buffer,"\0");
            if(sscanf(buffer,"qnt=%i,entidade=[%s ],qnt_campos=[%i ],campos=[%s ],tamanho=[%s ],tipo=[%s ],relacao=[%s ],localizacao=[%s ],versao=[%s ]",&tamanho_header,entidade,&qnt_campos,campos,tamanhos,tipo,relacao,localizacao,versao)==9){
                vet_entidades=(char **)realloc(vet_entidades,sizeof(char *)*i);
                vet_entidades[i-1]=(char *)malloc(sizeof(char)*100);
                strcpy(string,buffer);
                strcpy(vet_entidades[i-1],entidade);
                if((fp=fopen(entidade,"rb"))==NULL){
                    if((fp=fopen(entidade,"wb"))!=NULL){
                        fwrite(buffer,tamanho_header,1,fp);
                        fclose(fp);
                    }
                }
                else{
                    fseek(fp,tamanho_header-6,SEEK_SET);
                    fread(aux,sizeof(char),3,fp);
                    if(atof(versao)<atof(aux)){
                        //printf("\n\nERRO >> ARQUIVO CONTEM UMA VERSAO SUPERIOR A DA CONFIGURACAO DO BD\nENCERRANDO O PROGRAMA\n\n");
                        fclose(fp);
                        exit(0);
                    }
                    else if(atof(versao)==atof(aux)){
                        //printf("VERSAO DO ARQUIVO EM DIA\n");
                        fclose(fp);
                    }
                    else if(atof(versao)>atof(aux)){
                        FILE *fp2;
                        //printf("ARQUIVO DESATUALIZADO >> RECRIANDO\n");
                        if((fp2=fopen("aux","wb"))!=NULL){
                            fwrite(buffer,sizeof(char),tamanho_header,fp2);
                            fclose(fp2);
                        }
                        fclose(fp);
                        remove(entidade);
                        rename("aux",entidade);
                    }
                }
            }
            else{
                puts("Erro no sscanf!");
            }
        }
    }
    fclose(arqGeral);
    menu(vet_entidades,i);
}

void readFiles(char *nome){
    char string[1000]="\0";
    int tam_arq=0,i;
    arqGeral=fopen(nome,"rb");
    fseek(arqGeral,0,SEEK_END);
    tam_arq=ftell(arqGeral);
    rewind(arqGeral);
    tam_arq-=entidadeGeral.tamanho_header;
    fseek(arqGeral,entidadeGeral.tamanho_header,SEEK_CUR);
    if(tam_arq==0){
        puts("Arquivo Nao contem Entidades!");
    }
    else{
        while(tam_arq!=0){
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                if(strcmp(entidadeGeral.tipos[i],"int")==0){
                    fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                    printf("\t%s:\t%i",entidadeGeral.campos[i],atoi(string));
                }
                else if(strcmp(entidadeGeral.tipos[i],"char")==0){
                    fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                    printf("\t%s:\t%s",entidadeGeral.campos[i],string);
                }
                tam_arq-=entidadeGeral.tamanhos[i];
            }
            puts("");
        }
    }
    fclose(arqGeral);
}

tabela findOne(char *nome,int ID){
    tabela aux;
    char string[1000]="\0";
    int tam_arq=0,i;
    aux.campos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
    aux.tipos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
    aux.tamanhos=(int *)malloc(sizeof(int)*entidadeGeral.qtd_campos);
    for(i=0;i<entidadeGeral.qtd_campos;i++){
        aux.campos[i]=(char *)malloc(sizeof(char)*100);
        aux.tipos[i]=(char *)malloc(sizeof(char)*100);
    }
    arqGeral=fopen(nome,"rb");
    fseek(arqGeral,0,SEEK_END);
    tam_arq=ftell(arqGeral);
    rewind(arqGeral);
    tam_arq-=entidadeGeral.tamanho_header;
    tam_arq-=4;
    fseek(arqGeral,entidadeGeral.tamanho_header,SEEK_CUR);
    if(tam_arq!=0){
        while(tam_arq!=0){
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                if(strcmp(entidadeGeral.tipos[i],"int")==0){
                    fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                    strcpy(aux.campos[i],string);
                    strcpy(aux.tipos[i],entidadeGeral.tipos[i]);
                    aux.tamanhos[i]=entidadeGeral.tamanhos[i];
                    //printf("\t%s:\t%i",entidadeGeral.campos[i],atoi(string));
                }
                else if(strcmp(entidadeGeral.tipos[i],"char")==0){
                    fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                    strcpy(aux.tipos[i],entidadeGeral.tipos[i]);
                    strcpy(aux.campos[i],string);
                    aux.tamanhos[i]=entidadeGeral.tamanhos[i];
                    //printf("\t%s:\t%s",entidadeGeral.campos[i],string);
                }
                tam_arq-=entidadeGeral.tamanhos[i];
            }
            //puts("");
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                //printf("AAA: %i %s %i\n",atoi(aux.campos[i]),entidadeGeral.campos[i],ID);
                if(atoi(aux.campos[i])==ID && strcmp("id",entidadeGeral.campos[i])==0){
                    aux.qtd_campos=entidadeGeral.qtd_campos;
                    strcpy(aux.nome_entidade,entidadeGeral.nome_entidade);
                    aux.tamanho_header=entidadeGeral.tamanho_header;
                    fclose(arqGeral);
                    return aux;
                }
            }
        }
    }
    //printf("Entidade Nao Encontrada!\n");
    fclose(arqGeral);
    aux.qtd_campos=-1;
    return aux;
}

tabela *chargeEntity(char *nome){
    tabela *vet;
    char string[1000]="\0";
    int tam_arq=0,i,cont=0;
    vet=(tabela *)malloc(sizeof(tabela));
    arqGeral=fopen(nome,"rb");
    fseek(arqGeral,0,SEEK_END);
    tam_arq=ftell(arqGeral);
    rewind(arqGeral);
    tam_arq-=entidadeGeral.tamanho_header;
    fseek(arqGeral,entidadeGeral.tamanho_header,SEEK_CUR);
    if(tam_arq==0){
        puts("Arquivo Nao contem Entidades!");
    }
    else{
        while(tam_arq!=0){
            puts("OI");
            cont++;
            vet=(tabela *)realloc(vet,sizeof(tabela)*cont);
            vet[cont-1].campos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
            vet[cont-1].tipos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
            vet[cont-1].tamanhos=(int *)malloc(sizeof(int)*entidadeGeral.qtd_campos);
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                vet[cont-1].campos[i]=(char *)malloc(sizeof(char)*100);
                vet[cont-1].tipos[i]=(char *)malloc(sizeof(char)*100);
            }
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                strcpy(vet[cont-1].tipos[i],entidadeGeral.tipos[i]);
                strcpy(vet[cont-1].campos[i],string);
                vet[cont-1].tamanhos[i]=entidadeGeral.tamanhos[i];
                tam_arq-=entidadeGeral.tamanhos[i];
            }
            vet[cont-1].qtd_campos=entidadeGeral.qtd_campos;
        }
    }
    fclose(arqGeral);
    cont++;
    vet=(tabela *)realloc(vet,sizeof(tabela)*cont);
    vet[cont-1].qtd_campos=-1;
    return vet;
}

void changeEntity(char *nome){
    int ID,qnt_elementos=0,i,j,flag=0;
    tabela *vet;
    char novo[100],string[1000];
    puts("Digite o ID do elemento que deseja alterar");
    scanf("%i",&ID);
    vet=chargeEntity(nome);
    i=0;
    while(vet[i].qtd_campos!=-1){
        qnt_elementos++;
        i++;
    }
    for(i=0;i<qnt_elementos;i++){
        if(ID==atoi(vet[i].campos[1])){
            for(j=2;j<entidadeGeral.qtd_campos;j++){
                 __fpurge(stdin);
            	//fflush(stdin);
                printf("Digite o novo %s\n",entidadeGeral.campos[j]);
                scanf("%s",novo);
                strcpy(vet[i].campos[j],novo);
                novo[0]='\0';
            }
            flag=1;
            break;
        }
    }
    if(flag==0){
        puts("ID da entidade nao encontrado!");
        return;
    }
    else{
        arqGeral=fopen(nome,"rb");
        fread(string,sizeof(char),entidadeGeral.tamanho_header,arqGeral);
        fclose(arqGeral);
        arqGeral=fopen(nome,"wb");
        fwrite(string,sizeof(char),entidadeGeral.tamanho_header,arqGeral);
        for(i=0;i<qnt_elementos;i++){
            for(j=0;j<entidadeGeral.qtd_campos;j++){
                fwrite(vet[i].campos[j],entidadeGeral.tamanhos[j],1,arqGeral);
            }
        }
    }
    fclose(arqGeral);
}

void removeEntity(char *nome){
    int ID,qnt_elementos=0,i=0,j,flag=0,qtd_relacoes=0,tam_header,k=0;
    tabela *vet;
    FILE *fp;
    char string[1000],relacao[100],localizacao[100],nome2[100],nome3[100],**localizacoes,**relacoes,*aux;
    puts("Digite o ID do elemento que deseja remover");
    scanf("%i",&ID);
    carrega_entidadeGeral(nome);
    vet=chargeEntity(nome);
    i=0;
    while(vet[i].qtd_campos!=-1){
        qnt_elementos++;
        i++;
    }
    for(i=0;i<qnt_elementos;i++){
        if(ID==atoi(vet[i].campos[1])){
            flag=1;
            break;
        }
    }
    if(flag==0){
        puts("ID da entidade nao encontrado!");
        return;
    }
    else{
        arqGeral=fopen(nome,"rb");
        fread(string,sizeof(char),entidadeGeral.tamanho_header,arqGeral);
        sscanf(string,"qnt=%*i,entidade=[%*s ],qnt_campos=[%*i ],campos=[%*s ],tamanho=[%*s ],tipo=[%*s ],relacao=[%s ],localizacao=[%s ],versao=[%*s ]",relacao,localizacao);
        /*********************************************************************************/
        /*QUEBRANDO QUANTAS RELACOES TEMOS*/
        relacoes=(char **)malloc(sizeof(char *));
        relacoes[i]=(char *)malloc(sizeof(char )*100);
        aux=(char *)strtok(relacao,",");
        if(aux!=NULL){
            strcpy(relacoes[i],aux);
            while(aux!=NULL){
                i++;
                relacoes=(char **)realloc(relacoes,sizeof(char *)*i+1);
                relacoes[i]=(char *)malloc(sizeof(char )*100);
                aux= (char*)strtok(NULL, ",");
                if(aux!=NULL)
                    strcpy(relacoes[i],aux);
            }
        }
        else{
            strcpy(relacoes[i],relacao);
        }
        qtd_relacoes=i;
        /*FIM DA QUEBRA DE RELACOES*/
        /**********************************************************************************/
        /*QUEBRANDO QUANTAS LOCALIZACOES TEMOS*/
        i=0;
        localizacoes=(char **)malloc(sizeof(char *));
        localizacoes[i]=(char *)malloc(sizeof(char )*100);
        aux=(char *)strtok(localizacao,",");
        if(aux!=NULL){
            strcpy(localizacoes[i],aux);
            while(aux!=NULL){
                i++;
                localizacoes=(char **)realloc(localizacoes,sizeof(char *)*i+1);
                localizacoes[i]=(char *)malloc(sizeof(char )*100);
                aux= (char*)strtok(NULL, ",");
                if(aux!=NULL)
                    strcpy(localizacoes[i],aux);
            }
        }
        else{
            strcpy(localizacoes[i],localizacao);
        }
        __fpurge(stdin);
        /*FIM DA QUEBRA DE LOCALIZACOES*/
        /**********************************************************************************/
        /*INICIO DA EXCLUSAO NO ARQUIVO PRINCIPAL*/
        fclose(arqGeral);
        arqGeral=fopen(nome,"wb");
        fwrite(string,sizeof(char),entidadeGeral.tamanho_header,arqGeral);
        for(i=0;i<qnt_elementos;i++){
            for(j=0;j<entidadeGeral.qtd_campos;j++){
                if(ID!=atoi(vet[i].campos[1]))
                    fwrite(vet[i].campos[j],entidadeGeral.tamanhos[j],1,arqGeral);
            }
        }
        fclose(arqGeral);
        /*FIM DA EXCLUSAO NO ARQUIVO PRINCIPAL*/
        /*************************************************************************************/
        /*EXCLUIR DOS ARQUIVOS DE RELACOES*/
        for(k=0;k<qtd_relacoes;k++){
            carrega_entidadeGeral(relacoes[k]);
            vet=chargeEntity(relacoes[k]);
            strcpy(nome2,relacoes[k]);
            strcpy(nome3,relacoes[k]);
            strcat(nome2,".bin");
            rename(relacoes[k],nome2);
            strcat(relacoes[k],".bin");
            if((arqGeral=fopen(relacoes[k],"rb"))!=NULL){
                char auxiliar[1000];
                if((fp=fopen("aux","wb"))!=NULL){
                    fread(auxiliar,sizeof(char),entidadeGeral.tamanho_header,arqGeral);
                    fwrite(auxiliar,sizeof(char),entidadeGeral.tamanho_header,fp);
                    i=0;
                    while(vet[i].qtd_campos!=-1){
                        puts("oi");
                        for(j=0;j<entidadeGeral.qtd_campos;j++){
                            if(ID!=atoi(vet[i].campos[atoi(localizacoes[k])-1]))
                                fwrite(vet[i].campos[j],entidadeGeral.tamanhos[j],1,fp);
                        }
                        i++;
                    }
                }
                fclose(fp);
            }
            fclose(arqGeral);
            remove(nome2);
            rename("aux",nome3);
        }
        /*FIM DE EXCLUIR DOS ARQUIVOS DE RELACOES*/
        /*************************************************************************************/
        /*EXCLUIR DO ARQUIVO DE INDICES*/
        carrega_entidadeGeral(nome);
        ordena(nome);
        /*FIM EXCLUSAO DOS INDICES*/
        /*************************************************************************************/
    }
    puts("Entidade Removida com Sucesso!");
}

void carrega_entidadeGeral(char *nome){
    char buffer[1000],auxiliar[50];
    int tamanho,i,j,k,cont=0,cont_campos,cont_tamanhos,cont_tipos;
    if((arqGeral=fopen(nome,"rb"))!=NULL){
        fseek(arqGeral,4,SEEK_SET);//andamos para pegar os valores do total do header
        fread(&buffer,sizeof(char),3,arqGeral);//lemos o valor
        buffer[3]='\0';//fechamos a string
        tamanho=atoi(buffer);
        entidadeGeral.tamanho_header=tamanho;
        tamanho-=8;
        fseek(arqGeral,8,SEEK_SET);
        fread(&buffer,sizeof(char),tamanho,arqGeral);
        fclose(arqGeral);//como ja temos o header salvo no buffer, fechamos o arquivo
        for(i=0;i<tamanho;i++){
            j=0;
            cont_campos=0;
            cont_tipos=0;
            cont_tamanhos=0;
            if(buffer[i]=='['){
                auxiliar[0]='\0';
                i++;
                cont++;
                while(buffer[i]!=']' && buffer[i]!=' '){
                    if(buffer[i]==','){
                        if(cont==3){
                            entidadeGeral.campos[cont_campos]=(char *)malloc(sizeof(char)*100);
                            for(k=0;auxiliar[k]!='\0';k++){
                                entidadeGeral.campos[cont_campos][k]=auxiliar[k];
                                entidadeGeral.campos[cont_campos][k+1]='\0';
                            }
                            cont_campos++;
                            j=0;
                            auxiliar[0]='\0';
                        }
                        else if(cont==4){
                            entidadeGeral.tamanhos[cont_tamanhos]=atoi(auxiliar);
                            cont_tamanhos++;
                            j=0;
                            auxiliar[0]='\0';
                        }
                        else if(cont==5){
                            entidadeGeral.tipos[cont_tipos]=(char *)malloc(sizeof(char)*100);
                            for(k=0;auxiliar[k]!='\0';k++){
                                entidadeGeral.tipos[cont_tipos][k]=auxiliar[k];
                                entidadeGeral.tipos[cont_tipos][k+1]='\0';
                            }
                            cont_tipos++;
                            j=0;
                            auxiliar[0]='\0';
                        }
                    }
                    else{
                        auxiliar[j]=buffer[i];
                        j++;
                        auxiliar[j]='\0';
                    }
                    i++;
                }
                if(cont==1){
                    strcpy(entidadeGeral.nome_entidade,auxiliar);
                    auxiliar[0]='\0';
                }
                else if(cont==2){
                    entidadeGeral.qtd_campos=atoi(auxiliar);
                    entidadeGeral.campos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
                    entidadeGeral.tipos=(char **)malloc(sizeof(char *)*entidadeGeral.qtd_campos);
                    entidadeGeral.tamanhos=(int *)malloc(sizeof(int)*entidadeGeral.qtd_campos);
                    auxiliar[0]='\0';
                }
                else if(cont==3){
                    entidadeGeral.campos[cont_campos]=(char *)malloc(sizeof(char)*100);
                    for(k=0;auxiliar[k]!='\0';k++){
                        entidadeGeral.campos[cont_campos][k]=auxiliar[k];
                        entidadeGeral.campos[cont_campos][k+1]='\0';
                    }
                    auxiliar[0]='\0';
                }
                else if(cont==4){
                    entidadeGeral.tamanhos[cont_tamanhos]=atoi(auxiliar);
                    auxiliar[0]='\0';
                }
                else if(cont==5){
                    entidadeGeral.tipos[cont_tipos]=(char *)malloc(sizeof(char)*100);
                    for(k=0;auxiliar[k]!='\0';k++){
                        entidadeGeral.tipos[cont_tipos][k]=auxiliar[k];
                        entidadeGeral.tipos[cont_tipos][k+1]='\0';
                    }
                    auxiliar[0]='\0';
                }
            }
        }
    }
}

void ordena(char *nome){
    char string[1000]="\0", index[100]="\0";
    int tam_arq=0, tam_arq_aux=0,i, count=0, ii=0, pos;
    arqGeral=fopen(nome,"rb");
    fseek(arqGeral,0,SEEK_END);
    tam_arq=ftell(arqGeral);
    rewind(arqGeral);
    tam_arq-=entidadeGeral.tamanho_header;
    tam_arq_aux = tam_arq;
    fseek(arqGeral,entidadeGeral.tamanho_header,SEEK_CUR);
    if(tam_arq==0){
        puts("Arquivo Nao contem Entidades!");
    }
    else{
        while(tam_arq!=0){
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                tam_arq-=entidadeGeral.tamanhos[i];
            }
	        count++;
        }
        heap vet[count];

        fseek(arqGeral,entidadeGeral.tamanho_header,SEEK_SET);

        pos = entidadeGeral.tamanho_header;

        while(tam_arq_aux!=0){
            for(i=0;i<entidadeGeral.qtd_campos;i++){
                fread(string,sizeof(char),entidadeGeral.tamanhos[i],arqGeral);
                if(strcmp(entidadeGeral.tipos[i],"int")==0){
                    if((strcmp(entidadeGeral.campos[i],"id")==0)){
                         vet[ii].id = atoi(string);
                    }
                }
                tam_arq_aux-=entidadeGeral.tamanhos[i];
            }
            vet[ii].pos = pos;
            pos = ftell(arqGeral);
            ii++;
        }
        fclose(arqGeral);

        for(i=0;i<count;i++){
            printf("\nid - %d, pos - %d", vet[i].id, vet[i].pos);
            insertion(vet[i].id,vet[i].pos);
        }
        strcpy(index, nome);
        strcat(index, "Index");
        if((arqIndice=fopen(index,"wb"))!=NULL){
            fclose(arqIndice);
            traversal(root,index);
        }
    }
    root=NULL;
}

void mostraIndex(char *nome){
    char nomea[100];
    heap aux;
    strcpy(nomea,nome);
    strcat(nomea,"Index");
    arqIndice = fopen(nomea, "rb");
    if(arqIndice!=NULL){
        while(!feof(arqIndice)){
            fread(&aux,sizeof(heap),1,arqIndice);
            printf("\nid: %d, pos: %d", aux.id, aux.pos);
        }
        __fpurge(stdout);
        fclose(arqIndice);
    }
    else{
        puts("Nao contem nenhum elemento na entidade!");
    }
    //fflush(stdout);  //pra windows
}
