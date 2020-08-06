/******************************************************************************/
/* Theo Soares de Camargo - 206191 - LAB.4                                    */
/* EA876 - 1S2020 - 06 08 2020                                                */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <sys/types.h> /* define pid_t */
#include <sys/wait.h>  /* waitpid */
#include <unistd.h> /* fork() */
#include <sys/mman.h> /* mmap */

/*============================================================================*/
/*= Definicoes                                                               =*/
/*============================================================================*/
#define MAX 100			/* Quantidade maxima de numeros na entrada  */

typedef enum {			/* Define um tipo para verdadeiro e falso */
    TRUE,
    FALSE
} bool;

/*============================================================================*/
/*= Variaveis globais                                                        =*/
/*============================================================================*/

unsigned long int *numeros;	/* Vetor de numeros a serem  analisados */
int *contador;	                /* Conta quantos numeros sao primos */

/*============================================================================*/
/*= Funcoes                                                                  =*/
/*============================================================================*/

int ler_entrada(void) {
/*============================================================================*/
/*= Leitura da entrada padrao                                                =*/
/*= * Usa a variavel global 'numeros' como um ponteiro para um vetor,        =*/
/*= compartilhado na memoria, que contem os dados da entrada.                =*/
/*= * Retorna o numero de elementos do vetor.                                =*/
/*= Obs:                                                                     =*/
/*=     O numero maximo de entradas eh especificado por MAX (ver definicoes) =*/
/*============================================================================*/

    /* Leitura da entrada padrao -------------------------------------------- */

    unsigned long int *dados;       	/* Aloca memoria para o vetor de dados*/
    dados = (unsigned long int*) malloc (sizeof(unsigned long int)*MAX);

    int i = 0;
    char aux;

    do {			/* Gera um vetor com os numeros da entrada */
	scanf("%ld%c", &dados[i], &aux);
	i++;
    }while(aux != '\n');

    /* Criacao da memoria compartilhada ------------------------------------ */

    /* Definicao das flags de protecao e visibilidade de memoria */
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANON;

    /* Criacao da area de memoria compartilhada */
    /* Note que 'i' eh o numero de elemetos do vetor */

    numeros = (unsigned long int*) mmap(NULL, sizeof(unsigned long int)*i,
				   protection, visibility, 0, 0);

    if ((long int)numeros==-1) { /* Verifica se ocorreu erro */
	printf("Erro de alocacao!\n");
	free(dados);		/* Libera o heap */
	return(-1);

    } else {
	/* Copia os dados para a memoria compartilhada */
	for (int j = 0; j < i; j++){
	    numeros[j] = dados[j];
	}

	free(dados);		/* Libera o heap */

	return(i);		/* Retorna o numero de elementos do vetor */
    }
}

bool primo(unsigned long int n) {
/*============================================================================*/
/*= Teste de primalidade com otimizacao  6k+-1                               =*/
/*= Disponivel em: <https://en.wikipedia.org/wiki/Primality_test>            =*/
/*= Obs: n eh um numero inteiro                                              =*/
/*============================================================================*/

    if ( n <= 1) {		/* Caso n menor ou igual a 1 */
	return(FALSE);

    } else if ( n <= 3) {	/* Caso 1 < n <= 3 */
	return(TRUE);

    }else if ( (n % 2 == 0) || (n % 3 == 0) ) { /* Caso n seja divisivel por */
	return(FALSE);				/* 2 ou por 3 */

    } else {
	unsigned long int i = 5;

	while (i*i <= n) {	/* Otimização 6k+1 */
	    if ((n % i == 0) || (n % (i+2) == 0)) {
		return(FALSE);
	    }

	    i += 6;
	}

	return(TRUE);
    }
}

int init_contador(void){
/*============================================================================*/
/*=  Inicializa a variavel contadora compartilhada na memoria                =*/
/*============================================================================*/

    /* Definicao das flags de protecao e visibilidade de memoria */
    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANON;

    /* Criacao da area de memoria compartilhada */

    contador = (int*) mmap(NULL, sizeof(int), protection, visibility, 0, 0);

    if ((int)*contador==-1) {	/* Verifica se ha erro */
	 printf("Erro de alocacao!\n");
	return(-1);

    } else {
	*contador = 0;		/* Inicializa a variavel */
	return(0);
    }
}

void filho(int i, int elementos) {
/*============================================================================*/
/*= Processo filho                                                           =*/
/*=                                                                          =*/
/*= Cada filho eh especificado pela variavel 'i', a qual pode assumir o      =*/
/*= valor de 0 a 3. A variavel 'elementos' especifica quantos numeros ha no  =*/
/*= vetor global 'numeros'.                                                  =*/
/*=                                                                          =*/
/*= Essas duas informacoes sao necessarias para cada processo filho acesse   =*/
/*= uma posicao diferente no vetor. O filho i acessa as posicoes p = 4j+i,   =*/
/*= onde 'j' eh um numero inteiro que varia de forma que p < elementos.      =*/
/*============================================================================*/
    int j = 0;
    int p;

    while ((p = 4*j+i) < elementos) {

	if(primo(numeros[p]) == TRUE) {
	    *contador = *contador + 1;
	}

	j++;
    }
}

/******************************************************************************/
/* Programa Principal                                                         */
/******************************************************************************/

int main() {

    int status_cont;
    status_cont =init_contador(); /* Inicializa o contador */

    int elementos;
    elementos = ler_entrada();	/* Le a entrada, 'elementos' contem a
				   quantidade de numeros da entrada */

    /* Verifica se ha erro na alocacao de memoria --------------------------- */
    if ((elementos == -1) || status_cont == -1) {

	return(1);

    } else {			/* Senao segue o programa */


    /* Verifica quantos processos serao necessarios ------------------------- */
	int n;			/* Armazena o numero de processos */
	if (elementos <= 3){
	    n = elementos;
	} else {
	    n = 4;
	}

    /* Cria os processos ---------------------------------------------------- */
    pid_t pid[n];

    for(int i = 0; i < n; i++) {
	pid[i] = fork();

	if (pid[i] == 0) {	/* Processo filho */
	    filho(i, elementos);
	    exit(0);
	} else {

	    ;			/* Processo pai */
	}
    }

    /* Espera os processos terminarem --------------------------------------- */
    for (int j=0; j<n; j++){
	waitpid(pid[j], NULL, 0);
    }

    /* Imprime a quantidade de numeros primos ------------------------------- */
    printf("%d\n", *contador);
    return 0;
    }
}
