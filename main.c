//
//  main.c
//  TP2
//
//  Created by Juscelino Tanaka on 22/01/14.
//  Copyright (c) 2014 Juscelino Tanaka. All rights reserved.
//

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



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define NUMERO_BUCKETS 150
#define BLOCOS_POR_BUCKET 10
#define TAMANHO_BLOCO 4096
#define REGISTROS_POR_BLOCO 8

FILE *arquivoHash;

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

int calculaChave (int id){
    return id % NUMERO_BUCKETS;
}

void inicializarHash(){
    int i;
    
    void *bloco = NULL;
    bloco = malloc(sizeof(TAMANHO_BLOCO));
    memset(bloco, 0, TAMANHO_BLOCO);
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    for (i = 0; i < NUMERO_BUCKETS*BLOCOS_POR_BUCKET; i++) {
        fwrite(bloco, 1, TAMANHO_BLOCO, arquivoHash);
    }
    
    fclose(arquivoHash);
}

void inserirArtigo(int chave, tArtigo artigo){
    
    int i, j, achouEspacoLivre = 0;
    
    arquivoHash = fopen("hash_file.txt", "rb+");
    
    // Ponteiro para leitura do bloco
    void *bloco = NULL;
    memset(bloco, 0, TAMANHO_BLOCO);
    
    // Vetor de artigos para armazenar os registros lidos em 1 bloco
    tArtigo *artigos;
    artigos = (tArtigo*) malloc(sizeof(tArtigo)*REGISTROS_POR_BLOCO);
    
    // Posiciona o arquivo no bucket correspondente ao valor da chave
    fseek(arquivoHash, chave * BLOCOS_POR_BUCKET * TAMANHO_BLOCO, SEEK_SET);
    
    // Le o primeiro bloco do bucket
    fread(bloco, 1, TAMANHO_BLOCO, arquivoHash);
    
    // Copia os registros do bloco lido para um vetor de artigos
    memcpy(artigos, bloco, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
    
    // Loop para caminhar nos blocos de um bucket
    for (i = 0; i < BLOCOS_POR_BUCKET; i++) {
        
        // Loop para caminhar nos registros de um bloco
        for (j = 0; i < REGISTROS_POR_BLOCO; j++) {
            if (artigos[j].id == 0) {
                
                achouEspacoLivre = 1;
                
                artigos[j] = artigo;
                memcpy(bloco, artigos, sizeof(tArtigo)*REGISTROS_POR_BLOCO);
                
                fseek(arquivoHash, -TAMANHO_BLOCO, SEEK_CUR);
                fwrite(bloco, 1, TAMANHO_BLOCO, arquivoHash);
                
                break;
            }
        }
        
        // Se nao achou nenhum espaco livre no bloco j, pula para o proximo bloco do bucket.
        if (achouEspacoLivre == 0) {
            fseek(arquivoHash, TAMANHO_BLOCO, SEEK_CUR);
        }
        else{
            break;
        }
    }
    
    // Se nao achou nenhum espaco livre no bucket i, pula para o proximo bucket.
    if (achouEspacoLivre == 0) {
        inserirArtigo(chave+1, artigo);
    }
    
    fclose(arquivoHash);
}

void buscarArtigo(int chave, tArtigo artigo){
    
}

void upload(){
    
    tArtigo artigo;
    
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
    
    FILE *arquivoEntrada = fopen("artigos2.txt", "r");
    
    if (arquivoEntrada == NULL) {
        printf("Erro");
        exit(0);
    }
    
    arquivoHash = fopen("hash.txt", "rb+");
    
    while (!feof(arquivoEntrada)) {
        fgets(str, 2000, arquivoEntrada);
        
        id = strtok(str, "\",\"");
        artigo.id = atoi(id);
        printf("ID: %d\n", artigo.id);
        
        sigla = strtok(NULL, "\",\"");
        strcpy(artigo.sigla, sigla);
        printf("Sigla: %s\n", artigo.sigla);
        
        titulo = strtok(NULL, "\",\"");
        strcpy(artigo.titulo, titulo);
        printf("Titulo: %s\n", artigo.titulo);
        
        ano = strtok(NULL, "\",\"");
        artigo.ano = atoi(ano);
        printf("Ano: %d\n", artigo.ano);
        
        autores = strtok(NULL, "\",\"");
        strcpy(artigo.autores, autores);
        printf("Autores: %s\n", artigo.autores);
        
        citacoes = strtok(NULL, "\",\"");
        artigo.citacoes = atoi(citacoes);
        printf("Citacoes: %d\n", artigo.citacoes);
        
        citepage = strtok(NULL, "\",\"");
        strcpy(artigo.citepage, citepage);
        printf("Citepage: %s\n", artigo.citepage);
        
        timestamp = strtok(NULL, "\",\"");
        strcpy(artigo.timestamp, timestamp);
        printf("Timestamp: %s\n\n", artigo.timestamp);
        
        chave = calculaChave(artigo.id);
        printf("CHAVE: %d\n\n", chave);
        
        inserirArtigo(chave, artigo);
        
    }
    
    fclose(arquivoHash);
    fclose(arquivoEntrada);
    
}


#define M 2
#define MM 4
#define NIL (-1L)
#define T 7


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

node rootnode;
long start[2], root=NIL, freelist=NIL;
FILE *fptree;

/************************FUNCTION PROTOTYPES****************************/
void error(char *str);
void readnode(long t, node *pnode);
void writenode(long t, node *pnode);
long getnode(void);
void found(long t,  int i);
void notfound(int x);
int binsearch(int x, campo *a, int n);
status search(int x);
status ins(int x, long t, int *y, long *u);
status insert(int x, int hash);
void freenode(long t);
void rdstart(void);
void wrstart(void);
status del(int x, long t);
status delnode(int x);
void printtree(long t);
/**********************END FUNCTION PROTOTYPES***************************/




int main()
{
    int x, code=0, hash;
    char ch, treefilnam[51], inpfilnam[51];
    FILE *fpinp;
    
    /* FILE *fpinp;  */
    
    puts("\nA estrutura da Arvore B sera representada por identacao.");
    printf("Nome do Arquivo Binario para a Arvore B:  ");
    scanf("%50s", treefilnam);
    fptree = fopen(treefilnam, "r+b");
    if (fptree == NULL) {
        // abre o arquivo
        fptree = fopen(treefilnam, "w+b");
        wrstart();
    }else{
        rdstart();
        printtree(root);
    }
    puts("Entre com a sequencia de inteiros seguido de / (pode ser vazia):");
    while (scanf("%d", &x) == 1) {
        hash = x % T;
        insert(x, hash);
        code = 1;
    }
    // ler a barra / de fim de linha
    (void) getchar();
    puts("");
    
    //imprime a arvore ao fim da insercao
    if (code)
        printtree(root);
    
    printf("Ha numeros a serem lidos de um arquivo de texto? (Y/N): ");
    scanf(" %c", &ch);
    while (getchar() != '\n');   /*  Rest of line skipped  */
    if (toupper(ch) == 'Y'){
        printf("Nome do arquivo de texto: ");
        scanf("%50s", inpfilnam);
        if ((fpinp = fopen(inpfilnam, "r")) == NULL)
            error("arquivo nao disponivel");
        while (fscanf(fpinp, "%d", &x) == 1) {
            hash = x % T;
            insert(x, hash);
        }
        fclose(fpinp);
        printtree(root);
    }
    
    for ( ; ; ) {
        printf("Informe um numero seguido de I, D, or P (para Inserir, \n"
               "Deletar and Procurar), ou entre Q para sair: ");
        code = scanf("%d", &x);
        scanf(" %c", &ch);
        ch = toupper(ch);
        if (code)
            
            switch (ch) {
                    
                case 'I': if ((hash = x % T) && insert(x, hash) == SUCCESS)
                    printtree(root);
                    break;
                    
                case 'D': if (delnode(x) == SUCCESS)
                    printtree(root);
                else
                    puts("Not found");
                    break;
                    
                case 'P': if (search(x) == NOTFOUND)
                    puts("Not found");
                    break;
            }
        else
            if (ch == 'Q')
                break;
    }
    
    wrstart();
    fclose(fptree);
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
        error("fseek in readnode");
    if (fread(pnode, sizeof(node), 1, fptree) == 0)
        error("fread in readnode");
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



void found(long t,  int i)
{
    node nod;
    
    printf("Found in position %d of node with contents:  ", i);
    readnode(t, &nod);
    for (i=0; i < nod.cnt; i++)
        printf("  %d", nod.key[i].valor);
    puts("");
    
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






status search(int x)
{
    int i, j, n;
    campo *k;
    node nod;
    long t = root;
    
    puts("Caminho Encontrado:");
    while (t != NIL){
        readnode(t, &nod);
        k = nod.key;
        //printf("Tnk: %d\n", k[0].valor);
        n = nod.cnt;
        for (j=0; j < n; j++)
            printf("  %d:%d", k[j].valor, k[j].hash);
        puts("");
        i = binsearch(x, k, n);
        if (i < n && x == k[i].valor){
            found(t,i);
            return(SUCCESS);
        }
        t = nod.ptr[i];
    }
    return(NOTFOUND);
}



/*
 Insert x in B-tree with root t.  If not completely successful, the
 integer *y and the pointer *u remain to be inserted.
 */

//        valor / rooot / xnew  / tnew
status ins(int x, long t, int *y, long *u)
{
    long tnew, p_final, *p;
    int i, j, *n, k_final, xnew;
    campo *k;
    status code;
    node nod, newnod;
    
    /*  Examine whether t is a pointer member in a leaf  */
    if (t == NIL){
        *u = NIL;
        *y = x;
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
    code = ins(x, p[i], &xnew, &tnew);
    if (code != INSERTNOTCOMPLETE)
        return code;
    /* Insertion in subtree did not completely succeed; try to insert xnew and
     tnew in the current node:  */
    if (*n < MM){
        i = binsearch(xnew, k, *n);
        for (j = *n; j > i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        k[i].valor = xnew;
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
        p_final = tnew;
    }else{
        k_final = k[MM-1].valor;
        p_final = p[MM];
        for (j=MM-1; j>i; j--){
            k[j] = k[j-1];
            p[j+1] = p[j];
        }
        k[i].valor = xnew;
        p[i+1] = tnew;
    }
    *y = k[M].valor;
    *n = M;
    *u = getnode(); newnod.cnt = M;
    for (j=0; j< M-1; j++){
        newnod.key[j] = k[j+M+1];
        newnod.ptr[j] = p[j+M+1];
    }
    newnod.ptr[M-1] = p[MM];
    newnod.key[M-1].valor = k_final;
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
    printf("valor: %d:%d\n", x, hash);
    
    long tnew, u;
    int xnew;
    status code = ins(x, root, &xnew, &tnew);
    
    if (code == DUPLICATEKEY)
        printf("Duplicate uid %d ignored \n", x);
    else
        if (code == INSERTNOTCOMPLETE){
            u = getnode();
            rootnode.cnt = 1;
            rootnode.key[0].valor = xnew;
            rootnode.key[0].hash = hash;
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
        error("fseek in rdstart");
    if (fread(start, sizeof(long), 2, fptree) == 0)
        error("fread in rdstart");
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





/* Delete item x in B-tree with root t.
 
 Return value:
 
 SUCCESS, NOTFOUND, OR UNDERFLOW
 */
status del(int x, long t)
{
    int i, j, *n, *nleft, *nright,  borrowleft=0, nq;
    campo *lkey, *rkey, *addr, *item, *k;
    status code;
    long *p, left, right, *lptr, *rptr, q, q1;
    node nod, nod1, nod2, nodL, nodR;
    
    if (t == NIL)
        return(NOTFOUND);
    readnode(t, &nod);
    n = & nod.cnt;
    k = nod.key;
    p=nod.ptr;
    i=binsearch(x, k, *n);
    /* *t is a leaf */
    if (p[0] == NIL){
        if (i == *n || x < k[i].valor)
            return NOTFOUND;
        /* x is now equal to k[i], located in a leaf:  */
        for (j=i+1; j < *n; j++){
            k[j-1] = k[j];
            p[j] = p[j+1];
        }
        --*n;
        writenode(t, &nod);
        return(*n >= (t==root ? 1 : M) ? SUCCESS : UNDERFLOW);
    }
    /*  t is an interior node (not a leaf): */
    item = k+i;
    left = p[i];
    readnode(left, &nod1);
    nleft = & nod1.cnt;
    /* x found in interior node.  Go to left child *p[i] and then follow a
     
     path all the way to a leaf, using rightmost branches:  */
    if (i < *n && x == item->valor){ //problema
        q = p[i];
        readnode(q, &nod1);
        nq = nod1.cnt;
        while (q1 = nod1.ptr[nq], q1!= NIL){
            q = q1;
            readnode(q, &nod1);
            nq = nod1.cnt;
        }
        /*  Exchange k[i] with the rightmost item in that leaf:   */
        addr = nod1.key + nq -1;
        *item = *addr;
        addr->valor = x; //problema
        writenode(t, &nod);
        writenode(q, &nod1);
    }
    
    /*  Delete x in subtree with root p[i]:  */
    code = del(x, left);
    if (code != UNDERFLOW)
        return code;
    /*  Underflow, borrow, and , if necessary, merge:  */
    if (i < *n)
        readnode(p[i+1], &nodR);
    if (i == *n || nodR.cnt == M){
        if (i > 0){
            readnode(p[i-1], &nodL);
            if (i == *n || nodL.cnt > M)
                borrowleft = 1;
        }
    }
    /* borrow from left sibling */
    if (borrowleft){
        item = k+i-1;
        left = p[i-1];
        right = p[i];
        nod1 = nodL;
        readnode(right, &nod2);
        nleft = & nod1.cnt;
    }else{
        right = p[i+1];        /*  borrow from right sibling   */
        readnode(left, &nod1);
        nod2 = nodR;
    }
    nright = & nod2.cnt;
    lkey = nod1.key;
    rkey = nod2.key;
    lptr = nod1.ptr;
    rptr = nod2.ptr;
    if (borrowleft){
        rptr[*nright + 1] = rptr[*nright];
        for (j=*nright; j>0; j--){
            rkey[j] = rkey[j-1];
            rptr[j] = rptr[j-1];
        }
        ++*nright;
        rkey[0] = *item;
        rptr[0] = lptr[*nleft];
        *item = lkey[*nleft - 1];
        if (--*nleft >= M){
            writenode(t, &nod);
            writenode(left, &nod1);
            writenode(right, &nod2);
            return SUCCESS;
        }
    }else
    /* borrow from right sibling */
        if (*nright > M){
            lkey[M-1] = *item;
            lptr[M] = rptr[0];
            *item = rkey[0];
            ++*nleft;
            --*nright;
            for (j=0; j < *nright; j++){
                rptr[j] = rptr[j+1];
                rkey[j] = rkey[j+1];
            }
            rptr[*nright] = rptr[*nright + 1];
            writenode(t, &nod);
            writenode(left, &nod1);
            writenode(right, &nod2);
            return(SUCCESS);
        }
    /*  Merge   */
    lkey[M-1] = *item;
    lptr[M] = rptr[0];
    for (j=0; j<M; j++){
        lkey[M+j] = rkey[j];
        lptr[M+j+1] = rptr[j+1];
    }
    *nleft = MM;
    freenode(right);
    for (j=i+1; j < *n; j++){
        k[j-1] = k[j];
        p[j] = p[j+1];
    }
    --*n;
    writenode(t, &nod);
    writenode(left, &nod1);
    return( *n >= (t==root ? 1 : M) ? SUCCESS : UNDERFLOW);
}




/*  Driver function for node deletion, called only in the main function.
 
 Most of the work is delegated to 'del'.
 */
status delnode(int x)
{
    long newroot;
    
    status code = del(x, root);
    if (code == UNDERFLOW){
        newroot = rootnode.ptr[0];
        freenode(root);
        if (newroot != NIL)
            readnode(newroot, &rootnode);
        root = newroot;
        code = SUCCESS;
    }
    return(code);  /* Return value:  SUCCESS  or NOTFOUND   */
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