//
//  main.c
//  TP2
//
//  Created by Juscelino Tanaka on 22/01/14.
//  Copyright (c) 2014 Juscelino Tanaka. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>


// 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51/

// 1 2 3 4 5 6 7 8 9/

// alteracao Henrique

// 15i 16i 17i 18i 19i 20i 21i 22i 23i 24i 25i
// 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51/

// alteração Tanaka


/*
 btree.C
 
 Demonstration program for a B-tree on disk.  After building the B-tree
 by entering integers on the keyboard or by supplying these as a text file,
 we can update it interactively by entering (or deleting) individual
 integers.  Instead, we can search the B-tree for a given integer.  Each
 time, the tree or a search path is displayed.  Program DISKTREE.CPP writes,
 reads, and updates nodes on disk using a binary file.  The name of this
 file is to be entered on the keyboard.  If a B-tree file with that name
 exists, that B-tree is used; otherwise such a file is created.
 Caution:
 Do not confuse the (binary) file for the B-tree with the optional text
 file for input data.  Use different file-name extensions, such as .BIN
 and .TXT.
 
 fonte: http://www.bitcore.org/stuff/school/programming%20courses/csc345/btree.c
 
 google: b-tree disk c
 
 */


#define DEBUGANDO 0

#define M 2
#define MM 4
#define NIL (-1L)
#define T 7


#define NUMERO_BUCKETS 3000
#define BLOCOS_POR_BUCKET 100
#define TAMANHO_BLOCO 4096
#define REGISTROS_POR_BLOCO 8

// Estrutura dos artigos
typedef struct artigo{
    int id;
    char sigla[19];
    char titulo[300];
    int ano;
    char autores[100];
    int citacoes;
    char citepage[30];
    char timestamp[20];
}tArtigo;


typedef enum {
    INSERTNOTCOMPLETE, SUCCESS, DUPLICATEKEY,
    UNDERFLOW, NOTFOUND
} status;


typedef struct {
    int valor;
    int hash;
} campo;


typedef struct{
    int cnt;
    campo key[MM];
    long ptr[MM+1];
} node;


typedef struct {
    char valor[301];
    int hash;
} campoTitulo;


typedef struct{
    int cnt;
    campoTitulo key[MM];
    long ptr[MM+1];
} nodeTitulo;


node rootnode;
long start[2], root=NIL, freelist=NIL;
FILE *fptree, *titulotree;

nodeTitulo rootnodeTitulo;
long startTitulo[2], rootTitulo=NIL, freelistTitulo=NIL;
FILE *fptreeTitulo;


FILE *arquivoHash;

/************************FUNCTION PROTOTYPES****************************/
void error(char *str);
void readnode(long t, node *pnode);
void writenode(long t, node *pnode);
long getnode(void);
int found(long t,  int i);
void notfound(int x);
int binsearch(int x, campo *a, int n);
int search(int x);
status ins(int x, int hash, long t, int *y, int *hashnew, long *u);
status insert(int x, int hash);
void freenode(long t);
void rdstart(void);
void wrstart(void);
void printtree(long t);
/**********************END FUNCTION PROTOTYPES***************************/

void readnodeTitulo(long t, nodeTitulo *pnodeTitulo);
void writenodeTitulo(long t, nodeTitulo *pnodeTitulo);
long getnodeTitulo(void);
int binsearchTitulo(char x[301], campoTitulo *a, int n);
int searchTitulo(char x[301]);
status insTitulo(char x[301], int hash, long t, char *y, int *z, long *u);
status insertTitulo(char x[301], int hash);
void freenodeTitulo(long t);
void rdstartTitulo(void);
void wrstartTitulo(void);
void printtreeTitulo(long t);

int foundTitulo(long t,  int i);

/**********************END FUNCTION PROTOTYPES***************************/


void inicializarHash();
void upload(char * path);
int calculaChave (int id);
int buscarArtigo(int id, int chave, int blocosLidos);
int buscarArtigoPorTitulo(char titulo[301], int chave, int blocosLidos);


int hashString(char *str)
{
    int hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}

//upa string para maiusculo
void convertToUpperCase(char *sPtr)
{
    while(*sPtr != '\0')
        *sPtr = toupper((unsigned char)*sPtr);
}

char * idpath = "indexid.bin";
char * titulopath = "indextitulo.bin";

char msgStatus[30];
int  sysStatus;

tArtigo aBuscado;

int chaveGlobal;

void setStatus(int sts) {
    sysStatus = sts;
    
    switch (sts) {
        case 0:
            strcpy(msgStatus, "BASE VAZIA. FAZER UPLOAD.");
            break;
        case 1:
            strcpy(msgStatus, "BASE POPULADA.");
            break;
            
        default:
            
            break;
    }
    
}

void imprimirArtigo(tArtigo art) {
    printf("ID: %d\n", art.id);
    printf("Sigla: %s\n", art.sigla);
    printf("Titulo: %s\n", art.titulo);
    printf("Ano: %d\n", art.ano);
    printf("Autores: %s\n", art.autores);
    printf("Citacoes: %d\n", art.citacoes);
    printf("Citepage: %s\n", art.citepage);
    printf("Timestamp: %s\n\n", art.timestamp);
}

void openID() {
    fptree = fopen(idpath, "r+b");
    if (fptree == NULL) {
        // abre o arquivo
        fptree = fopen(idpath, "w+b");
        wrstart();
    }else{
        struct stat st;
        stat(idpath, &st);
        long long int size = st.st_size;
        //verifica se o arquivo ja foi criado mas esta vazio
        if (size > 0) {
            //continua de onde parou os dados
            rdstart();
        } else {
            //prepara para primeiro inicio
            wrstart();
        }
        //printtree(root);
    }
}

void openTitulo() {
    fptreeTitulo = fopen(titulopath, "r+b");
    if (fptreeTitulo == NULL) {
        // abre o arquivo
        fptreeTitulo = fopen(titulopath, "w+b");
        wrstartTitulo();
    }else{
        struct stat st;
        stat(idpath, &st);
        long long int size = st.st_size;
        //verifica se o arquivo ja foi criado mas esta vazio
        if (size > 0) {
            //continua de onde parou os dados
            rdstartTitulo();
        } else {
            //prepara para primeiro inicio
            wrstartTitulo();
        }
        //printtreeTitulo(rootTitulo);
    }
}

void closeID() {
    wrstart();
    fclose(fptree);
}

void closeTitulo() {
    wrstartTitulo();
    fclose(fptreeTitulo);
}

int main()
{
    char acao, uploadpath[51];
    int achou, idbusca;
    
    printf("INICIALIZANDO... (ESTE PROCESSO PODE DEMORAR NA PRIMEIRA EXECUCAO)");
    printf("\n");
    inicializarHash();
    
    printf("VERIFICANDO STATUS DA BASE...");
    printf("\n");
    openID();
    achou = search(1);
    closeID();
    
    //se achou o registro 1 significa que a base ja esta populada
    if (achou == -1) { // -1 == nao encontrou
        setStatus(0);
    } else {
        setStatus(1);
    }
    
    do {
        printf("*****************************************************\n");
        printf("* STATUS DO SISTEMA: %30s *\n", msgStatus);
        printf("*****************************************************\n");
        printf("* SELECIONE A OPCAO DESEJADA                        *\n");
        printf("* 1 - UPLOAD ARQUIVO                                *\n");
        printf("* 2 - BUSCA POR ID NO ARQUIVO DE DADOS              *\n");
        printf("* 3 - BUSCA POR ID NO ARQUIVO DE INDICE             *\n");
        printf("* 4 - BUSCA POR TITULO NO ARQUIVO DE INDICE         *\n");
        printf("*                                                   *\n");
        printf("* 0 - SAIR                                          *\n");
        printf("*****************************************************\n");
        printf("* Opcao: ");
        
        scanf("%c", &acao);
        
        // printf("\n");
        
        switch (acao) {
            case '0':
                printf("BANCO DE DADOS ENCERRADO!\n");
                break;
                
            case '1': //upload
                printf("Nome do arquivo de entrada: ");
                scanf("%50s", uploadpath);
                
                upload(uploadpath);
                break;
                
            case '2': //findrec <id>
                if (sysStatus == 1) {
                    printf("BUSCA POR ID NO ARQUIVO DE DADOS\n\nID: ");
                    scanf("%d", &idbusca);
                    
                    achou = buscarArtigo(idbusca, calculaChave(idbusca), 0);
                    if (!achou) {
                        printf("REGISTRO NAO ENCONTRADO.\n");
                    } else {
                        printf("REGISTRO ENCONTRADO:\n\n");
                        imprimirArtigo(aBuscado);
                    }
                    
                } else {
                    printf("BASE NAO POPULADA. FAZER UPLOAD DO ARQUIVO DE DADOS PRIMEIRO.\n");
                }
                break;
                
            case '3': //seek1 <id>
                if (sysStatus == 1) {
                    printf("BUSCA POR ID NO ARQUIVO DE INDICE\n\nID: ");
                    scanf("%d", &idbusca);
                    
                    openID();
                    int resultadoBusca = search(idbusca);
                    closeID();
                    
                    if (resultadoBusca == -1) {
                        printf("REGISTRO NAO ENCONTRADO NA ARVORE!\n");
                    } else {
                        achou = buscarArtigo(idbusca, calculaChave(idbusca), 0);
                        if (!achou) {
                            printf("REGISTRO NAO ENCONTRADO NO ARQUIVO DE DADOS.\n");
                        } else {
                            printf("REGISTRO ENCONTRADO:\n\n");
                            imprimirArtigo(aBuscado);
                        }
                    }
                    
                    
                } else {
                    printf("BASE NAO POPULADA. FAZER UPLOAD DO ARQUIVO DE DADOS PRIMEIRO.\n");
                }
                
                break;
                
            case '4': //seek2 <titulo>
                if (sysStatus == 1) {
                    //Building Semantic Mappings from Databases to Ontologies
                    
                    //Low-cost Outdoor Robot Platform for the Penn State Abington Mini Grand Challenge
                    printf("BUSCA POR TITULO NO ARQUIVO DE INDICE\n\nTITULO: ");
                    char tituloBusca[301];
                    getchar();
                    gets(tituloBusca);
                    
                    printf("Buscando: ");
                    puts(tituloBusca);
                    
                    openTitulo();
                    int resultadoBusca = searchTitulo(tituloBusca);
                    closeTitulo();
                    
                    if (resultadoBusca == -1) {
                        printf("REGISTRO NAO ENCONTRADO NA ARVORE!\n");
                    } else {
                        achou = buscarArtigoPorTitulo(tituloBusca, calculaChave(idbusca), 0);
                        if (!achou) {
                            printf("REGISTRO NAO ENCONTRADO NO ARQUIVO DE DADOS.\n");
                        } else {
                            printf("REGISTRO ENCONTRADO (%d):\n\n", resultadoBusca);
                            imprimirArtigo(aBuscado);
                        }
                    }
                    
                    
                } else {
                    printf("BASE NAO POPULADA. FAZER UPLOAD DO ARQUIVO DE DADOS PRIMEIRO.\n");
                }
                break;
                
            case '5':
                openID();
                printtree(root);
                closeID();
                break;
                
            case '6':
                openTitulo();
                printtreeTitulo(rootTitulo);
                closeTitulo();
                break;
                
            default:
                printf("OPCAO INVALIDA. ESCOLHA OUTRA OPCAO\n");
                break;
        }
        
        if (acao == '0') {
            break;
        }
        
        getchar();
        
    } while (acao != 0);
    
    return(0);
}



void error(char *str)
{
    printf("\nError: %s\n", str);
    exit(1);
}



void readnode(long t, node *pnode)
{
    if (t == root){
        *pnode = rootnode;
        return;
    }
    if (fseek(fptree, t, SEEK_SET))
        error("fseek in readnode 1");
    if (fread(pnode, sizeof(node), 1, fptree) == 0)
        error("fread in readnode 2");
}


void writenode(long t, node *pnode)
{
    if (t == root)
        rootnode = *pnode;
    if (fseek(fptree, t, SEEK_SET))
        error("fseek in writenode");
    if (fwrite(pnode, sizeof(node), 1, fptree) == 0)
        error("fwrite in writenode");
}



long getnode(void)
{
    long t;
    node nod;
    
    if (freelist == NIL) {
        if (fseek(fptree, 0L, SEEK_END))
            error("fseek in getnode");
        t = ftell(fptree);
        writenode(t, &nod);  }         /*  Allocate space on disk  */
    
    else{
        t = freelist;
        readnode(t, &nod);             /*  To update freelist      */
        freelist = nod.ptr[0];
    }
    return(t);
}



int found(long t,  int i)
{
    node nod;
    
    //printf("Found in position %d of node with contents:  ", i);
    readnode(t, &nod);
    //printf("  %d : %d\n\n", nod.key[i].valor, nod.key[i].hash);
    
    return nod.key[i].hash;
    
    //    for (i=0; i < nod.cnt; i++)
    //        printf("  %d", nod.key[i].valor);
    //    puts("");
    
}

int foundTitulo(long t,  int i)
{
    nodeTitulo nod;
    
    //printf("Found in position %d of node with contents:  ", i);
    readnodeTitulo(t, &nod);
    //printf("  %d : %d\n\n", nod.key[i].valor, nod.key[i].hash);
    
    return nod.key[i].hash;
    
    //    for (i=0; i < nod.cnt; i++)
    //        printf("  %d", nod.key[i].valor);
    //    puts("");
    
}




void notfound(int x)
{
    printf("Item %d not found\n", x);
}




int binsearch(int x, campo *a, int n)
{
    int i, left, right;
    
    if (x <= a[0].valor)
        return 0;
    if (x > a[n-1].valor)
        return n;
    left = 0;
    right = n-1;
    while (right -  left > 1){
        i = (right + left)/2;
        if (x <= a[i].valor)
            right = i;
        else
            left = i;
    }
    return(right);
}


int search(int x)
{
    int i, n, nosLidos = 0;
    campo *k;
    node nod;
    long t = root;
    
    puts("RESULTADO DA BUSCA:\n");
    while (t != NIL){
        nosLidos++;
        readnode(t, &nod);
        k = nod.key;
        //printf("Tnk: %d\n", k[0].valor);
        n = nod.cnt;
        //        for (j=0; j < n; j++)
        //            printf("  %d:%d", k[j].valor, k[j].hash);
        //        puts("");
        i = binsearch(x, k, n);
        if (i < n && x == k[i].valor){
            printf("NOS LIDOS: %d\n\n", nosLidos);
            return(found(t,i));
        }
        t = nod.ptr[i];
    }
    return(-1);
}



/*
 Insert x in B-tree with root t.  If not completely successful, the
 integer *y and the pointer *u remain to be inserted.
 */

//        valor / rooot / xnew  / tnew
status ins(int x, int hash, long t, int *y, int *z, long *u)
{
    long tnew, p_final, *p;
    int i, j, *n, k_final, hash_final, xnew, hashnew;
    campo *k;
    status code;
    node nod, newnod;
    
    /*  Examine whether t is a pointer member in a leaf  */
    if (t == NIL){
        *u = NIL;
        *y = x;
        *z = hash;
        return(INSERTNOTCOMPLETE);
    }
    
    readnode(t, &nod);
    n = & nod.cnt;
    k = nod.key;
    p = nod.ptr;
    /*  Select pointer p[i] and try to insert x in  the subtree of whichp[i]
     is  the root:  */
    i = binsearch(x, k, *n);
    if (i < *n && x == k[i].valor)
        return(DUPLICATEKEY);
    code = ins(x, hash, p[i], &xnew, &hashnew, &tnew);
    if (code != INSERTNOTCOMPLETE)
        return code;
    /* Insertion in subtree did not completely succeed; try to insert xnew and
     tnew in the current node:  */
    if (*n < MM) {
        i = binsearch(xnew, k, *n);
        for (j = *n; j > i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        k[i].valor = xnew;
        k[i].hash = hashnew;
        p[i+1] = tnew;
        ++*n;
        writenode(t, &nod);
        return(SUCCESS);
    }
    /*  The current node was already full, so split it.  Pass item k[M] in the
     middle of the augmented sequence back through parameter y, so that it
     can move upward in the tree.  Also, pass a pointer to the newly created
     node back through u.  Return INSERTNOTCOMPLETE, to report that insertion
     was not completed:    */
    if (i == MM){
        k_final = xnew;
        hash_final = hashnew;
        p_final = tnew;
    }else{
        k_final = k[MM-1].valor;
        hash_final = k[MM-1].hash;
        p_final = p[MM];
        for (j=MM-1; j>i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        k[i].valor = xnew;
        k[i].hash = hashnew;
        p[i+1] = tnew;
    }
    *y = k[M].valor;
    *z = k[M].hash;
    *n = M;
    *u = getnode();
    newnod.cnt = M;
    for (j=0; j< M-1; j++){
        newnod.key[j] = k[j+M+1];
        newnod.ptr[j] = p[j+M+1];
    }
    newnod.ptr[M-1] = p[MM];
    newnod.key[M-1].valor = k_final;
    newnod.key[M-1].hash = hash_final;
    newnod.ptr[M] = p_final;
    writenode(t, &nod);
    writenode(*u, &newnod);
    return(INSERTNOTCOMPLETE);
}


/*  Driver function for node insertion, called only in the main function.
 
 Most of the work is delegated to 'ins'.
 */
// x = chave/valor
status insert(int x, int hash)
{
    
    long tnew, u;
    int xnew, hashnew;
    status code = ins(x, hash, root, &xnew, &hashnew, &tnew);
    
    if (code == DUPLICATEKEY) {
        if (DEBUGANDO) {
            
            printf("Duplicate uid %d ignored \n", x);
            exit(0);
        }
    } else
        if (code == INSERTNOTCOMPLETE){
            u = getnode();
            rootnode.cnt = 1;
            rootnode.key[0].valor = xnew;
            rootnode.key[0].hash = hashnew;
            rootnode.ptr[0] = root;
            rootnode.ptr[1] = tnew;
            root = u;
            writenode(u, &rootnode);
            code = SUCCESS;
        }
    return(code);     /*  return value: SUCCESS  of DUPLICATEKEY  */
}



void freenode(long t)
{
    node nod;
    
    readnode(t, &nod);
    nod.ptr[0] = freelist;
    freelist = t;
    writenode(t, &nod);
}



void rdstart(void)
{
    
    if (fseek(fptree, 0L, SEEK_SET))
        error("fseek in rdstart 1");
    if (fread(start, sizeof(long), 2, fptree) == 0)
        error("fread in rdstart 2");
    readnode(start[0], &rootnode);
    root = start[0];
    freelist = start[1];
}

void wrstart(void)
{
    
    start[0] = root;
    start[1] = freelist;
    if (fseek(fptree, 0L, SEEK_SET))
        error("fseek in wrstart");
    if (fwrite(start, sizeof(long), 2, fptree) == 0)
        error("fwrite in wrstart");
    if (root != NIL)
        writenode(root, &rootnode);
}

void printtree(long t)
{
    static int position=0;
    int i, n;
    campo *k;
    node nod;
    
    if (t != NIL){
        position += 6;
        readnode(t, &nod);
        k = nod.key;
        n = nod.cnt;
        printf("%*s", position, "");
        for (i=0; i<n; i++)
            printf(" %d:%d", k[i].valor, k[i].hash);
        puts("");
        for (i=0; i<=n; i++)
            printtree(nod.ptr[i]);
        position -= 6;
    }
}


/* */























void readnodeTitulo(long t, nodeTitulo *pnodeTitulo)
{
    if (t == rootTitulo){
        *pnodeTitulo = rootnodeTitulo;
        return;
    }
    if (fseek(fptreeTitulo, t, SEEK_SET))
        error("fseek in readnodeTitulo 1");
    if (fread(pnodeTitulo, sizeof(nodeTitulo), 1, fptreeTitulo) == 0)
        error("fread in readnodeTitulo 2");
}


void writenodeTitulo(long t, nodeTitulo *pnodeTitulo)
{
    if (t == rootTitulo)
        rootnodeTitulo = *pnodeTitulo;
    if (fseek(fptreeTitulo, t, SEEK_SET))
        error("fseek in writenodeTitulo");
    if (fwrite(pnodeTitulo, sizeof(nodeTitulo), 1, fptreeTitulo) == 0)
        error("fwrite in writenodeTitulo");
}



long getnodeTitulo(void)
{
    long t;
    nodeTitulo nod;
    
    if (freelistTitulo == NIL) {
        if (fseek(fptreeTitulo, 0L, SEEK_END))
            error("fseek in getnodeTitulo");
        t = ftell(fptreeTitulo);
        writenodeTitulo(t, &nod);  }         /*  Allocate space on disk  */
    
    else{
        t = freelistTitulo;
        readnodeTitulo(t, &nod);             /*  To update freelistTitulo      */
        freelistTitulo = nod.ptr[0];
    }
    return(t);
}

int binsearchTitulo(char x[301], campoTitulo *a, int n)
{
    int i, left, right;
    
    if (x < a[0].valor || !strcmp(x, a[0].valor))
        return 0;
    if (x > a[n-1].valor)
        return n;
    left = 0;
    right = n-1;
    while (right -  left > 1){
        i = (right + left)/2;
        if (x < a[i].valor || !strcmp(x, a[i].valor))
            right = i;
        else
            left = i;
    }
    return(right);
}


int searchTitulo(char x[300])
{
    int i, n, nosLidos = 0;
    campoTitulo *k;
    nodeTitulo nod;
    long t = rootTitulo;
    
    puts("RESULTADO DA BUSCA:\n");
    while (t != NIL){
        nosLidos++;
        readnodeTitulo(t, &nod);
        k = nod.key;
        n = nod.cnt;
        i = binsearchTitulo(x, k, n);
        if (i < n && !strcmp(x, k[i].valor)) {
            printf("NOS LIDOS: %d\n\n", nosLidos);
            return(foundTitulo(t,i));
        }
        t = nod.ptr[i];
    }
    return(-1);
}



/*
 insertTitulo x in B-tree with rootTitulo t.  If not completely successful, the
 integer *y and the pointer *u remain to be inserted.
 */

//        valor / rooot / xnew  / tnew
status insTitulo(char x[301], int hash, long t, char *y, int *z, long *u)
{
    long tnew, p_final, *p;
    int i, j, *n, hash_final, hashnew;
    
    char xnew[301], k_final[301];
    campoTitulo *k;
    status code;
    nodeTitulo nod, newnod;
    
    /*  Examine whether t is a pointer member in a leaf  */
    if (t == NIL){
        *u = NIL;
        strcpy(y, x);
        *z = hash;
        return(INSERTNOTCOMPLETE);
    }
    
    readnodeTitulo(t, &nod);
    n = & nod.cnt;
    k = nod.key;
    p = nod.ptr;
    /*  Select pointer p[i] and try to insertTitulo x in  the subtree of whichp[i]
     is  the rootTitulo:  */
    i = binsearchTitulo(x, k, *n);
    if (i < *n && !strcmp(x,k[i].valor))
        return(DUPLICATEKEY);
    code = insTitulo(x, hash, p[i], xnew, &hashnew, &tnew);
    if (code != INSERTNOTCOMPLETE)
        return code;
    /* Insertion in subtree did not completely succeed; try to insertTitulo xnew and
     tnew in the current nodeTitulo:  */
    if (*n < MM) {
        i = binsearchTitulo(xnew, k, *n);
        for (j = *n; j > i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        strcpy(k[i].valor, xnew);
        k[i].hash = hashnew;
        p[i+1] = tnew;
        ++*n;
        writenodeTitulo(t, &nod);
        return(SUCCESS);
    }
    /*  The current nodeTitulo was already full, so split it.  Pass item k[M] in the
     middle of the augmented sequence back through parameter y, so that it
     can move upward in the tree.  Also, pass a pointer to the newly created
     nodeTitulo back through u.  Return INSERTNOTCOMPLETE, to report that insertion
     was not completed:    */
    if (i == MM){
        strcpy(k_final, xnew);
        hash_final = hashnew;
        p_final = tnew;
    }else{
        strcpy(k_final, k[MM-1].valor);
        hash_final = k[MM-1].hash;
        p_final = p[MM];
        for (j=MM-1; j>i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        strcpy(k[i].valor, xnew);
        k[i].hash = hashnew;
        p[i+1] = tnew;
    }
    strcpy(y, k[M].valor);
    *z = k[M].hash;
    *n = M;
    *u = getnodeTitulo();
    newnod.cnt = M;
    for (j=0; j< M-1; j++){
        newnod.key[j] = k[j+M+1];
        newnod.ptr[j] = p[j+M+1];
    }
    newnod.ptr[M-1] = p[MM];
    strcpy(newnod.key[M-1].valor, k_final);
    newnod.key[M-1].hash = hash_final;
    newnod.ptr[M] = p_final;
    writenodeTitulo(t, &nod);
    writenodeTitulo(*u, &newnod);
    return(INSERTNOTCOMPLETE);
}


/*  Driver function for nodeTitulo insertion, called only in the main function.
 
 Most of the work is delegated to 'insTitulo'.
 */
// x = chave/valor
status insertTitulo(char x[301], int hash)
{
    
    long tnew, u;
    int hashnew;
    char xnew[301];
    status code = insTitulo(x, hash, rootTitulo, xnew, &hashnew, &tnew);
    
    if (code == DUPLICATEKEY) {
        if (DEBUGANDO) {
            
            printf("Duplicate uid ignored: ");
            puts(x);
            exit(0);
        }
    } else
        if (code == INSERTNOTCOMPLETE){
            u = getnodeTitulo();
            rootnodeTitulo.cnt = 1;
            strcpy(rootnodeTitulo.key[0].valor, xnew);
            rootnodeTitulo.key[0].hash = hashnew;
            rootnodeTitulo.ptr[0] = rootTitulo;
            rootnodeTitulo.ptr[1] = tnew;
            rootTitulo = u;
            writenodeTitulo(u, &rootnodeTitulo);
            code = SUCCESS;
        }
    return(code);     /*  return value: SUCCESS  of DUPLICATEKEY  */
}



void freenodeTitulo(long t)
{
    nodeTitulo nod;
    
    readnodeTitulo(t, &nod);
    nod.ptr[0] = freelistTitulo;
    freelistTitulo = t;
    writenodeTitulo(t, &nod);
}



void rdstartTitulo(void)
{
    
    if (fseek(fptreeTitulo, 0L, SEEK_SET))
        error("fseek in rdstartTitulo 1");
    if (fread(startTitulo, sizeof(long), 2, fptreeTitulo) == 0)
        error("fread in rdstartTitulo 2");
    readnodeTitulo(startTitulo[0], &rootnodeTitulo);
    rootTitulo = startTitulo[0];
    freelistTitulo = startTitulo[1];
}

void wrstartTitulo(void)
{
    
    startTitulo[0] = rootTitulo;
    startTitulo[1] = freelistTitulo;
    if (fseek(fptreeTitulo, 0L, SEEK_SET))
        error("fseek in wrstartTitulo");
    if (fwrite(startTitulo, sizeof(long), 2, fptreeTitulo) == 0)
        error("fwrite in wrstartTitulo");
    if (rootTitulo != NIL)
        writenodeTitulo(rootTitulo, &rootnodeTitulo);
}

void printtreeTitulo(long t)
{
    static int position=0;
    int i, n;
    campoTitulo *k;
    nodeTitulo nod;
    
    if (t != NIL){
        position += 6;
        readnodeTitulo(t, &nod);
        k = nod.key;
        n = nod.cnt;
        printf("%*s", position, "");
        for (i=0; i<n; i++)
            printf(" %s:%d", k[i].valor, k[i].hash);
        puts("");
        for (i=0; i<=n; i++)
            printtreeTitulo(nod.ptr[i]);
        position -= 6;
    }
}























// Função para calcular a chave do hash no qual o artigo será inserido.
int calculaChave (int id){
    return (id % (NUMERO_BUCKETS));
}



// Função para inicializar o arquivo de hash vazio.
void inicializarHash(){
    int i;
    
    void *bloco = NULL;
    bloco = malloc(TAMANHO_BLOCO);
    memset(bloco, 0, TAMANHO_BLOCO);
    
    arquivoHash = fopen("hash_file.txt", "r+b");
    
    if (arquivoHash == NULL) {
        arquivoHash = fopen("hash_file.txt", "w+b");
        
        for (i = 0; i < NUMERO_BUCKETS*BLOCOS_POR_BUCKET; i++) {
            fwrite(bloco, 1, TAMANHO_BLOCO, arquivoHash);
        }
    }
    
    //fseek(arquivoHash, 0L, SEEK_END);
    //printf("%ld", ftell(arquivoHash));
    
    fclose(arquivoHash);
}


// Função para inserção dos artigos no arquivo de hash
void inserirArtigo(int chave, tArtigo *artigo, int aux){
    
    int i, j, achouEspacoLivre = 0;
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    // Ponteiro para leitura do bloco.
    void *bloco = NULL;
    bloco = malloc(TAMANHO_BLOCO);
    memset(bloco, 0, TAMANHO_BLOCO);
    
    // Vetor de artigos para armazenar os registros lidos em 1 bloco.
    tArtigo *artigos;
    artigos = (tArtigo*) malloc(sizeof(tArtigo)*REGISTROS_POR_BLOCO);
    
    // Posiciona o arquivo no bucket correspondente ao valor da chave.
    fseek(arquivoHash, chave * BLOCOS_POR_BUCKET * TAMANHO_BLOCO, SEEK_SET);
    
    // Loop para caminhar nos blocos de um bucket.
    for (i = 0; i < BLOCOS_POR_BUCKET; i++) {
        
        // Lê o primeiro bloco do bucket.
        fread(bloco, 1, TAMANHO_BLOCO, arquivoHash);
        
        // Copia os registros do bloco lido para um vetor de artigos.
        memcpy(artigos, bloco, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
        
        // Loop para caminhar nos registros de um bloco.
        for (j = 0; j < REGISTROS_POR_BLOCO; j++) {
            if (artigos[j].id == 0) {
                
                achouEspacoLivre = 1;
                
                artigos[j] = *artigo;
                memcpy(bloco, artigos, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
                
                fseek(arquivoHash, -TAMANHO_BLOCO, SEEK_CUR);
                fwrite(bloco, 1, TAMANHO_BLOCO, arquivoHash);
                
                chaveGlobal = chave;
                
                break;
            }
        }
        
        // Se não achou nenhum espaço livre no bloco j, pula para o próximo bloco do bucket.
        if (achouEspacoLivre == 0) {
            fseek(arquivoHash, TAMANHO_BLOCO, SEEK_CUR);
        }
        else{
            break;
        }
    }
    
    fclose(arquivoHash);
    
    // Se não achou nenhum espaço livre no bucket i, pula para o próximo bucket.
    if (achouEspacoLivre == 0) {
        if (chave == aux-1) {
            printf("Disco cheio, nao foi possivel inserir artigo!");
        }
        else{
            inserirArtigo((chave+1)%NUMERO_BUCKETS, artigo, aux);
        }
    }
}

// Função para buscar um artigo no arquivo de hash.
int buscarArtigo(int id, int chave, int blocosLidos){
    
    int i, j, achouArtigo = 0;
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    // Ponteiro para leitura do bloco.
    void *bloco = NULL;
    bloco = malloc(TAMANHO_BLOCO);
    memset(bloco, 0, TAMANHO_BLOCO);
    
    // Vetor de artigos para armazenar os registros lidos em 1 bloco.
    tArtigo *artigos;
    artigos = (tArtigo*) malloc(sizeof(tArtigo)*REGISTROS_POR_BLOCO);
    
    // Posiciona o arquivo no bucket correspondente ao valor da chave.
    fseek(arquivoHash, chave * BLOCOS_POR_BUCKET * TAMANHO_BLOCO, SEEK_SET);
    
    // Loop para caminhar nos blocos de um bucket.
    for (i = 0; i < BLOCOS_POR_BUCKET; i++) {
        
        // Lê o primeiro bloco do bucket.
        fread(bloco, 1, TAMANHO_BLOCO, arquivoHash);
        
        // Copia os registros do bloco lido para um vetor de artigos.
        memcpy(artigos, bloco, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
        
        blocosLidos++;
        
        // Loop para caminhar nos registros de um bloco.
        for (j = 0; j < REGISTROS_POR_BLOCO; j++) {
            if (artigos[j].id == id) {
                achouArtigo = 1;
                
                if (DEBUGANDO) {
                    printf("BUCKET: %d\n\n", chave);
                    
                    printf("ID: %d\n", artigos[j].id);
                    printf("Sigla: %s\n", artigos[j].sigla);
                    printf("Titulo: %s\n", artigos[j].titulo);
                    printf("Ano: %d\n", artigos[j].ano);
                    printf("Autores: %s\n", artigos[j].autores);
                    printf("Citacoes: %d\n", artigos[j].citacoes);
                    printf("Citepage: %s\n", artigos[j].citepage);
                    printf("Timestamp: %s\n\n", artigos[j].timestamp);
                    printf("Numero de Blocos Lidos: %d\n", blocosLidos);
                    printf("Numero de Total de Blocos do Arquivo: %d\n\n", NUMERO_BUCKETS*BLOCOS_POR_BUCKET);
                }
                
                
                aBuscado.id= artigos[j].id;
                strcpy(aBuscado.sigla, artigos[j].sigla);
                strcpy(aBuscado.titulo, artigos[j].titulo);
                aBuscado.ano = artigos[j].ano;
                strcpy(aBuscado.autores, artigos[j].autores);
                aBuscado.citacoes = artigos[j].citacoes;
                strcpy(aBuscado.citepage, artigos[j].citepage);
                strcpy(aBuscado.timestamp, artigos[j].timestamp);
                
                return 1;
            }
        }
        
        // Se não achou o artigo no bloco j, pula para o próximo bloco do bucket.
        if (achouArtigo == 0) {
            fseek(arquivoHash, TAMANHO_BLOCO, SEEK_CUR);
        }
        else{
            break;
        }
    }
    fclose(arquivoHash);
    
    // Se não achou o artigo no bucket i, pula para o próximo bucket.
    if (achouArtigo == 0) {
        if (blocosLidos >= (NUMERO_BUCKETS*BLOCOS_POR_BUCKET)) {
            //printf("Artigo nao encontrado\n");
            return 0;
        }
        else {
            return buscarArtigo(id, ((chave+1)%NUMERO_BUCKETS), blocosLidos);
        }
    }
    return 0;
}


// Função para buscar um artigo no arquivo de hash.
int buscarArtigoPorTitulo(char titulo[301], int chave, int blocosLidos){
    
    int i, j, achouArtigo = 0;
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    // Ponteiro para leitura do bloco.
    void *bloco = NULL;
    bloco = malloc(TAMANHO_BLOCO);
    memset(bloco, 0, TAMANHO_BLOCO);
    
    // Vetor de artigos para armazenar os registros lidos em 1 bloco.
    tArtigo *artigos;
    artigos = (tArtigo*) malloc(sizeof(tArtigo)*REGISTROS_POR_BLOCO);
    
    // Posiciona o arquivo no bucket correspondente ao valor da chave.
    fseek(arquivoHash, chave * BLOCOS_POR_BUCKET * TAMANHO_BLOCO, SEEK_SET);
    
    // Loop para caminhar nos blocos de um bucket.
    for (i = 0; i < BLOCOS_POR_BUCKET; i++) {
        
        // Lê o primeiro bloco do bucket.
        fread(bloco, 1, TAMANHO_BLOCO, arquivoHash);
        
        // Copia os registros do bloco lido para um vetor de artigos.
        memcpy(artigos, bloco, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
        
        blocosLidos++;
        
        // Loop para caminhar nos registros de um bloco.
        for (j = 0; j < REGISTROS_POR_BLOCO; j++) {
            if (!strcmp(artigos[j].titulo, titulo)) {
                achouArtigo = 1;
                
                if (DEBUGANDO) {
                    printf("BUCKET: %d\n\n", chave);
                    
                    printf("ID: %d\n", artigos[j].id);
                    printf("Sigla: %s\n", artigos[j].sigla);
                    printf("Titulo: %s\n", artigos[j].titulo);
                    printf("Ano: %d\n", artigos[j].ano);
                    printf("Autores: %s\n", artigos[j].autores);
                    printf("Citacoes: %d\n", artigos[j].citacoes);
                    printf("Citepage: %s\n", artigos[j].citepage);
                    printf("Timestamp: %s\n\n", artigos[j].timestamp);
                    printf("Numero de Blocos Lidos: %d\n", blocosLidos);
                    printf("Numero de Total de Blocos do Arquivo: %d\n\n", NUMERO_BUCKETS*BLOCOS_POR_BUCKET);
                }
                
                
                aBuscado.id= artigos[j].id;
                strcpy(aBuscado.sigla, artigos[j].sigla);
                strcpy(aBuscado.titulo, artigos[j].titulo);
                aBuscado.ano = artigos[j].ano;
                strcpy(aBuscado.autores, artigos[j].autores);
                aBuscado.citacoes = artigos[j].citacoes;
                strcpy(aBuscado.citepage, artigos[j].citepage);
                strcpy(aBuscado.timestamp, artigos[j].timestamp);
                
                return 1;
            }
        }
        
        // Se não achou o artigo no bloco j, pula para o próximo bloco do bucket.
        if (achouArtigo == 0) {
            fseek(arquivoHash, TAMANHO_BLOCO, SEEK_CUR);
        }
        else{
            break;
        }
    }
    fclose(arquivoHash);
    
    // Se não achou o artigo no bucket i, pula para o próximo bucket.
    if (achouArtigo == 0) {
        if (blocosLidos >= (NUMERO_BUCKETS*BLOCOS_POR_BUCKET)) {
            //printf("Artigo nao encontrado\n");
            return 0;
        }
        else {
            return buscarArtigoPorTitulo(titulo, ((chave+1)%NUMERO_BUCKETS), blocosLidos);
        }
    }
    return 0;
}



// Função que lê o arquivo de entrada e manda inserir os artigos no arquivo hash.
void upload(char * path){
    
    printf("UPLOADING...\n");
    printf("\n");
    
    
    tArtigo *artigo;
    
    char str[2001];
    char *id;
    char *sigla;
    char *titulo;
    char *ano;
    char *autores;
    char *citacoes;
    char *citepage;
    char *timestamp;
    int chave;
    
    
    FILE *arquivoEntrada = fopen(path, "r");
    
    if (arquivoEntrada == NULL) {
        printf("ERRO AO ABRIR O ARQUIVO DE ENTRADA. VERIFIQUE O NOME DO ARQUIVO E/OU CAMINHO.");
        printf("\n");
        return;
    }
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    if (arquivoHash == NULL) {
        printf("ERRO AO ABRIR ARQUIVO HASH.");
        printf("\n");
        exit(0);
    }
    
    while (!feof(arquivoEntrada)) {
        
        // Aloca memória para a leitura de um novo artigo.
        artigo = (tArtigo*) malloc(sizeof(tArtigo));
        
        // Lê uma linha do arquivo.
        fgets(str, 2000, arquivoEntrada);
        
        // Quebra a string em tokens.
        id = strtok(str, "\",\"");
        artigo->id = atoi(id);
        if (DEBUGANDO) printf("ID: %d\n", artigo->id);
        
        sigla = strtok(NULL, "\",\"");
        strcpy(artigo->sigla, sigla);
        if (DEBUGANDO) printf("Sigla: %s\n", artigo->sigla);
        
        titulo = strtok(NULL, "\",\"");
        strcpy(artigo->titulo, titulo);
        if (DEBUGANDO) printf("Titulo: %s\n", artigo->titulo);
        
        ano = strtok(NULL, "\",\"");
        artigo->ano = atoi(ano);
        if (DEBUGANDO) printf("Ano: %d\n", artigo->ano);
        
        autores = strtok(NULL, "\",\"");
        strcpy(artigo->autores, autores);
        if (DEBUGANDO) printf("Autores: %s\n", artigo->autores);
        
        citacoes = strtok(NULL, "\",\"");
        artigo->citacoes = atoi(citacoes);
        if (DEBUGANDO) printf("Citacoes: %d\n", artigo->citacoes);
        
        citepage = strtok(NULL, "\",\"");
        strcpy(artigo->citepage, citepage);
        if (DEBUGANDO) printf("Citepage: %s\n", artigo->citepage);
        
        timestamp = strtok(NULL, "\",\"");
        strcpy(artigo->timestamp, timestamp);
        if (DEBUGANDO) printf("Timestamp: %s\n\n", artigo->timestamp);
        
        chave = calculaChave(artigo->id);
        if (DEBUGANDO) printf("CHAVE: %d\n\n", chave);
        
        
        //int hash =
        inserirArtigo(chave, artigo, chave);
        
        printf("Chave: %d\n", artigo->id);
        
        openID();
        if (insert(artigo->id, chaveGlobal) == SUCCESS) {
            //printtree(root);
        }else {
            printf("erro inserir");
        }
        closeID();
        
        openTitulo();
        if (insertTitulo(artigo->titulo, chaveGlobal) == SUCCESS) {
            //printtreeTitulo(rootTitulo);
        }else {
            printf("erro inserir");
        }
        closeTitulo();
    }
    
    setStatus(1);
    
    fclose(arquivoHash);
    fclose(arquivoEntrada);
    
}