/*
	* Programa de Analise Nodal Modificada no Tempo Compacta
	* Por: Ewerton Vasconcelso (ewerton.vasconcelos@poli.ufrj.br)
	*      Gabriel Silva Lopes (gabrielsl1996@poli.ufrj.br)
	*
	* Baseado no programa de demonstracao de analise nodal modificada compacta
	* do professor Antonio Carlos M. de Queiroz.
	*
	* Universidade Federal do Rio de Janeiro - UFRJ
	* Circuitos Eletricos II - 2018.2
	* Professor: Antonio Carlos M. de Queiroz (acmq@coe.ufrj.br)
	*
*/

/* Elementos aceitos:
	*
	* Resistor:             R<nome> <no+> <no-> <resistencia>
	* Indutor:              L<nome> <no+> <no-> <indutancia>
	* Capacitor:            C<nome> <no+> <no-> <capacitancia>
	* Fonte de Tensao:      V<nome> <no+> <no-> <parametros>
	* Fonte de Corrente:    I<nome> <no+> <no-> <parametros>
	* Amp. Operacional:     O<nome> <saida+> <saida-> <entrada+> <entrada->
	* Amp. de Tensao:       E<nome> <noV+> <noV-> <nov+> <nov-> <Av>
	* Amp. de Corrente:     F<nome> <noI+> <noI-> <noi+> <noi-> <Ai>
	* Transcondutor:        G<nome> <noI+> <noI-> <nov+> <nov-> <Gm>
	* Transresistor:        H<nome> <noV+> <noV-> <noi+> <noi-> <Rm>
	* AND:                  )<nome> <nóentrada> <nóentrada> <nósaída>  <V> <R> <C> <A>.
	* NAND:                 (<nome> <nóentrada> <nóentrada> <nósaída> <nóentrada> <nóentrada> <nósaída>
	* OR:                   }<nome> <nóentrada> <nóentrada> <nósaída> <nóentrada> <nóentrada> <nósaída>
	* NOR:                  {<nome> <nóentrada> <nóentrada> <nósaída> <nóentrada> <nóentrada> <nósaída>
	* Flip-Flop:            %<nome> <nóQ+> <nóQ-> <nóD> <nóCk> [<Reset>]  <V> <R> <C>
	* Monoestável:          @<nome> <nóQ+> <nóQ-> <nóTrigger> <nóReset> <V> <R> <C> <T>
	* Reset:                !<nome> <nóSet> <nóReset> <Parâmetros>
	* Comentário:           *<comentário>
	*
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS // Impede que o VS reclame de algumas funcoes
#endif

#define versao "1.0"
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LINHA 80 // Comprimento maximo de uma linha do netlist
#define MAX_NOME 11  // Comprimento maximo do nome de um elemento
#define MAX_ELEM 300 // Numero maximo de elementos no circuito
#define MAX_NOS 300  // Numero maximo de nos no circuito

#define TOLG_PADRAO 1e-15 // Tolerancia pivotal inicial
#define TOLG_MIN 1e-40	// Tolerancia pivotal minima
#define C_PO 1e9		  // Capacitor no ponto de operacao
#define L_PO 1e-9		  // Indutor no ponto de operacao
#define NR_INICIAL 0.1	// Valores iniciais para o Newton-Raphson
#define MAX_ITER 100	  // Maximo de iteracoes do Newton-Raphson
#define MAX_RAND 100	  // Maximo de randomizacoes pro Newton-Raphson
#define FAIXA_RAND 100	// Faixa de valores na randomizacao
#define TOLG_NR 1e-6	  // Tolerancia de erro no NR
#define E_MIN 1.0		  // Valor minimo entre calculo de erro absoluto ou relativo

#define THETA_MIN 1e-6	// Valor minimo pra theta
#define T_PADRAO 1e-3	 // Tempo padrao de simulacao
#define PASSO_PADRAO 1e-5 // Passo padrao
#define P_INT_PADRAO 1	// Quantidade padrao de passos internos
#define PI acos(-1)

//#define DEBUG

typedef struct elemento
{ // Elemento do netlist
	char nome[MAX_NOME];
	double valor, j_t0; // Armazenamos a corrente do tempo anterior pra C
	int a, b, c, d, x, y, x1, x2, x3, x4, x5;
	//double IC;
	char id[6]; // Pra identificar o tipo de Q e de fonte
	char resetName[10];
	int ciclos;
	double dc, ampl_1, freq, atraso, amort, phi;
	double ampl_2, subida, descida, ligada, periodo; // Valores exclusivos a PULSE


	double vOutMax, rOut, cIn, A; // para portas lógicas
	int clkMuda;				  // autoriza mudanca flipflop
	double Vo;
	//Tempo monoestavel
	double T, T0;
} elemento;

typedef int tabela[MAX_NOS + 1];

elemento netlist[MAX_ELEM];

int
	ne,						  // Elementos
	nv,						  // Variaveis
	neq, newNeq,			  // Equacoes
	nn,						  // Nos
	ponto,					  // Ponto de calculo
	qtdePontos,				  // Quantidade de pontos
	passosInt = P_INT_PADRAO, // Passos por ponto
	customTran = 0,			  // Confere se o usuario passou os parametros do tempo
	convergiu = 0,			  // Afirma se o NR convergiu ou nao
	nroIteracoes = 0,		  // Conta o numero de iteracoes
	nroRandomizacoes = 0,	 // Conta o numero de randomizacoes
	u, q, r, s,
	uci=0;

tabela C, L; // Vetores para o algoritmo de compactacao

char
	nomeArquivo[MAX_NOME + 10],
	nomeValores[MAX_NOME + 1],
	tipo,
	na[MAX_NOME], nb[MAX_NOME], nc[MAX_NOME], nd[MAX_NOME],
	lista[MAX_NOS + 1][MAX_NOME + 2],
	txt[MAX_LINHA + 1],
	*param,
	anaTipo[MAX_NOME],
	uciStr[5];

FILE *arquivo, *valores; // Arquivos com o netlist e t0

double
	Yn[MAX_NOS + 1][MAX_NOS + 2], // Matriz a ser resolvida
	tempo = T_PADRAO,			  // Tempo de simulacao
	passo = PASSO_PADRAO,		  // Tamanho do passo
	t0[MAX_NOS + 1],			  // Resultado em t0 para calcular t0 + /\t
	en[MAX_NOS + 1],			  // Resultado anterior de NR
	inicio, fim,
	tolg = TOLG_PADRAO,
	pCiclo, tempoS;

int resolverSistema(void)
{
	int i, j, l, a;
	double t, p;

	for (i = 1; i <= neq; i++)
	{
		t = 0.0;
		a = i;

		for (l = i; l <= neq; l++)
		{
			if (fabs(Yn[l][i]) > fabs(t))
			{
				a = l;
				t = Yn[l][i];
			}
		}

		if (i != a)
		{
			for (l = 1; l <= neq + 1; l++)
			{
				p = Yn[i][l];
				Yn[i][l] = Yn[a][l];
				Yn[a][l] = p;
			}
		}

		if (fabs(t) < tolg)
		{
			return 1;
		}

		for (j = neq + 1; j > 0; j--)
		{
			Yn[i][j] /= t;
			p = Yn[i][j];
			if (p != 0)
				for (l = 1; l <= neq; l++)
				{
					if (l != i)
						Yn[l][j] -= Yn[l][i] * p;
				}
		}
	}

	return 0;
}

void zerarSistema(void)
{
	for (u = 0; u <= neq; u++)
	{
		for (q = 0; q <= neq + 1; q++)
			Yn[u][q] = 0;
	}
}


int numeroNo(char *nome)
{
	int i, achou;
	i = 0;
	achou = 0;

	while (!achou && i <= nv)
		if (!(achou = !strcmp(nome, lista[i])))
			i++;

	if (!achou)
	{
		if (nv == MAX_NOS)
		{
			printf("(!) ERRO: O programa so aceita ate %d nos.\r\n", nv);
			printf("Pressione qualquer tecla para sair...");
			getchar();
			exit(1);
		}
		nv++;
		strcpy(lista[nv], nome);
		return nv; // Novo no
	}

	return i; // No ja conhecido
}

void testarNos(void)
{
	if (nv > MAX_NOS)
	{
		printf("(!) ERRO: As variaveis extra excederam o numero de variaveis permitido (%d).\r\n", MAX_NOS);
		printf("Pressione qualquer tecla para sair...");
		getchar();
		exit(1);
	}
}

void lerNetlist(void)
{
	do
	{
		ne = 0;
		nv = 0;
		strcpy(lista[0], "0");
		printf("Nome do arquivo com o netlist (ex: mna.net): ");
		scanf("%30s", nomeArquivo);
		while (getchar() != '\n')
		{ /* Limpando o buffer de possiveis \n */
		}
		arquivo = fopen(nomeArquivo, "r");
		if (arquivo == 0)
			printf("(!) ERRO: Arquivo %s inexistente.\r\n", nomeArquivo);
	} while (arquivo == 0);

	printf("Lendo netlist:\r\n");
	fgets(txt, MAX_LINHA, arquivo);
	printf("Titulo: %s", txt);

	while (fgets(txt, MAX_LINHA, arquivo))
	{

		ne++; // Nao usa o netlist[0]
		if (ne > MAX_ELEM)
		{
			printf("(!) ERRO: O programa so aceita ate %d elementos.\r\n", MAX_ELEM);
			printf("Pressione qualquer tecla para sair...");
			getchar();
			fclose(arquivo);
			exit(1);
		}

		txt[0] = toupper(txt[0]); // O primeiro caractere da linha descreve a linha
		tipo = txt[0];
		sscanf(txt, "%10s", netlist[ne].nome);
		param = txt + strlen(netlist[ne].nome);

		if (tipo == 'R')
		{
			sscanf(param, "%10s %10s %lf", na, nb, &netlist[ne].valor);
			printf("%s %s %s %f\r\n", netlist[ne].nome, na, nb, netlist[ne].valor);
			netlist[ne].a = numeroNo(na);
			netlist[ne].b = numeroNo(nb);
		}
		else if (tipo == 'L' || tipo == 'C')
		{
			int numeroDePalavras = 0;
			char tempIc[10];

			for (int i = 0; param[i] != '\n' && param[i] != '\r' && param[i] != '\0'; i++)
			{
				if (param[i] == ' ')
					numeroDePalavras++;
			}
			if (numeroDePalavras == 3){
				sscanf(param, "%10s %10s %lf", na, nb, &netlist[ne].valor);
				printf("%s %s %s %f\r\n", netlist[ne].nome, na, nb, netlist[ne].valor);
				netlist[ne].a = numeroNo(na);
				netlist[ne].b = numeroNo(nb);
				netlist[ne].j_t0 = 0;
			}
			else{
				sscanf(param, "%10s %10s %lf %10s", na, nb, &netlist[ne].valor, tempIc);
				printf("%s %s %s %f %10s\r\n", netlist[ne].nome, na, nb, netlist[ne].valor,tempIc);
				netlist[ne].a = numeroNo(na);
				netlist[ne].b = numeroNo(nb);
				
				char auxiliar[10];
				for (int i = 3; tempIc[i] != '\n' && tempIc[i] != '\r' && tempIc[i] != '\0'; i++){
					auxiliar[i-3] = tempIc[i];
				}
				strcat(auxiliar, "\0");

				sscanf (auxiliar, "%lf", &netlist[ne].j_t0);
				printf("%.3e\r\n", netlist[ne].j_t0);
			}

		}		
		else if (tipo == 'G' || tipo == 'E' || tipo == 'F' || tipo == 'H')
		{
			sscanf(param, "%10s %10s %10s %10s %lf", na, nb, nc, nd, &netlist[ne].valor);
			printf("%s %s %s %s %s %f\r\n", netlist[ne].nome, na, nb, nc, nd, netlist[ne].valor);
			netlist[ne].a = numeroNo(na);
			netlist[ne].b = numeroNo(nb);
			netlist[ne].c = numeroNo(nc);
			netlist[ne].d = numeroNo(nd);
		}
		else if (tipo == 'O')
		{
			sscanf(param, "%10s %10s %10s %10s", na, nb, nc, nd);
			printf("%s %s %s %s %s\r\n", netlist[ne].nome, na, nb, nc, nd);
			netlist[ne].a = numeroNo(na);
			netlist[ne].b = numeroNo(nb);
			netlist[ne].c = numeroNo(nc);
			netlist[ne].d = numeroNo(nd);
		}
		else if (tipo == '(' || tipo == ')' || tipo == '{' || tipo == '}')
		{ //double vOutMax, rOut, cIn, A;
			sscanf(param, "%10s %10s %10s %lf %lf %lf %lf", na, nb, nc, &netlist[ne].vOutMax, &netlist[ne].rOut,
				   &netlist[ne].cIn, &netlist[ne].A);

			printf("%s %s %s %s %.4e %.4e %.4e %.4e\r\n", netlist[ne].nome, nc, nb, na, netlist[ne].vOutMax, netlist[ne].rOut, netlist[ne].cIn, netlist[ne].A);

			netlist[ne].c = numeroNo(nc);
			netlist[ne].b = numeroNo(nb);
			netlist[ne].a = numeroNo(na);
		}

		else if (tipo == '%')
		{ //  %<nome> <nóQ+> <nóQ-> <nóD> <nóCk> [<Reset>] a b c

			int numeroDePalavras = 0;
			for (int i = 0; param[i] != '\n' && param[i] != '\r' && param[i] != '\0'; i++)
			{
	
				if (param[i] == ' ')
					numeroDePalavras++;
			}

			if (numeroDePalavras == 8)
			{
				sscanf(param, "%10s %10s %10s %10s %s %lf %lf %lf", na, nb, nc, nd, netlist[ne].resetName, &netlist[ne].vOutMax, &netlist[ne].rOut, &netlist[ne].cIn);
				printf("nome:%s na:%s nb:%s nc:%s nd:%s reset:%s v:%.4e r:%.4e c:%.4e\r\n", netlist[ne].nome, na, nb, nc, nd, netlist[ne].resetName, netlist[ne].vOutMax, netlist[ne].rOut, netlist[ne].cIn);
			}
			else if (numeroDePalavras == 7)
			{
				strcpy(netlist[ne].resetName, "");
				sscanf(param, "%10s %10s %10s %10s %lf %lf %lf", na, nb, nc, nd, &netlist[ne].vOutMax, &netlist[ne].rOut,
					   &netlist[ne].cIn);
				printf("nome:%s na:%s nb:%s nc:%s nd:%s v:%.4e r:%.4e c:%.4e\r\n", netlist[ne].nome, na, nb, nc, nd, netlist[ne].vOutMax, netlist[ne].rOut, netlist[ne].cIn);
			}
			else
			{
				printf("Verifique o numero de parametros do Flip-Flop.");
				exit(1);
			}
			netlist[ne].d = numeroNo(nd);
			netlist[ne].c = numeroNo(nc);
			netlist[ne].b = numeroNo(nb);
			netlist[ne].a = numeroNo(na);
		}
		else if (tipo == '!')
		{
			sscanf(param, "%10s %10s %lf %lf", na, nb, &netlist[ne].vOutMax, &netlist[ne].cIn); //Reset: !<nome> <nóSet> <nóReset> <Parâmetros = <V> <C> >
			printf("nome:%s na:%s nb:%s v:%.4e c:%.4e\r\n", netlist[ne].nome, na, nb, netlist[ne].vOutMax, netlist[ne].cIn);
			netlist[ne].b = numeroNo(nb);
			netlist[ne].a = numeroNo(na);
		}
		else if (tipo == '@')
		{
			sscanf(param, "%10s %10s %10s %10s %lf %lf %lf %lf", na, nb, nc, nd, &netlist[ne].vOutMax, &netlist[ne].rOut, &netlist[ne].cIn, &netlist[ne].T); 
			netlist[ne].b = numeroNo(nb);
			netlist[ne].a = numeroNo(na);
			netlist[ne].c = numeroNo(nc);
			netlist[ne].d = numeroNo(nd);
			printf("nome:%s na:%i nb:%i nc:%i nd:%i v:%.4e r:%.4e c:%.4e t:%.4e\r\n", netlist[ne].nome, netlist[ne].a, netlist[ne].b,netlist[ne].c, netlist[ne].d, netlist[ne].vOutMax, netlist[ne].rOut, netlist[ne].cIn,netlist[ne].T);

		}
		else if (tipo == 'I' || tipo == 'V')
		{
			sscanf(param, "%10s %10s %10s", na, nb, netlist[ne].id);
			param = param + strlen(na) + strlen(nb) + strlen(netlist[ne].id) + 4;
			if (!strcmp(netlist[ne].id, "DC"))
			{
				sscanf(param, "%lf", &netlist[ne].valor);
				printf("%s %s %s %s %f\r\n", netlist[ne].nome, na, nb, netlist[ne].id, netlist[ne].valor);
				netlist[ne].a = numeroNo(na);
				netlist[ne].b = numeroNo(nb);
			}
			else if (!strcmp(netlist[ne].id, "SIN"))
			{
				sscanf(param, "%lf %lf %lf %lf %lf %lf %i", &netlist[ne].dc, &netlist[ne].ampl_1, &netlist[ne].freq,
					   &netlist[ne].atraso, &netlist[ne].amort, &netlist[ne].phi, &netlist[ne].ciclos);
				printf("%s %s %s %s %f %f %f %f %f %f %i\r\n", netlist[ne].nome, na, nb, netlist[ne].id,
					   netlist[ne].dc, netlist[ne].ampl_1, netlist[ne].freq, netlist[ne].atraso, netlist[ne].amort,
					   netlist[ne].phi, netlist[ne].ciclos);
				netlist[ne].a = numeroNo(na);
				netlist[ne].b = numeroNo(nb);
			}
			else if (!strcmp(netlist[ne].id, "PULSE"))
			{
				sscanf(param, "%lf %lf %lf %lf %lf %lf %lf %i", &netlist[ne].ampl_1, &netlist[ne].ampl_2, &netlist[ne].atraso, &netlist[ne].subida, &netlist[ne].descida, &netlist[ne].ligada, &netlist[ne].periodo, &netlist[ne].ciclos);

	
				printf("%s %s %s %s %.7e %.7e %.7e %.7e %.7e %.7e %.7e %i\r\n", netlist[ne].nome, na, nb,
					   netlist[ne].id, netlist[ne].ampl_1, netlist[ne].ampl_2, netlist[ne].atraso, netlist[ne].subida,
					   netlist[ne].descida, netlist[ne].ligada, netlist[ne].periodo, netlist[ne].ciclos);

				netlist[ne].a = numeroNo(na);
				netlist[ne].b = numeroNo(nb);
			}
			else
			{
				printf("(!) ERRO: Tipo de fonte desconhecido: %s\r\n", netlist[ne].id);
				printf("Pressione qualquer tecla para sair...");
				getchar();
				fclose(arquivo);
			}
		}
		else if (tipo == '*')
		{ // Comentario comeca com "*"
			printf("Comentario: %s\r", txt);
			ne--;
		}
		else if (tipo == '.')
		{
	
			int numeroDePalavras = 0;
			for (int i = 0; param[i] != '\n' && param[i] != '\r' && param[i] != '\0'; i++)
			{
				if (param[i] == ' ')
					numeroDePalavras++;
			}
			
			if (numeroDePalavras == 4) {
				sscanf(param, "%lf %lf %s %i", &tempo, &passo, anaTipo, &passosInt);
				uci =0;
			} else {
				sscanf(param, "%lf %lf %s %i %s", &tempo, &passo, anaTipo, &passosInt, uciStr);
				uci =1;
			}
			
			
			int j = 0;
			for (int i = 0; anaTipo[i] != '\0'; i++)
			{
				//printf("entrou %i, anaTipo: %c",i, anaTipo[i]);
				if (anaTipo[i] != ' ')
					anaTipo[j++] = anaTipo[i];
			}
			anaTipo[j] = '\0';

			//printf("%i",strcmp(anaTipo,"BE"));
			if (strcmp(anaTipo, "BE"))
			{
				printf("Analise nao permitida\r\nPressione qualquer tecla para sair...");
				getchar();
				fclose(arquivo);
				exit(1);
			}

			printf("Tempo:%.7e , Passo: %.7e, Tipo: '%s' Passos internos: %i \r\n", tempo, passo, anaTipo, passosInt);
			customTran = 1;
			ne--;
		}
		else
		{
			printf("(!) ERRO: Elemento desconhecido: %s\r\n", txt);
			printf("Pressione qualquer tecla para sair...");
			getchar();
			fclose(arquivo);
			exit(1);
		}
	}
	fclose(arquivo);

	if (!customTran)
		printf("/!\\ Aviso: nao foram passados valores para a analise no tempo. Serao usados os valores padrao.\r\n");
	printf("Tempo de simulacao: %f s\r\n", tempo);
	printf("Tamanho de Passo: %f s\r\n", passo);
	printf("Passos internos: %i\r\n", passosInt);
	getchar();
}

/* Rotina de simplificacao do sistema com amp. ops. */
void somar(int *Q, int a, int b)
{
	int i, a1, b1;

	if (Q[a] > Q[b])
	{
		a1 = Q[b];
		b1 = Q[a];
	}
	else
	{
		a1 = Q[a];
		b1 = Q[b];
	}

	if (a1 == b1)
	{
		printf("(!) ERRO: Circuito invalido - Entradas ou saidas de um amp. op. em curto.\r\n");
		printf("Pressione qualquer tecla para sair...\r\n");
		getchar();
		exit(1);
	}

	for (i = 1; i <= MAX_NOS; i++)
	{
		if (Q[i] == b1)
			Q[i] = a1;
		if (Q[i] > b1)
			Q[i]--;
	}
}

/* Elementos do programa. */
void operacional(int na, int nb, int nc, int nd)
{
#ifdef DEBUG
	printf(" Saida: %d %d; entrada %d %d\r\n", na, nb, nc, nd);
#endif
	somar(L, na, nb);
	somar(C, nc, nd);
}

void transcondutancia(double gm, int n1, int n2, int n3, int n4)
{
	Yn[L[n1]][C[n3]] += gm;
	Yn[L[n2]][C[n4]] += gm;
	Yn[L[n1]][C[n4]] -= gm;
	Yn[L[n2]][C[n3]] -= gm;
}

void condutancia(double g, int a, int b)
{
	transcondutancia(g, a, b, a, b);
}

/* Rotina que corrige o tempo de subida e descida de fontes do tipo PULSE. */
void correcaoPulse(void)
{
	for (u = 1; u <= ne; u++)
	{
		if (!strcmp(netlist[ne].id, "PULSE"))
		{
			if (netlist[ne].subida < passo)
				netlist[ne].subida = passo;
			if (netlist[ne].descida < passo)
				netlist[ne].descida = passo;
		}
	}
}

/* Funcao que cacula o valor de uma fonte de tensao/corrente no tempo passo*ponto. */
double
valorFonte(elemento componente)
{
	//printf("Passo:%.7e, Ponto:%i, ciclos:%lu, periodo:%.7e \r\n", passo, ponto, netlist[u].ciclos, netlist[u].periodo);
	double val, t;
	//printf("%c \r\n",componente.id[0] );
	switch (componente.id[0])
	{
	case 'D':
		val = componente.valor;
		break;

	case 'S':
		t = componente.atraso + (componente.ciclos / componente.freq);
		if (passo * ponto <= componente.atraso) // Antes do atraso
			val = componente.dc + componente.ampl_1 * (sin(componente.phi * PI / 180));
		else if (passo * ponto >= t) // Fim dos ciclos
			val = componente.dc + componente.ampl_1 * exp(-1 * componente.amort * (t - componente.atraso)) * (sin(2 * PI * componente.freq * (t - componente.atraso) + (componente.phi * PI / 180)));
		else // Caso geral, depois do atraso e antes do fim
			val = componente.dc + componente.ampl_1 * exp(-1 * componente.amort * (passo * ponto - componente.atraso)) * (sin(2 * PI * componente.freq * (passo * ponto - componente.atraso) + (componente.phi * PI / 180)));
		break;

	case 'P':
		if (componente.subida == 0.0)
		{
			componente.subida = passo / passosInt;
		}
		if (componente.descida == 0.0)
		{
			componente.descida = passo / passosInt;
		}
		tempoS = ponto * passo;
		//caso o tempo seja maior que o atraso
		if (tempoS > componente.atraso)
		{
			//caso seja maior que o atraso e o tempo total dividido pelo periodo seja menor que o numero
			//de ciclos ento entra
			if (((tempoS - componente.atraso) / (componente.periodo)) < componente.ciclos)
			{
				//calcula o periodo do ciclo
				pCiclo = fmod((tempoS - componente.atraso), componente.periodo);
				// Caso tempo dentro do ciclo seja menor que o tempo de subida
				if (pCiclo < componente.subida)
					return (((componente.ampl_2 - componente.ampl_1) * (pCiclo / componente.subida)) + componente.ampl_1);
				//caso o tempo dentro do ciclo seja maior que o tempo de subida
				if (pCiclo <= (componente.subida + componente.ligada))
					return (componente.ampl_2);
				//caso o tempo dentro do ciclose seja maior que o tempo de decida
				if (pCiclo < (componente.subida + componente.ligada + componente.descida))
					return (((componente.ampl_1 - componente.ampl_2) * (pCiclo - componente.subida - componente.ligada) / (componente.descida)) + componente.ampl_2);
			}
		}
		//caso ele n�o atenda nenhuma das tentativas acima devolver amplitude DC
		return (componente.ampl_1);
		break;
	}

	return val;
}

void corrente(double i, int a, int b)
{
	Yn[L[a]][neq + 1] -= i;
	Yn[L[b]][neq + 1] += i;
}

void tensao(double v, int a, int b, int x)
{
	transcondutancia(1, 0, x, a, b);
	corrente(v, x, 0);
}

void ganhoTensao(double av, int a, int b, int c, int d, int x)
{
	transcondutancia(1, 0, x, a, b);
	transcondutancia(av, x, 0, c, d);
}

void ganhoCorrente(double ai, int a, int b, int c, int d, int x)
{
	transcondutancia(ai, a, b, x, 0);
	transcondutancia(1, c, d, x, 0);
}

void transresistencia(double rm, int a, int b, int c, int d, int x, int y)
{
	transcondutancia(1, 0, y, a, b);
	transcondutancia(rm, y, 0, x, 0);
	transcondutancia(1, c, d, x, 0);
}

void transformador(double n, int a, int b, int c, int d, int x)
{
	Yn[L[a]][C[x]] -= n;
	Yn[L[b]][C[x]] += n;
	Yn[L[c]][C[x]] += 1;
	Yn[L[d]][C[x]] -= 1;
	Yn[L[x]][C[a]] += n;
	Yn[L[x]][C[b]] -= n;
	Yn[L[x]][C[c]] -= 1;
	Yn[L[x]][C[d]] += 1;
}

void capacitor(double c, int a, int b, double j)
{
	//printf("entrou\r\n");
	if(!ponto && j!=0.0 && uci) {		
		corrente(c * j/ passo, b, a);
		condutancia(c / (passo), a, b);
		//printf("B\r\n");
	} else {
		corrente(c * (t0[a] - t0[b]) / passo, b, a);
		condutancia(c / (passo), a, b);
		//printf("C\r\n");
	}
    
}

void indutor(double l, int a, int b, int x, double j0)
{

	if (uci && !ponto)
	{
		Yn[L[a]][C[x]] += 1;
		Yn[L[b]][C[x]] -= 1;
		Yn[L[x]][C[a]] -= 1;
		Yn[L[x]][C[b]] += 1;
		Yn[L[x]][C[x]] += l / (passo);
		Yn[L[x]][neq + 1] += (j0 * l / (passo));
	} else {
		Yn[L[a]][C[x]] += 1;
		Yn[L[b]][C[x]] -= 1;
		Yn[L[x]][C[a]] -= 1;
		Yn[L[x]][C[b]] += 1;
		Yn[L[x]][C[x]] += l / (passo);
		Yn[L[x]][neq + 1] += (t0[x] * l / (passo));
	}

}




void portaNand(int c, int b, int a, double V, double rOut, double cIn, double A)
{
	double Vx, Vo, A1, A2;

	//printf("en[a]: %f \r\n",en[a]);
	//printf("en[b]: %f \r\n",en[b]);
	//printf("en[c]: %f \r\n",en[b]);

	if (en[a] > en[b])
	{

		Vx = V / 2 - A * (en[b] - (V / 2));
		//printf("Aqui1: Vx: %f V: %f A: %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				Vo = V / 2 * (1 + A);
				A1 = 0;
				A2 = -A;
			}
		}
	}
	else
	{

		Vx = V / 2 - A * (en[a] - (V / 2));
		//printf("Aqui2: Vx: %f V: %f A:  %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			//printf("2.1\r\n");
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				//printf("2.2\r\n");
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				//printf("2.3\r\n");
				Vo = V / 2 * (1 + A);
				A1 = -A;
				A2 = 0;
			}
		}
	}

	//printf("ea:%f eb:%f A1:%f A2:%f Vo:%f Ro: %f\r\n",en[a],en[b],A1,A2,Vo, rOut);

	//Modelo Linearizado
	//resistor

	condutancia(1 / rOut, 0, c);
	////I
	corrente(Vo / rOut, 0, c);
	//////G1
	transcondutancia(A1 / rOut, 0, c, a, 0);
	////////G2
	transcondutancia(A2 / rOut, 0, c, b, 0);
	// capacitancia de entrada
	//capacitor(cIn, 0, a, 0 );

	//capacitor(cIn, 0, b, 0 );
}

/*void
	setReset (char *resetName, int *a, int *b, double *V){
	for (int x = 1; x <= 11; x++) {
	printf("nome: %s, noa: %i, nob:%i\r\n",netlist[x].nome, netlist[x].a,netlist[x].b);
	if (!strcmp(resetName, netlist[x].nome)){
	*a = netlist[x].a;
	*b = netlist[x].b;
	*V = netlist[x].vOutMax;
	//printf("aSet: %i, bSet: %i, vSet: %.3e\r\n", *a,*b,*V);			
	}
	
	}
	}
*/



void flipflopD(char *nome, int a, int b, int c, int d, char *resetName, double V, double rOut, double cIn, int *clkMuda, double *Vo, int aSet, int bSet, double vSet)
{
	//printf("entrou - %i %.4e %.4e\r\n", *clkMuda, *Vo, en[d]);
	double Vob;

	int controleSet = 0;
	//printf("a:%i, b:%i, c:%i, d:%i, V:%.3e CLK:%i en[c]:%.3e\r\n",a, b,c ,d, V, *clkMuda, en[d]);

	//printf("resetName: '%s'\r\n", resetName);
	if (strcmp(resetName, ""))
	{
		//printf("ENTROU\r\n");
		//setReset(resetName, &aSet, &bSet, &vSet);
		controleSet = 1;
		//printf("aSet: %i, bSet: %i, vSet: %.3e\r\n", aSet, bSet, vSet);
		//printf("en == aSet: %.3e, bSet: %.3e \r\n", en[aSet], en[bSet]);
	}

	if (ponto == 0)
	{
		*Vo = 0;
		//en[bSet]=0;
		//en[aSet]=0;
	}

	if (en[d] < V / 2)
	{
		*clkMuda = 1;
	}

	if (controleSet)
	{
		//printf("Novo: en[aSet]: %.3e, en[bSet]: %.3e, en[c]:%.3e\r\n",en[aSet],en[bSet],en[c]);
		if (en[aSet] >= vSet / 2)
		{
			*Vo = V;
			//printf("SET Ponto: %i, Nome: %s set: %.3e reset: %.3e, q:%.3e, |q|:%.3e \r\n",ponto, nome, en[aSet], en[bSet], en[a],en[b]);
			*clkMuda = 0;
		}
		else if (en[bSet] >= vSet / 2)
		{
			*Vo = 0;
			//printf("RESET Ponto: %i, Nome: %s set: %.3e reset: %.3e, q:%.3e, |q|:%.3e \r\n",ponto, nome, en[aSet], en[bSet], en[a],en[b]);
			*clkMuda = 0;
		}
		else if (en[d] >= V / 2 && *clkMuda)
		{
			if (en[c] >= V / 2)
			{
				*Vo = V;
				//printf("D>V/2 Ponto: %i, Nome: %s set: %.3e reset: %.3e, q:%.3e, |q|:%.3e \r\n",ponto, nome, en[aSet], en[bSet], en[a],en[b]);*/
				*clkMuda = 0;
			}
			else
			{
				//printf("D<V/2 Ponto: %i, Nome: %s set: %.3e reset: %.3e, q:%.3e, |q|:%.3e \r\n",ponto, nome, en[aSet], en[bSet], en[a],en[b]);
				*Vo = 0;
				*clkMuda = 0;
			}
		}
	}
	else
	{
		if (ponto == 0)
		{
			*Vo = 0;
		}

		if (en[d] >= V / 2 && *clkMuda)
		{
			//printf("en[aSet]: %.3e, en[bSet]: %.3e\r\n", en[aSet], en[bSet]);
			if (en[c] >= V / 2)
				*Vo = V;
			else
				*Vo = 0;
			*clkMuda = 0;
		}
	}

	//printf("en[d]: %.3e  en[c]: %.3e  en[b]: %.3e  en[a]: %.3e\r\n\r\n", en[d], en[c], en[b], en[a]);

	if (*Vo == 0)
	{
		Vob = V;
	}
	else
	{
		Vob = 0;
	}

	//printf("Vo: %.3e\r\n", *Vo);

	//printf("%.3e %.3e \r\n",*Vo, Vob);
	condutancia(1 / rOut, a, 0);
	////I
	corrente(*Vo / rOut, 0, a);

	condutancia(1 / rOut, b, 0);
	////I
	corrente(Vob / rOut, 0, b);
}

//monoestavel(netlist[u].a, netlist[u].b, netlist[u].c, netlist[u].d, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, &netlist[u].clkMuda, netlist[u].T);
void monoestavel(int a, int b, int c, int d, double V, double rOut, double cIn, double T, int *clkMuda, double *Vo, double *T0 )
{

	double Vob, Tf;
	Tf = ponto*passo;

	//printf("ponto:%i, TF-T0:%.3e,T:%.3e, en[c]:%.3e, en[a]:%.3e, clk:%i\r\n",ponto, Tf-*T0,T, en[c],en[a], *clkMuda);
	//getchar();
	if (ponto == 0)
	{
		*Vo = 0;

	}

	if (en[c] <= V/2)
	{
		*clkMuda = 1;

	} 
	

	
	if (en[c] >= V/2 && *clkMuda)
	{
		//printf("a\r\n");
		*Vo = V;
		*clkMuda = 0;
		*T0=ponto*passo;
	} 
	
	if ((Tf-*T0) > T && *Vo==V) {
		//printf("b\r\n");
		*Vo = 0;

	} 
		if (ponto == 0)
	{
		*Vo = 0;

	}

	if (*Vo == 0)
	{
		Vob = V;
	}
	else
	{
		Vob = 0;
	}
	//printf("Vo:%.3e, r:%.3e\r\n", *Vo,rOut);
	condutancia(1 / rOut, a, 0);
	////I
	corrente(*Vo / rOut, 0, a);

	condutancia(1 / rOut, b, 0);
	////I
	corrente(Vob / rOut, 0, b);
}


void portaAnd(int c, int b, int a, double V, double rOut, double cIn, double A)
{
	double Vx, Vo, A1, A2;

	A = -A; //Modificação nand para and
	//printf("A: %f", A);
	//printf("en[a]: %f \r\n",en[a]);
	//printf("en[b]: %f \r\n",en[b]);
	//printf("en[c]: %f \r\n",en[b]);

	if (en[a] > en[b])
	{

		Vx = V / 2 - A * (en[b] - (V / 2));
		//printf("Aqui1: Vx: %f V: %f A: %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				Vo = V / 2 * (1 + A);
				A1 = 0;
				A2 = -A;
			}
		}
	}
	else
	{

		Vx = V / 2 - A * (en[a] - (V / 2));
		//printf("Aqui2: Vx: %f V: %f A:  %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			//printf("2.1\r\n");
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				//printf("2.2\r\n");
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				//printf("2.3\r\n");
				Vo = V / 2 * (1 + A);
				A1 = -A;
				A2 = 0;
			}
		}
	}

	//printf("ea:%f eb:%f A1:%f A2:%f Vo:%f Ro: %f\r\n",en[a],en[b],A1,A2,Vo, rOut);

	//Modelo Linearizado
	//resistor

	condutancia(1 / rOut, 0, c);
	////I
	corrente(Vo / rOut, 0, c);
	//////G1
	transcondutancia(A1 / rOut, 0, c, a, 0);
	////////G2
	transcondutancia(A2 / rOut, 0, c, b, 0);
	// capacitancia de entrada
	//capacitor(cIn, 0, a, 0 );

	//capacitor(cIn, 0, b, 0 );
}

void portaOr(int c, int b, int a, double V, double rOut, double cIn, double A)
{
	double Vx, Vo, A1, A2;

	A = -A; //Modificação nand para and

	//printf("en[a]: %f \r\n",en[a]);
	//printf("en[b]: %f \r\n",en[b]);
	//printf("en[c]: %f \r\n",en[b]);

	if (en[a] < en[b])
	{

		Vx = V / 2 - A * (en[b] - (V / 2));
		//printf("Aqui1: Vx: %f V: %f A: %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				Vo = V / 2 * (1 + A);
				A1 = 0;
				A2 = -A;
			}
		}
	}
	else
	{

		Vx = V / 2 - A * (en[a] - (V / 2));
		//printf("Aqui2: Vx: %f V: %f A:  %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			//printf("2.1\r\n");
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				//printf("2.2\r\n");
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				//printf("2.3\r\n");
				Vo = V / 2 * (1 + A);
				A1 = -A;
				A2 = 0;
			}
		}
	}

	//printf("ea:%f eb:%f A1:%f A2:%f Vo:%f Ro: %f\r\n",en[a],en[b],A1,A2,Vo, rOut);

	//Modelo Linearizado
	//resistor

	condutancia(1 / rOut, 0, c);
	////I
	corrente(Vo / rOut, 0, c);
	//////G1
	transcondutancia(A1 / rOut, 0, c, a, 0);
	////////G2
	transcondutancia(A2 / rOut, 0, c, b, 0);
	// capacitancia de entrada
	//capacitor(cIn, 0, a, 0 );

	//capacitor(cIn, 0, b, 0 );
}

void portaNor(int c, int b, int a, double V, double rOut, double cIn, double A)
{
	double Vx, Vo, A1, A2;

	//printf("en[a]: %f \r\n",en[a]);
	//printf("en[b]: %f \r\n",en[b]);
	//printf("en[c]: %f \r\n",en[b]);

	if (en[a] < en[b])
	{

		Vx = V / 2 - A * (en[b] - (V / 2));
		//printf("Aqui1: Vx: %f V: %f A: %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				Vo = V / 2 * (1 + A);
				A1 = 0;
				A2 = -A;
			}
		}
	}
	else
	{

		Vx = V / 2 - A * (en[a] - (V / 2));
		//printf("Aqui2: Vx: %f V: %f A:  %f\r\n",Vx, V, A);
		if (Vx > V)
		{
			//printf("2.1\r\n");
			Vo = V;
			A1 = 0;
			A2 = 0;
		}
		else
		{
			if (Vx < 0)
			{
				//printf("2.2\r\n");
				Vo = 0;
				A1 = 0;
				A2 = 0;
			}
			else
			{
				//printf("2.3\r\n");
				Vo = V / 2 * (1 + A);
				A1 = -A;
				A2 = 0;
			}
		}
	}

	//printf("ea:%f eb:%f A1:%f A2:%f Vo:%f Ro: %f\r\n",en[a],en[b],A1,A2,Vo, rOut);

	//Modelo Linearizado
	//resistor

	condutancia(1 / rOut, 0, c);
	////I
	corrente(Vo / rOut, 0, c);
	//////G1
	transcondutancia(A1 / rOut, 0, c, a, 0);
	////////G2
	transcondutancia(A2 / rOut, 0, c, b, 0);
	// capacitancia de entrada
	//capacitor(cIn, 0, a, 0 );

	//capacitor(cIn, 0, b, 0 );
}
/*
void 
flipflopD(char *nome, int a, int b, int c, int d, char *resetName, double V, double rOut, double cIn,int int1,int int2,int int3,int int4,int int5) {

//printf("%i %i %i %i %i \r\n", int1, int2, int3, int4, int5);

portaNand(int3,int1,int2,V, rOut, cIn, 20); //1
portaNand(int2,d,int3,V, rOut, cIn, 20); //2
portaNand(int4,int1,int5,V, rOut, cIn, 20); //3
portaNand(int1,int4,c,V, rOut, cIn, 20); //4
portaNand(a,int2,b,V, rOut, cIn, 20); //5
portaNand(b,int4,a,V, rOut, cIn, 20); //6
portaAnd(int5,d,int2,V, rOut, cIn, 20);

}*/

/* Essa rotina conta os elementos nao aceitos pela analise nodal simples,
* simplificando com amp. ops. ou nao, dependendo do elemento. */
void elementosModificada(void)
{
	nn = nv;
	neq = nn;

	for (u = 1; u <= ne; u++)
	{
		tipo = netlist[u].nome[0];
		if (tipo == 'V' || tipo == 'E')
		{
			nv++;
			strcpy(lista[nv], "j"); // Tem espaco para mais dois caracteres
			strcat(lista[nv], netlist[u].nome);
			netlist[u].x = nv;
			operacional(netlist[u].a, netlist[u].b, 0, netlist[u].x);
		}
		else if (tipo == 'F')
		{
			nv++;
			testarNos();
			strcpy(lista[nv], "j"); // Tem espaco para mais dois caracteres
			strcat(lista[nv], netlist[u].nome);
			netlist[u].x = nv;
			operacional(netlist[u].x, 0, netlist[u].c, netlist[u].d);
		}
		else if (tipo == 'H')
		{
			nv = nv + 2;
			testarNos();
			strcpy(lista[nv - 1], "jx");
			strcat(lista[nv - 1], netlist[u].nome);
			netlist[u].x = nv - 1;
			strcpy(lista[nv], "jy");
			strcat(lista[nv], netlist[u].nome);
			netlist[u].y = nv;
			operacional(netlist[u].a, netlist[u].b, 0, netlist[u].y);
			operacional(netlist[u].x, 0, netlist[u].c, netlist[u].d);
		}
		else if (tipo == 'O')
		{
			operacional(netlist[u].a, netlist[u].b, netlist[u].c, netlist[u].d);
			neq--;
		}
		else if (tipo == 'L')
		{
			nv++;
			neq++;
			//printf("elementosModificada neq:%i\r\n ",neq);
			testarNos();
			strcpy(lista[nv], "j");
			strcat(lista[nv], netlist[u].nome);
			netlist[u].x = nv;
		} 
	}
}

void listarTudo(void)
{
	printf("Variaveis internas:\r\n");
	for (u = 0; u <= nv; u++)
		printf("%d -> %s (%d)\r\n", u, lista[u], C[u]);
	getchar();

	printf("Netlist interno final\r\n");
	for (u = 1; u <= ne; u++)
	{
		tipo = netlist[u].nome[0];
		if (tipo == 'R' || tipo == 'I' || tipo == 'V' || tipo == 'C' || tipo == 'L')
		{
			printf("%s %d %d %f\r\n", netlist[u].nome, netlist[u].a, netlist[u].b, netlist[u].valor);
		}
		else if (tipo == 'G' || tipo == 'E' || tipo == 'F' || tipo == 'H')
		{
			printf("%s %d %d %d %d %f\r\n", netlist[u].nome, netlist[u].a, netlist[u].b,
				   netlist[u].c, netlist[u].d, netlist[u].valor);
		}
		else if (tipo == 'O' || tipo == '%')
		{
			printf("%s %d %d %d %d\r\n", netlist[u].nome, netlist[u].a, netlist[u].b,
				   netlist[u].c, netlist[u].d);
		}

		if (tipo == 'V' || tipo == 'E' || tipo == 'F' || tipo == 'O' || tipo == 'K' || tipo == 'L')
			printf("Corrente jx: %d\r\n", netlist[u].x);
		else if (tipo == 'H')
			printf("Correntes jx e jy: %d, %d\r\n", netlist[u].x, netlist[u].y);
	}
	getchar();
}

/* Rotina que calcula a corrente no capacitor para ser usada no proximo passo. */
void memoriaCapacitor(void)
{
	double e_a, e_b;

	for (s = 1; s <= ne; s++)
	{
		if (netlist[s].nome[0] == 'C')
		{
			if (!ponto)
				netlist[s].j_t0 = 0.0;
			else
			{
				if (C[netlist[s].a] == 0)
					e_a = 0.0;
				else
					e_a = Yn[C[netlist[s].a]][neq + 1];

				if (C[netlist[s].b] == 0)
					e_b = 0.0;
				else
					e_b = Yn[C[netlist[s].b]][neq + 1];

				netlist[s].j_t0 = (netlist[s].valor / (passo)) * ((e_a - e_b) - (t0[netlist[s].a] - t0[netlist[s].b]));
			}
		}
	}
}

/* Rotina que checa a convergencia do Newton-Raphson. */
void checarConvergencia(void)
{
	convergiu = 1;

	for (r = 1; r <= nv; r++)
	{
		if (C[r])
		{
			if (fabs(Yn[C[r]][neq + 1]) > E_MIN)
			{
				if (fabs((en[r] - Yn[C[r]][neq + 1]) / Yn[C[r]][neq + 1]) > TOLG_NR)
					convergiu = 0;
			}
			else if (fabs(en[r] - Yn[C[r]][neq + 1]) > TOLG_NR)
				convergiu = 0;

#ifdef DEBUG
			printf("Variavel %d, iteracao %d. Convergencia alcancada? %d\r\n", r, nroIteracoes + 1, convergiu);
			printf("Valor anterior: %f; Valor atual: %f\r\n", en[r], Yn[C[r]][neq + 1]);
			getchar();
#endif

			en[r] = Yn[C[r]][neq + 1];
		}
	}

	nroIteracoes++;
	if ((nroIteracoes == MAX_ITER) && (!convergiu))
	{
		for (r = 0; r <= nv; r++)
		{
			if (C[r])
				en[r] = rand() % FAIXA_RAND;
		}
		nroRandomizacoes++;
		nroIteracoes = 0;
	}
}


void montarEstampas(void)
{

	for (u = 1; u <= ne; u++)
	{
		switch (netlist[u].nome[0])
		{
		case 'R':
			condutancia(1 / netlist[u].valor, netlist[u].a, netlist[u].b);
			break;
		case 'G':
			transcondutancia(netlist[u].valor, netlist[u].a, netlist[u].b,
							 netlist[u].c, netlist[u].d);
			break;
		case 'I':
			corrente(valorFonte(netlist[u]), netlist[u].a, netlist[u].b);
			break;
		case 'V':

			tensao(valorFonte(netlist[u]), netlist[u].a, netlist[u].b, netlist[u].x);
			break;
		case 'E':
			ganhoTensao(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].c,
						netlist[u].d, netlist[u].x);
			break;
		case 'F':
			ganhoCorrente(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].c,
						  netlist[u].d, netlist[u].x);
			break;
		case 'H':
			transresistencia(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].c,
							 netlist[u].d, netlist[u].x, netlist[u].y);
			break;
		case 'K':
			transformador(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].c,
						  netlist[u].d, netlist[u].x);
			break;
		case 'C':
			if (!ponto && netlist[u].j_t0 == 0.0 ) 
				condutancia(1 / C_PO, netlist[u].a, netlist[u].b);
			else
				capacitor(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].j_t0);
			break;
		case 'L':
			indutor(netlist[u].valor, netlist[u].a, netlist[u].b, netlist[u].x,netlist[u].j_t0);
			break;
		case '(':
			portaNand(netlist[u].c, netlist[u].b, netlist[u].a, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, netlist[u].A);
			capacitor(netlist[u].cIn, netlist[u].a, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].b, 0, netlist[u].j_t0);
			break;
		case ')':
			portaAnd(netlist[u].c, netlist[u].b, netlist[u].a, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, netlist[u].A);
			capacitor(netlist[u].cIn, netlist[u].a, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].b, 0, netlist[u].j_t0);
			break;
		case '{':
			portaNor(netlist[u].c, netlist[u].b, netlist[u].a, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, netlist[u].A);
			capacitor(netlist[u].cIn, netlist[u].a, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].b, 0, netlist[u].j_t0);
			break;
		case '}':
			portaOr(netlist[u].c, netlist[u].b, netlist[u].a, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, netlist[u].A);
			capacitor(netlist[u].cIn, netlist[u].a, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].b, 0, netlist[u].j_t0);
			break;
		case '%':
			if (strcmp(netlist[u].resetName, ""))
			{
				for (int index = 0; index <= ne; index++)
				{
					if (!strcmp(netlist[u].resetName, netlist[index].nome))
					{
						flipflopD(netlist[u].nome, netlist[u].a, netlist[u].b, netlist[u].c, netlist[u].d, netlist[u].resetName, netlist[u].vOutMax,
								  netlist[u].rOut, netlist[u].cIn, &netlist[u].clkMuda, &netlist[u].Vo, netlist[index].a, netlist[index].b, netlist[index].vOutMax);
						break;
					}
				}
			}
			else
			{
				flipflopD(netlist[u].nome, netlist[u].a, netlist[u].b, netlist[u].c, netlist[u].d, netlist[u].resetName, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, &netlist[u].clkMuda, &netlist[u].Vo, 0, 0, 0.0);
			}


				capacitor(netlist[u].cIn, netlist[u].c, 0, netlist[u].j_t0);
				capacitor(netlist[u].cIn, netlist[u].d, 0, netlist[u].j_t0);
			break;

		case '!':
			capacitor(netlist[u].cIn, netlist[u].a, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].b, 0, netlist[u].j_t0);
			break;
		case '@':
			monoestavel(netlist[u].a, netlist[u].b, netlist[u].c, netlist[u].d, netlist[u].vOutMax, netlist[u].rOut, netlist[u].cIn, netlist[u].T, &netlist[u].clkMuda,&netlist[u].Vo,&netlist[u].T0);			
			capacitor(netlist[u].cIn, netlist[u].c, 0, netlist[u].j_t0);
			capacitor(netlist[u].cIn, netlist[u].d, 0, netlist[u].j_t0);
			break;
		case 'O':
			break;
		}
	}
}

int main()
{
	tempo = T_PADRAO;	 // Tempo de simulacao
	passo = PASSO_PADRAO; // Tamanho do passo
	passosInt = P_INT_PADRAO;

	system("cls");

	printf("OCS - Open Circuit Simulator - Analise Nodal Modificada (Versao %s)\r\n", versao);
	printf("Por Ewerton Vasconcelos(UFRJ), Gabriel Lopes (UFRJ).\r\n");
	printf("Codigo base por Antonio Carlos M. de Queiroz (UFRJ)\r\n");
	srand((unsigned int)time(NULL));

	for (u = 0; u <= MAX_NOS; u++)
	{ // Inicializacao de tabelas
		C[u] = u;
		L[u] = u;
		t0[u] = 0.0;
	}

	lerNetlist();		   // Chamada da rotina que le o netlist
	elementosModificada(); // Processamento de elementos da analise modificada
	listarTudo();		   // Listagem de variaveis e elementos

	passo = passo / passosInt;
	qtdePontos = (int)round(tempo / passo);
	correcaoPulse();

	//######### PROBLEMA ############
	newNeq = neq;
	char nome[20];
	int j;
	for (j = 0; nomeArquivo[j] != '.'; j++)
	{
		nomeValores[j] = nomeArquivo[j];
	}
	nomeValores[j++] = '.';
	nomeValores[j++] = 't';
	nomeValores[j++] = 'a';
	nomeValores[j++] = 'b';
	nomeValores[j++] = '\0';
	strcpy(nome, nomeValores);

	//######### PROBLEMA ############

	neq = newNeq;
	printf("O circuito tem %i nos, %i variaveis internas, %i equacoes e %i elementos.\r\n", nn, nv, neq, ne);
	getchar();

	valores = fopen(nome, "w");

	// Escreve as variaveis calculadas na primeira linha do .tab
	fprintf(valores, "t");
	for (r = 1; r <= nv; r++)
	{
		if (C[r] || r <= nn)
			fprintf(valores, " %s", lista[r]);
		if (r == nv)
			fprintf(valores, "\n");
	}

	inicio = clock();

	// Inicializamos o vetor de valores do Newton-Raphson com NR_INICIAL ou 0
	for (r = 0; r <= nv; r++)
	{
		if (C[r])
			en[r] = NR_INICIAL;
		else
			en[r] = 0.0;
	}

	// Calculo dos pontos
	for (ponto = 0; ponto <= qtdePontos; ponto++)
	{
		convergiu = 0;
		nroIteracoes = 0;
		nroRandomizacoes = 0;
		tolg = TOLG_PADRAO;

		while ((!convergiu) && (nroRandomizacoes <= MAX_RAND))
		{
#ifdef DEBUG
			printf("Iteracao %d, randomizacao %d, ponto %d.\r\n", nroIteracoes + 1, nroRandomizacoes, ponto);
			getchar();
#endif

			zerarSistema();
			montarEstampas();

			/* Esse while diminui a tolerancia caso o sistema seja singular, porque as vezes
* a singularidade ocorre por conta de erros numericos ou valores ruins no NR. */
			while (resolverSistema())
			{
#ifdef DEBUG
				printf("Possivel sistema singular. tolg diminuida para %f.\r\n", tolg);
				getchar();
#endif

				if (tolg > TOLG_MIN)
					tolg *= 1e-1;

				if (tolg < TOLG_MIN)
				{
#ifdef DEBUG
					printf("Sistema singular nao resolvido. Randomizando valores.\r\n");
					printf("Randomizacao %d.\r\n", nroRandomizacoes);
					getchar();
#endif

					for (r = 0; r <= nv; r++)
					{
						if (C[r])
							en[r] = rand() % FAIXA_RAND;
						else
							en[r] = 0.0;
					}

					nroRandomizacoes++;
					nroIteracoes = 0;
					tolg = TOLG_PADRAO;
					break;
				}
			} /* while resolverSistema */

			checarConvergencia();
		} /* while ((!convergiu) && (nroRandomizacoes <= MAX_RAND)) */

		// Se o sistema nao convergir em um ponto, saimos do programa
		if ((!convergiu) && (nroRandomizacoes > MAX_RAND))
		{
			printf("(!) ERRO: O sistema nao convergiu no ponto %d.\r\n", ponto);
			printf("Pressione qualquer tecla para sair...\r\n");
			getchar();
			fclose(valores);
			exit(1);
		}

		// Resultados registrados apos a convergencia do NR
		memoriaCapacitor();
		for (r = 0; r <= nv; r++)
		{
			if (C[r])
				t0[r] = Yn[C[r]][neq + 1];
			else
				t0[r] = 0.0;
		}

		// Escrevemos os resultados no arquivo
		if (ponto % passosInt == 0)
		{
			fprintf(valores, "%.7e", ponto * passo);
			for (r = 1; r <= nv; r++)
			{
				if (C[r] || r <= nn)
					fprintf(valores, " %.7e", t0[r]);
				if (r == nv)
					fprintf(valores, "\n");
			}
		}
	}

	fim = clock();

	// Informamos a quantidade de pontos calculados e o tempo de analise
	printf("Pronto. %d pontos calculados internamente; %d foram incluidos na tabela.\r\n",
		   ponto - 1, (ponto - 1) / passosInt);
	printf("O programa demorou %.4f s para simular o circuito.\r\n", (double)(fim - inicio) / CLOCKS_PER_SEC);
	getchar();
	fclose(valores);

	return 0;
}