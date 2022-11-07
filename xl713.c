/********************************************************************
*   SUPER CHESS-ENGINE XL-713 (nome provisorio), versao 1.4   beta                    *
*   Autor: Fabricio Caluza Machado                                                                            *
*                                                                                                                                     *
*                                                                                                                                     *
*   Main contem a interface do jogo, funcoes que controlam a producao            *
* do tabuleiro inicial, o desenvolvimento do jogo e a criacao de backups.           *
*                                                                                                                                     *
*   Avaliador contem a engine que decide os movimentos do computador,         *
* baseado na heuristica minimax com cortes alfa-beta.                                           *
*                                                                                                                                      *
*   Gerador contem as estruturas usadas na representacao do tabuleiro e           *
* funcoes que geram os movimentos validos a partir de certa posicao.                *
*                                                                                                                                     *
*********************************************************************/
#include "gerador.h"
#include "avaliador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void le_jogada(int jogada[], int *num, char *c); // Le a jogada do jogador e, se a sintaxe estiver correta, retorna movimento em jogada[], num, c (ver int jogada[4] em struct posicao, gerador.h )
void le_jogada2(int jogada[], int *num, char *c, char jogadac[]); // semelhante a funcao anterior, usada pra ler as jogadas do dicionario de aberturas

Posicao* inicio(); // retorna o tabuleiro inicial de xadrez
void imprime(FILE* saida, Posicao* pos, int move, int jogador); // imprime a posicao atual em saida
void imprime_back(char nome[], Posicao* pos, int move, int jogador); //imprime backup
Posicao* leitura_back(char nome[], int *move, int *jogador); // le backup
void leitura_aberturas(Posicao* inicio); // le dicionario de aberturas e reproduz a arvore em inicio


int main(){
   Posicao* pos, *aux;
   int i, j, jogador = 1, move = 0; // jogador guarda a informacao sobre qual lado jogador escolheu ( 1 = brancas, -1 = pretas). move guarda quantos movimentos ja foram feitos, e serve como flag no menu de jogo interrompido
   char c = 'a';
   int jogada[4], num;
   FILE* saida;

   printf("*************    SUPER CHESS-ENGINE XL-713 (nome provisorio)  *************\n");
   printf("versao 1.4 beta                                               Por Fabricio.\n");


   pos = leitura_back("backup.txt", &move, &jogador); //na ausencia de backup, pos recebe posicao inicial.
   if(move != 0)
      printf("\nJogo interrompido na ultima sessao:\n");
   imprime(stdout, pos, move, jogador); // imprime posicao atual (interrompida da ultima sessao ou inicial)

   if(move != 0){ // Menu de jogo interrompido
      while( c != 'N' && c != 'C' ){
         printf("\nO que deseja fazer?\n[C] Continuar jogo\n[N] Iniciar novo jogo\n");
         scanf("%c", &c);
         scanf("%*[^\n]");
         scanf("%*c");
         if( c != 'C' && c != 'N' )
            printf("Digite C ou N\n");
      }
      if( c == 'N' ){
         move = 0;
         pos = inicio();
         imprime(stdout, pos, move, 1);
      }
   }

   if(move == 0){ // Menu de inicio de jogo, usado pra definir variavel jogador
      while( c != 'P' && c != 'B' ){
         printf("\nDe que lado deseja jogar?\n[B] Brancas\n[P] Pretas\n");
         scanf("%c", &c);
         scanf("%*[^\n]");
         scanf("%*c");
         if( c != 'P' && c != 'B' )
            printf("Digite B ou P\n");
      }
      if(c == 'B')
         jogador = 1; // jogador escolheu Brancas
      else
         jogador = -1; // jogador escolheu Pretas
   }
   if(pos->turno == 1)
         move++;

   while(1){ // um turno de cada lado
      if(pos->turno == jogador){ // turno do jogador
         gerador_completo(pos); // gera posicoes para verificar se a jogada e valida
         while(1){
            le_jogada(jogada, &num, &c); // le jogada do jogador e grava no vetor jogada (ver struct posicao), num e c servem pra desempate no caso de jogadas diferentes com notacoes iguais

            // verifica se a jogada e valida
            for(i = 0; i < pos->nmov; i++){
               if(jogada[0] == pos->movimentos[i]->jogada[0] && jogada[2] == pos->movimentos[i]->jogada[3] && jogada[3] == pos->movimentos[i]->jogada[4] && jogada[1] == abs( pos->tabuleiro[ pos->movimentos[i]->jogada[1] ][ pos->movimentos[i]->jogada[2] ] ) ){
                  if(c != 0){
                     if( jogada[1] == 1){
                        if( pos->movimentos[i]->jogada[1] != (c - 'a' + 2) )
                           continue;
                     }
                     else{
                        for(j = 2; j <= 9; j++)
                           if( pos->movimentos[i]->tabuleiro[c-'a'+2][j] * jogador == jogada[1] )
                              c = 0;
                        if (c == 0)
                           continue;
                     }
                  }
                  if(num != 0){
                     for(j = 2; j <= 9; j++)
                        if( pos->movimentos[i]->tabuleiro[j][num] * jogador == jogada[1] )
                           num = 0;
                     if (num == 0)
                        continue;
                  }
                  break;
               }
               else if( (jogada[0] == 2 || jogada[0] == 3) && (jogada[0] == pos->movimentos[i]->jogada[0]) )
                  break;
            }
            if(i == pos->nmov)
               continue; //se nenhuma jogada for valida, voltar para le_jogada

            aux = pos->movimentos[0]; //transfere a jogada para a posicao 0
            pos->movimentos[0] = pos->movimentos[i];
            pos->movimentos[i] = aux;
            break;
         }

         for(i = 1; i < pos->nmov; i++) //libera as outras jogadas possiveis
            libera_posicao(pos->movimentos[i]);
         pos->nmov = 1;

         pos = pos->movimentos[0]; //atualiza a posicao atual
         imprime(stdout, pos, move, jogador); //imprime a posicao
       //  imprime_back("backup.txt", pos, move, jogador);
      }

      else{ // turno do computador
         printf("\n\nPensando, por favor, aguarde...");
         pos = engine(pos); // funcao que retorna a jogada do computador. Informacao sobre o movimento que originou a jogada em pos->jogada

         fprintf(stdout, "%c", 13);
         imprime(stdout, pos, move, jogador);
         imprime_back("backup.txt", pos, move, jogador);
      }

      // verificacao de cheque-mate
      if(pos->profundidade == 20 && pos->turno == jogador){
         printf("\nVoce perdeu! Boa sorte da proxima vez!\n");
         break;
      }
      if(pos->profundidade == 20 && pos->turno != jogador){
         printf("\nVoce venceu! Parabens!\n");
         break;
      }

      //contador de movimentos
      if(pos->turno == 1)
         move++;
   }

   //libera estruturas usadas
   saida = fopen("backup.txt", "w+");
   fclose(saida);
   libera_pilha();
   while(pos->pai != NULL)
      pos = pos->pai;
   libera_posicao(pos);
   getchar(); // pausa, para o jogador poder ler a mensagem de vitoria ou derrota.
   return 0;
}


void le_jogada(int jogada[], int *num, char *c){ // le entrada em notacao algebrica e grava em jogada com o tipo [tipo][peca][col_chegada][lin_chegada]
   char jogadac[6];

   while(1){
      printf("\n\nSua jogada: ");
      scanf("%5[^\n]", jogadac);
      scanf("%*[^\n]");
      scanf("%*c");
      *c = 0;         // c e num carregam informacao de desempate, ja que e possivel duas jogadas
      *num = 0;    // diferentes com mesma notacao jogada[]. Se nao forem usados, seu valor deve ser 0.

      // de acordo com o comprimento da string lida, a funcao faz testes para verificar se a sintaxe esta correta e gravar os valores em jogada[],
      //se for detectado erro na entrada, um continue ira retornar para o inicio do loop, para nova leitura. Caso contrario, o break no final encerra a funcao.
      if(strlen(jogadac) == 2){
         if(jogadac[0] >= 'a' && jogadac[0] <= 'h' && jogadac[1] >= '1' && jogadac[1] <= '8'){
            jogada[0] = 0;
            jogada[1] = 1;
            jogada[2] = jogadac[0] - 'a' + 2;
            jogada[3] = jogadac[1] - '0' + 1;
         }
         else
            continue;
      }
      else if(strlen(jogadac) == 3){
         if(jogadac[0] == 'O' && jogadac[1] == '-' && jogadac[2] == 'O'){ //Roque
            jogada[0] = 2;
            jogada[1] = 1;
            jogada[2] = 0;
            jogada[3] = 0;
         }
         else if( codigo(jogadac[0]) ){
            if(jogadac[1] >= 'a' && jogadac[1] <= 'h' && jogadac[2] >= '1' && jogadac[2] <= '8'){
               jogada[0] = 0;
               jogada[1] = abs( codigo(jogadac[0]) );
               jogada[2] = jogadac[1] - 'a' + 2;
               jogada[3] = jogadac[2] - '0' + 1;
            }
         }
         else
            continue;
      }
      else if(strlen(jogadac) == 4){
         if((jogadac[0] >= 'a' && jogadac[0] <= 'h') || codigo(jogadac[0]) ){
            if(jogadac[0] >= 'a' && jogadac[0] <= 'h'){
               jogada[1] = 1;
               *c = jogadac[0];
            }
            else
               jogada[1] = abs(codigo(jogadac[0]));
            if(jogadac[1] == 'x')
               jogada[0] = 1;
            else if(jogadac[1] >= '1' && jogadac[1] <= '8'){
               jogada[0] = 0;
               *num = jogadac[1] - '0' + 1;
            }
            else if(jogadac[1] >= 'a' && jogadac[1] <= 'h'){
               jogada[0] = 0;
               *c = jogadac[1];
            }
            else
               continue;
            if(jogadac[2] >= 'a' && jogadac[2] <= 'h' && jogadac[3] >= '1' && jogadac[3] <= '8'){
               jogada[2] = jogadac[2] - 'a' + 2;
               jogada[3] = jogadac[3] - '0' + 1;
            }
            else
               continue;
         }
         else
            continue;
      }
      else if(strlen(jogadac) == 5){
         if(jogadac[0] == 'O' && jogadac[1] == '-' && jogadac[2] == 'O' && jogadac[3] == '-' && jogadac[4] == 'O'){ // roque
            jogada[0] = 3;
            jogada[1] = 0;
            jogada[2] = 0;
            jogada[3] = 0;
         }
         else if ( codigo(jogadac[0]) ){
            jogada[1] = abs(codigo(jogadac[0]));
            if(jogadac[1] >= '1' && jogadac[1] <= '8')
               *num = jogadac[1] - '0' + 1;
            else if(jogadac[1] >= 'a' && jogadac[1] <= 'h')
               *c = jogadac[1];
            else
               continue;
            if(jogadac[2] == 'x')
               jogada[0] = 1;
            else
               continue;
            if(jogadac[3] >= 'a' && jogadac[3] <= 'h' && jogadac[4] >= '1' && jogadac[4] <= '8'){
               jogada[2] = jogadac[3] - 'a' + 2;
               jogada[3] = jogadac[4] - '0' + 1;
            }
            else
               continue;
         }
         else
            continue;
      }
      else
         continue;
      break;
   }
}

void le_jogada2(int jogada[], int *num, char *c, char jogadac[]){ // le entrada em notacao algebrica
   while(1){
      *c = 0;         // c e num carregam informacao de desempate, ja que e possivel duas jogadas
      *num = 0;    // diferentes com mesma notacao jogada[]. Se nao forem usados, seu valor deve ser 0.

      // de acordo com o comprimento da string lida, a funcao faz testes para verificar se a sintaxe esta correta e gravar os valores em jogada[],
      //se for detectado erro na entrada, um break ira sair do loop e uma mensagem de erro sera exibida. Caso contrario, o return no final encerra a funcao.
      if(strlen(jogadac) == 2){
         if(jogadac[0] >= 'a' && jogadac[0] <= 'h' && jogadac[1] >= '1' && jogadac[1] <= '8'){
            jogada[0] = 0;
            jogada[1] = 1;
            jogada[2] = jogadac[0] - 'a' + 2;
            jogada[3] = jogadac[1] - '0' + 1;
         }
         else
            break;
      }
      else if(strlen(jogadac) == 3){
         if(jogadac[0] == 'O' && jogadac[1] == '-' && jogadac[2] == 'O'){ //Roque
            jogada[0] = 2;
            jogada[1] = 1;
            jogada[2] = 0;
            jogada[3] = 0;
         }
         else if( codigo(jogadac[0]) ){
            if(jogadac[1] >= 'a' && jogadac[1] <= 'h' && jogadac[2] >= '1' && jogadac[2] <= '8'){
               jogada[0] = 0;
               jogada[1] = abs( codigo(jogadac[0]) );
               jogada[2] = jogadac[1] - 'a' + 2;
               jogada[3] = jogadac[2] - '0' + 1;
            }
         }
         else
            break;
      }
      else if(strlen(jogadac) == 4){
         if((jogadac[0] >= 'a' && jogadac[0] <= 'h') || codigo(jogadac[0]) ){
            if(jogadac[0] >= 'a' && jogadac[0] <= 'h'){
               jogada[1] = 1;
               *c = jogadac[0];
            }
            else
               jogada[1] = abs(codigo(jogadac[0]));
            if(jogadac[1] == 'x')
               jogada[0] = 1;
            else if(jogadac[1] >= '1' && jogadac[1] <= '8'){
               jogada[0] = 0;
               *num = jogadac[1] - '0' + 1;
            }
            else if(jogadac[1] >= 'a' && jogadac[1] <= 'h'){
               jogada[0] = 0;
               *c = jogadac[1];
            }
            else
               break;
            if(jogadac[2] >= 'a' && jogadac[2] <= 'h' && jogadac[3] >= '1' && jogadac[3] <= '8'){
               jogada[2] = jogadac[2] - 'a' + 2;
               jogada[3] = jogadac[3] - '0' + 1;
            }
            else
               break;
         }
         else
            break;
      }
      else if(strlen(jogadac) == 5){
         if(jogadac[0] == 'O' && jogadac[1] == '-' && jogadac[2] == 'O' && jogadac[3] == '-' && jogadac[4] == 'O'){ // roque
            jogada[0] = 3;
            jogada[1] = 0;
            jogada[2] = 0;
            jogada[3] = 0;
         }
         else if ( codigo(jogadac[0]) ){
            jogada[1] = abs(codigo(jogadac[0]));
            if(jogadac[1] >= '1' && jogadac[1] <= '8')
               *num = jogadac[1] - '0' + 1;
            else if(jogadac[1] >= 'a' && jogadac[1] <= 'h')
               *c = jogadac[1];
            else
               break;
            if(jogadac[2] == 'x')
               jogada[0] = 1;
            else
               break;
            if(jogadac[3] >= 'a' && jogadac[3] <= 'h' && jogadac[4] >= '1' && jogadac[4] <= '8'){
               jogada[2] = jogadac[3] - 'a' + 2;
               jogada[3] = jogadac[4] - '0' + 1;
            }
            else
               break;
         }
         else
            break;
      }
      else
         break;
      return;
   }

   fprintf(stderr, "[aberturas] erro de leitura %s\n", jogadac);
   exit(-1);
}

Posicao* inicio(){
   Posicao* pos = (Posicao*) malloc(sizeof(Posicao));
   int i, j, aux=0;

   pos->turno = 1;
   for(i=0; i<4; i++)
      pos->roque[i] = 1;
   pos->enpassant[0] = 0;
   pos->enpassant[1] = 0;
   pos->nmov=0;
   pos->gerado=0;
   pos->pai=NULL;
   pos->jogada[0] = 4;
   pos->jogada[1] = 0;
   pos->jogada[2] = 0;
   pos->jogada[3] = 0;
   pos->profundidade = -20;
   pos->valor = 0;

   for(j = 11; j>=0; j--)
      for(i = 0; i<12; i++){
         if(i<2 || i>9 || j<2 || j>9)
            pos->tabuleiro[i][j] = 0;
         else{
            if(j<8 && j>3)
               pos->tabuleiro[i][j] = 10;
            else{
               if(j == 3 || j == 2)
                  aux = 1;
               else
                  aux = -1;

               switch(i){
                  case 2:
                  case 9:
                     pos->tabuleiro[i][j] = 2 * aux;
                     break;
                  case 3:
                  case 8:
                     pos->tabuleiro[i][j] = 3 * aux;
                     break;
                  case 4:
                  case 7:
                     pos->tabuleiro[i][j] = 4 * aux;
                     break;
                  case 5:
                     pos->tabuleiro[i][j] = 5 * aux;
                     break;
                  case 6:
                     pos->tabuleiro[i][j] = 6 * aux;
                     break;
               }
               if( j == 3 || j == 8)
                  pos->tabuleiro[i][j] = aux ;
            }
         }
      }

   leitura_aberturas(pos);

   return pos;
}

void imprime(FILE* saida, Posicao* pos, int move, int jogador){
   int i, j;

   if(pos->turno == -1 && move != 0)
      fprintf(saida, "%d. ", move);
   else if (move != 0)
      fprintf(saida, "%d. ... ", move);

   if(pos->jogada[0] == 0 || pos->jogada[0] == 1){
      if( (pos->tabuleiro[pos->jogada[3]][pos->jogada[4]] != 1 && pos->tabuleiro[pos->jogada[3]][pos->jogada[4]] != -1 ) )
         fprintf(saida, "%c",  letra( abs( pos->tabuleiro[ pos->jogada[3] ][ pos->jogada[4] ] ) ) );
      else if( pos->jogada[0] == 1 )
         fprintf(saida, "%c", pos->jogada[1] - 2 + 'a');
      if(pos->jogada[0] == 1)
         fprintf(saida, "x");
      fprintf(saida, "%c", pos->jogada[3] - 2 + 'a');
      fprintf(saida, "%d", pos->jogada[4]-1);
   }
   else if(pos->jogada[0] == 2)
      fprintf(saida, "O-O");
   else if(pos->jogada[0] == 3)
      fprintf(saida, "O-O-O");
   fprintf(saida, "                                     \n\n");

   for(j = (5.5 + 3.5*jogador); (jogador == 1 && j >= 2) || (jogador == -1 && j <= 9); j -= jogador){
      fprintf(saida, "%d   ", j-1);
      for(i = (5.5 - 3.5*jogador); (jogador == -1 && i > 2) || (jogador == 1 && i < 9); i += jogador)
         fprintf(saida, "%c  ", letra(pos->tabuleiro[i][j]) );
      fprintf(saida, "%c\n\n", letra(pos->tabuleiro[i][j]) );
   }
   if(jogador == 1)
      fprintf(saida, "\n    a  b  c  d  e  f  g  h\n");
   else
      fprintf(saida, "\n    h  g  f  e  d  c  b  a\n");
}

void imprime_back(char nome[], Posicao* pos, int move, int jogador){
   int i, j;
   FILE* saida;

   saida = fopen(nome, "w+");
   fclose(saida);
   saida = fopen(nome, "a+");

   fprintf(saida, "Arquivo backup XL-713 (nome provisorio)\n");
   fprintf(saida, "%d %d %d %d %d ", pos->jogada[0], pos->jogada[1], pos->jogada[2], pos->jogada[3], pos->jogada[4]);
   fprintf(saida, "%d %d %d %d ", pos->roque[0], pos->roque[1], pos->roque[2], pos->roque[3]);
   fprintf(saida, "%d %d ", pos->enpassant[0], pos->enpassant[1]);
   fprintf(saida, "%d %d %d\n", pos->turno, jogador, move);

   for(j = 9; j>=2 ; j--){
      for(i = 2; i < 9; i++)
         fprintf(saida, "%c ", letra( pos->tabuleiro[i][j] ) );
      fprintf(saida, "%c\n", letra( pos->tabuleiro[i][j] ) ) ;
   }
   fclose(saida);
}

Posicao* leitura_back(char nome[], int *move, int *jogador){
   Posicao* pos = (Posicao*) malloc(sizeof(Posicao));
   FILE* entrada = fopen(nome, "r");
   if(entrada == NULL)
      return inicio();
   int i, j;
   char string[80], c;
   string[0] = '\0';

   fscanf(entrada, "%s", string);
   if(strlen(string) < 3)
      return inicio();
   fscanf(entrada, "%*[^\n]");
   fscanf(entrada, "%*c");

   fscanf(entrada, "%d %d %d %d %d ", &pos->jogada[0], &pos->jogada[1], &pos->jogada[2], &pos->jogada[3], &pos->jogada[4]);
   fscanf(entrada, "%d %d %d %d ", &pos->roque[0], &pos->roque[1], &pos->roque[2], &pos->roque[3]);
   fscanf(entrada, "%d %d ", &pos->enpassant[0], &pos->enpassant[1]);
   fscanf(entrada, "%d %d %d%*c", &pos->turno, jogador, move);
   pos->nmov=0;
   pos->gerado=0;
   pos->pai=NULL;
   pos->profundidade = -20;
   pos->valor = 0;

   for(j = 11; j>=0; j--)
      for(i = 0; i<12; i++){
         if(i<2 || i>9 || j<2 || j>9)
            pos->tabuleiro[i][j] = 0;
         else{
            fscanf(entrada, "%c%*c", &c);
            pos->tabuleiro[i][j] = codigo(c);
         }
      }

   fclose(entrada);
   return pos;
}

void leitura_aberturas(Posicao* inicio){
   char jogadac[6], c;
   int jogada[4], num, i, j;
   FILE* dicionario;
   Posicao* pos, *aux;

   dicionario = fopen("aberturas.xl7", "r");
   if( dicionario == NULL){
      printf("Dicionario de aberturas nao encontrado\n\n");
      return;
   }

   fscanf(dicionario, "%s", jogadac);
   while( strcmp(jogadac, "#") != 0 ){ // inicio de uma nova linha (abertura)
      pos = inicio;
      while( strcmp(jogadac, "#") != 0 ){ // leitura de uma jogada
         le_jogada2(jogada, &num, &c, jogadac); // converte notacao, semelhante ao que e feito me main

         gerador_completo(pos);
         for(i = 0; i < pos->nmov; i++){
            if(jogada[0] == pos->movimentos[i]->jogada[0] && jogada[2] == pos->movimentos[i]->jogada[3] && jogada[3] == pos->movimentos[i]->jogada[4] && jogada[1] == abs( pos->movimentos[i]->tabuleiro[ jogada[2] ][ jogada[3] ] ) ){
               if(c != 0){
                  if( jogada[1] == 1){
                     if( pos->movimentos[i]->jogada[1] != (c - 'a' + 2) )
                        continue;
                  }
                  else{
                     for(j = 2; j <= 9; j++)
                        if( pos->movimentos[i]->tabuleiro[c-'a'+2][j] * pos->turno == jogada[1] )
                           c = 0;
                     if (c == 0)
                        continue;
                  }
               }
               if(num != 0){
                  for(j = 2; j <= 9; j++)
                     if( pos->movimentos[i]->tabuleiro[j][num] * pos->turno == jogada[1] )
                        num = 0;
                  if (num == 0)
                     continue;
               }
               break;
            }
            else if( (jogada[0] == 2 || jogada[0] == 3) && (jogada[0] == pos->movimentos[i]->jogada[0]) )
               break;
         }
         if(i == pos->nmov){ // jogada nao encontrada
            fprintf(stderr, "[aberturas] jogada invalida %s\n", jogadac);
            exit(-1);
         }

         pos = pos->movimentos[i];

         fscanf(dicionario, "%s", jogadac);
      }

      fscanf(dicionario, "%d", &pos->valor);
      pos->pai->profundidade = -19;

      fscanf(dicionario, "%s", jogadac);
   }
}
