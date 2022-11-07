#ifndef GERADOR_H
#define GERADOR_H

typedef struct posicao{
   int jogada[5]; /*jogada que originou essa posicao [tipo, col_saida, lin_saida, col_chegada, lin_chegada] / tipo: 0 = jogada normal, 1 = captura, 2 = O-O, 3 = O-O-O, 4 = nulo / coordenadas do tabuleiro com moldura. Por exemplo, um cavalo em f2 que se move para d3 e representado por {0, 7, 3, 5, 4} */

   /* Informacoes necessarias sobre a posicao */
   int tabuleiro[12][12]; //com 2 linhas e colunas de moldura, marcadas como 0. Pecas sao representadas pelo codigo (ver int codigo(char c) em gerador.c), numero positivos para pecas brancas e numeros negativos para pretas. Casas vazias são marcadas como 10.
   int turno; //1 = brancas movem, -1 = pretas movem
   int enpassant[2]; //caso um peao tenha movido duas casas na jogada anterior, a casa en passant registra a casa intermediaria. Caso contrario, sera {0, 0}
   int roque[4]; //possibilidades de roque {Brancas King-Side, BQS, PKS, PQS}

   /* Informacoes extras usadas pelo avaliador e gerador */
   int valor; //valor estimado para a posicao
   int profundidade;  //estagio da avaliacao:   -20 = sem avaliacao, -19 = abertura, -10 = estatico, -10 + q = quiescense, 0 = quiescense completa, 0 + p = dinamico com profundidade p, 20 = final.
   int gerado; // 0 = sem movimentos, 1 = quiescense, 2 = completo
   int nmov; // numero de movimentos gerados
   struct posicao* movimentos[80]; // vetor de apontadores para os movimentos gerados
   struct posicao* pai;
} Posicao;

void gerador_completo(Posicao* pos); /*supoe pos->gerado = 0 ou 1 e altera para 2*/
void gerador_parcial(Posicao* pos); /*supoe pos->gerado = 0 e altera para 1. inclui movimento nulo na posicao 0*/

void elimina_xeques(Posicao* pos); // verifica todos os filhos e elimina aqueles em que o rei está em xeque (jogadas invalidas)
int sob_ataque(Posicao* pos, int casa[]); //retorna o numero de pecas do oponente que atacam casa[]
void libera_posicao(Posicao* pos); /*libera posicao e todos seus filhos*/
void libera_pilha();
char letra(int n);    // retorna a letra a partir do codigo, ou 0 caso entrada invalida
int codigo(char c); // retorna o codigo da peca, ou 0 caso entrada invalida

#endif /* GERADOR_H */
