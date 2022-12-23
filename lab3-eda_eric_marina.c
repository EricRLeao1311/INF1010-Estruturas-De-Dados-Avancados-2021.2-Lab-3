/*INF1010 - ESTRUTURAS DE DADOS AVANCADAS - 2022.1 - 3WB
Laboratório 03 - Compactador e descompactador de arquivos 
Nome: Eric Leao     Matrícula: 2110694
Nome: Marina Schuler Martins    Matrícula: 2110075*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/*strutura da lista de frequência para árvore de huffman*/
typedef struct _frequencia {
	unsigned char ch;
	int freq;
	struct _frequencia *prox;
	struct _frequencia *esq;
	struct _frequencia *dir;
}node_freq;

/*prototipos das funções utilizadas*/
node_freq* ordena_lst(node_freq* lst);
node_freq* insere_ordenado(node_freq *lst, unsigned char c, int f);
node_freq* insere_char(node_freq* lst, unsigned char c);
node_freq* cria_lst_frequencia(FILE *arq, node_freq *lst, int *tamanho);
node_freq* insere_new_huff(node_freq *lst, node_freq *novo);
node_freq* gera_tree_huffman(node_freq* lst);
int altura_arvore(node_freq *root);
int tam_lst(node_freq *lst);
void copia(unsigned char* string, unsigned char* stringcopia);
void concatena(unsigned char* string, unsigned char caractere);
void gerastring(FILE *arq, unsigned char * s);
void preenche_hash(unsigned char ** hash, node_freq * root, unsigned char* caminho, int colunas);
void codifica(unsigned char ** hash, unsigned char * string, int * tamanho, node_freq *lst);
unsigned char* lebinario(unsigned char *string, int* tamanho, node_freq* lst, char *compactado);
void decodifica(node_freq  * root, unsigned char * string, int tamanho);
void libera_lista(node_freq * no);
void libera_arvore(node_freq *no);

int main(void) {
  FILE *arq;
	node_freq *lst, *aux;
	lst = NULL;
	aux = NULL;
  unsigned char *s;
	char compactar[20];
  int tamanho, flag = 0;
	
	printf("Menu:\nPara compactar um arquivo texto digite 0;\nPara descompactar um arquivo binário digite 1;\n");
	scanf("%d", &flag);
		if (flag) {  /*descompactando um arquivo binário*/
			unsigned char* codificado;
			aux = NULL;
			int tam;
			char compactado[20];
			printf("Digite o nome do arquivo que deseja descompactar: ");
			scanf("%s", compactado);
			if ((arq = fopen(compactado, "rb")) == NULL) {
				fprintf(stderr, "erro ao abrir o arquivo para leitura.\n");
				exit(1);
			}
			fread(&tam, 4, 1, arq);
			int lixo;
			while (tam--) {
				fread(&lixo, 1, 1, arq);
				fread(&lixo, 4, 1, arq);
			}
			fread(&tam, 4, 1, arq);
			fclose(arq);
			if ((codificado = (unsigned char*) malloc(sizeof(unsigned char*)*tam))==NULL){
				fprintf(stderr, "erro ao alocar memoria.\n");
				exit(1);
			}
			codificado[0] = '\0';
			codificado = lebinario(codificado, &tam, aux, compactado);
			if ((arq = fopen(compactado, "rb")) == NULL) {
				fprintf(stderr, "erro ao abrir o arquivo para leitura.\n");
				exit(1);
			}
			fread(&tam, 4, 1, arq);
			unsigned char c;
			while (tam--) {
				fread(&c, 1, 1, arq);
				fread(&lixo, 4, 1, arq);
				aux = insere_ordenado(aux, c, lixo);
			}
			fread(&tam, 4, 1, arq);
			fclose(arq);
			aux = gera_tree_huffman(aux);
			decodifica(aux, codificado, tam);
      libera_arvore(aux);
      free(codificado);
		}
		else {  /*compactando um arquivo texto*/
			printf("Digite o nome do arquivo que deseja compactar: ");
			scanf("%s", compactar);
			if ((arq = fopen(compactar, "r")) == NULL) {
				fprintf(stderr, "erro ao abrir o arquivo para leitura.\n");
				exit(1);
			}
			lst = cria_lst_frequencia(arq, lst, &tamanho);
			fclose(arq);
			if ((arq = fopen(compactar, "r")) == NULL) {
				fprintf(stderr, "erro ao abrir o arquivo para leitura.\n");
				exit(1);
			}
			aux = cria_lst_frequencia(arq, aux, &tamanho);
			fclose(arq);
			if ((arq = fopen(compactar, "r")) == NULL) {
				fprintf(stderr, "erro ao abrir o arquivo para leitura.\n");
				exit(1);
			}
			if ((s = (unsigned char *)malloc(sizeof(unsigned char)*(tamanho + 1))) == NULL) {
				fprintf(stderr, "Erro ao alocar memória para texto.\n");
				exit(1);
			}
			s[0] = '\0';
			gerastring(arq, s);
			lst = gera_tree_huffman(lst);
			fclose(arq);
			int colunas = altura_arvore(lst);
			unsigned char ** hash;
      if ((hash = (unsigned char**) malloc(sizeof(unsigned char**) * 256))==NULL) {
		fprintf(stderr, "erro alocar memoria.\n");
		exit(1);
	}
	for (int c = 0; c <= 255; c++)
		if ((hash[c] = (unsigned char*) calloc(colunas, sizeof(unsigned char*)))==NULL) {
			fprintf(stderr, "erro alocar memoria.\n");
			exit(1);
		}
			unsigned char * caminho;
			caminho = "\0";
      preenche_hash(hash, lst, caminho, colunas);
			codifica(hash, s, &tamanho, aux);
      free(s);
      libera_arvore(lst);
      libera_lista(aux);
      for (int w = 0; w<=255;w++)
        free(hash[w]);
      free(hash);
		}
	return 0;
}

/*funções utilizadas*/
/*funções para criação da lista de prioridade*/
node_freq* ordena_lst(node_freq* lst) {
	node_freq* novo = NULL;
	while (lst != NULL) {
		novo = insere_ordenado(novo, lst->ch, lst->freq);
		lst = lst->prox;
	}
	return novo;
}

node_freq* insere_ordenado(node_freq *lst, unsigned char c, int f) {
	node_freq* novo;
	node_freq* a = NULL;
	node_freq* p = lst;

	while (p != NULL && p->freq <= f) {
		a = p;
		p = p->prox;
	}
	if ((novo = (node_freq*)malloc(sizeof(node_freq))) == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
  //printf("entrei\n");
	novo->ch = c;
	novo->freq = f;
	novo->esq = NULL;
	novo->dir = NULL;

	if (a == NULL) {
		novo->prox = lst;
		lst = novo;
	}
	else {
		novo->prox = a->prox;
		a->prox = novo;
	}return lst;
}

node_freq* insere_char(node_freq* lst, unsigned char c) {
	node_freq* novo, *p = lst;
	while (p != NULL) {
		if (p->ch == c) {
			p->freq += 1;
			return lst;
		} p = p->prox;
	}
	if ((novo = (node_freq*)malloc(sizeof(node_freq))) == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
	novo->ch = c;
	novo->freq = 1;
	novo->prox = lst;
	novo->esq = NULL;
	novo->dir = NULL;

	return novo;
}

node_freq* cria_lst_frequencia(FILE *arq, node_freq *lst, int*tamanho) {
	unsigned char c;
	int i = 0;
  node_freq *novo;
	while (fscanf(arq, "%c", &c) == 1) {
		lst = insere_char(lst, c);
		i++;
	}novo = ordena_lst(lst);
	*tamanho = i;
  libera_lista(lst);
  //printf("entrei\n");
	return novo;
}

/*funções para criação da árvore de codificação dos caracteres (árvore de huffman)*/
node_freq* insere_new_huff(node_freq *lst, node_freq *novo) {
	node_freq *a = NULL, *p = lst;
	while (p != NULL && p->freq <= novo->freq) {
		a = p;
		p = p->prox;
	}if (a == NULL) {
		novo->prox = lst;
		lst = novo;
	}
	else {
		novo->prox = a->prox;
		a->prox = novo;
	}return lst;
}

node_freq* gera_tree_huffman(node_freq* lst) {
	node_freq* novo, *node1 = lst, *node2 = lst->prox;

	if (node1 == NULL) return NULL;
	if (node2 == NULL) {
		return lst;
	}
	lst = (lst->prox)->prox;
	if ((novo = (node_freq*)malloc(sizeof(node_freq))) == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
	novo->ch = '+';
	novo->freq = node1->freq + node2->freq;
	novo->esq = node1;
	novo->dir = node2;
	lst = insere_new_huff(lst, novo);
	return gera_tree_huffman(lst);
}

/*funções auxiliares para codificação e decodificação de texto*/
int altura_arvore(node_freq* root) {
	if (root == NULL)
		return 0;
	int esq, dir;
	esq = altura_arvore(root->esq) + 1;
	dir = altura_arvore(root->dir) + 1;
	return MAX(esq, dir);
}

int tam_lst(node_freq *lst) {
	node_freq *p = lst;
	int tam = 0;
	while (p != NULL) {
		tam++;
		p = p->prox;
	}
	return tam;
}

void copia(unsigned char* stringcopia, unsigned char* string) {
	int c = 0;
	while (string[c] != '\0') {
		stringcopia[c] = string[c];
		c++;
	}
	stringcopia[c] = '\0';
}

void concatena(unsigned char* string, unsigned char caractere) {
	int c = 0;
	while (string[c] != '\0') {
		c++;
	}
	string[c] = caractere;
	string[c + 1] = '\0';
}

void gerastring(FILE *arq, unsigned char * s) {
	unsigned char c;
	while (fscanf(arq, "%c", &c) == 1) {
		concatena(s, c);
	}
}

/*funções para codificação e decodificação de texto*/

void preenche_hash(unsigned char ** hash, node_freq * root, unsigned char* caminho, int colunas) {
	if (root->esq == NULL) {
		copia(hash[root->ch], caminho);
		return;
	}
	unsigned char *left, *right;
  if ((left = (unsigned char*) malloc(sizeof(unsigned char*) * colunas))==NULL) {
		fprintf(stderr, "erro alocar memoria.\n");
		exit(1);
	}
  if ((right = (unsigned char*) malloc(sizeof(unsigned char*) * colunas))==NULL) {
		fprintf(stderr, "erro alocar memoria.\n");
		exit(1);
	}
	copia(left, caminho);
  copia(right, caminho);
	concatena(left, '0');
	concatena(right, '1');
	preenche_hash(hash, root->esq, left, colunas);
  free(left);
	preenche_hash(hash, root->dir, right, colunas);
  free(right);
}

void codifica(unsigned char ** hash, unsigned char * string, int * tamanho, node_freq* lst) {
	FILE *binario;
	unsigned char* binario_compactado;
	char compactado[20];
	printf("Digite o nome do arquivo compactado: ");
	scanf("%s", compactado);
	binario = fopen(compactado, "wb");
	if (binario == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
	int tam_lista = tam_lst(lst);
	fwrite(&tam_lista, 4, 1, binario);
	while (lst != NULL) {
		fwrite(&(lst->ch), 1, 1, binario);
		fwrite(&(lst->freq), 4, 1, binario);
		lst = lst->prox;
	}
	int c, k, tam = 0;
	binario_compactado = 0;
	fwrite(tamanho, 4, 1, binario);
	for (c = 0; string[c] != '\0'; c++) {
		for (k = 0; hash[string[c]][k] != '\0'; k++) {
			if (tam != 0 && tam % 8 == 0) {
				fwrite(&binario_compactado, 1, 1, binario);
				binario_compactado = 0;
			}
			if (hash[string[c]][k] == '1')
				binario_compactado += (128 >> (tam % 8));
			tam++;
		}
	}
	if (binario_compactado != 0) {
		fwrite(&binario_compactado, 1, 1, binario);
		binario_compactado = 0;
	}
	fclose(binario);
}

unsigned char* lebinario(unsigned char *string, int * tamanho, node_freq* lst, char *compactado) {
	FILE *binario;
	unsigned char binario_compactado;
	unsigned char t = 128;
	binario = fopen(compactado, "rb");
	if (binario == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
	unsigned int tam = 0, f;
	unsigned char c;
	fread(&tam, 4, 1, binario);
	while (tam != 0) {
		fread(&c, 1, 1, binario);
		fread(&f, 4, 1, binario);
		//lst = insere_ordenado(lst, c, f);
		tam--;
	}
	fread(tamanho, 4, 1, binario);
	while (fread(&binario_compactado, 1, 1, binario)) {
		for (int k = 0; k < 8; k++) {
			if (binario_compactado & (t >> k)) {
				concatena(string, '1');
			}
			else concatena(string, '0');
		}
	}
	fclose(binario);
	return string;
}

void decodifica(node_freq  * root, unsigned char * string, int tamanho) {
	FILE * arq;
	char s[30];
	printf("Digite o nome do arquivo descompactado: ");
	scanf("%s", s);
	if ((arq = fopen(s, "w")) == NULL) {
		fprintf(stderr, "Erro ao alocar memória para novo elemento.\n");
		exit(1);
	}
	node_freq * temporario = root;
	int c = 0;
	while (tamanho) {
		if (temporario->esq == NULL) {
			fwrite(&(temporario->ch), 1, 1, arq);
			temporario = root;
			tamanho--;
		}
		else if (string[c] == '0') {
			temporario = temporario->esq;
			c++;
		}
		else {
			temporario = temporario->dir;
			c++;
		}
	}
	fclose(arq);
	return;
}

/*liberando memória alocada*/

void libera_arvore(node_freq *no) {
  if (no->esq == NULL && no->dir == NULL){
    free(no);
    return;
  }
	node_freq *aux1=no->esq, *aux2=no->dir;
	free(no);
	libera_arvore(aux1);
	libera_arvore(aux2);
}

void libera_lista(node_freq * no) {
	if (no == NULL) return;
	node_freq * aux = no->prox;
  free(no);
  libera_lista(aux);
}