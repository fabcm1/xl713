#include "gerador.h"
#include "avaliador.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define PROF 10

void aval_dinamico(Posicao* pos, int profundidade, int alfa, int existe_alfa); // avalia dinamicamente uma posicao com a profundidade dada. Atribui pos->valor a posicao e a seus filhos.
void aval_quiescence(Posicao* pos, int profundidade, int alfa, int existe_alfa); // executa avalicao quiescense na posicao, atraves de uma busca apenas nos movimentos de captura.
void aval_estatico(Posicao* pos); // avalia estaticamente a posicao

/*****************  Funcoes Auxiliares   ****************/
int compara(Posicao* pos1, Posicao* pos2); // retorna 1 se pos1 for igual a pos2 e 0 caso contrario
int sob_ataque2(Posicao* pos, int casa[]); //retorna o numero de pecas brancas menos o numero de pecas pretas que atacam casa[]
int sob_ataque3(Posicao* pos, int casa[]); // supoe rei ou rainha em casa[]. Retorna 2 vezes o numero de ataques diretos + o numero de cravos (que não sejam em peoes) sobre a peca em casa[]
void insertion (Posicao* v[], int p, int r); // ordenacao
void zera_table();  // inicializa tabelas usadas para agilizar o processo alfa-beta
char table[PROF][8][8][8][8]; // table[profundidade][col_saida][lin_saida][col_chegada][lin_chegada]

Posicao* engine(Posicao* pos){
   int i, k, peso = 0, r, turno;
   clock_t tempo, tinicio;

   if( pos->profundidade == -19){ // abertura
      turno = pos->turno;
      pos->turno = 1;
      ordena(pos);
      pos->turno = turno;
      for(i = 0; i < pos->nmov; i++)
         peso += pos->movimentos[i]->valor;
      srand(time(NULL));
      r = rand() % peso;

      for(i = -1; r >= 0; i++)
         r -= pos->movimentos[i+1]->valor;
   }
   else {
      zera_table();
      gerador_completo(pos); // gera movimentos completos de pos antes do avaliador para garantir que o movimento nulo nao seja considerado.
      elimina_xeques(pos); // remove movimentos invalidos
      if( pos->nmov == 0) // empate por afogamento ou xeque-mate
         return pos;
      for(k = 0; k < pos->nmov; k++){ // verifica jogadas que levam a empates por afogamento ou xeque-mates
         gerador_completo(pos->movimentos[k]);
         elimina_xeques(pos->movimentos[k]);

         if(pos->movimentos[k]->valor == 10000*pos->turno)
            return pos->movimentos[k];
      }

      aval_estatico(pos);
      i = 6;  tempo = 0;
      while(tempo <= 1200){
         tinicio = clock();
         aval_dinamico(pos, i++, 0, 0); //avalia a posicao
         tempo += clock() - tinicio;
      }
      if( pos->pai != NULL && pos->pai->pai != NULL && (pos->valor - pos->pai->pai->valor) * pos->turno <= -150 && tempo <= 6000 )
         aval_dinamico(pos, i++, 0, 0); //avalia a posicao mais uma vez
      ordena(pos); // apos a ordenacao, o melhor movimento estara em pos->movimentos[0]

      if(pos->movimentos[0]->jogada[0] == 4){  // verifica movimento nulo, por seguranca.
         fprintf(stderr, "[engine] movimento nulo\n");
         exit(-1);
      }
      for(i = 1; i < pos->nmov; i++) // libera o resto dos movimentos
         libera_posicao(pos->movimentos[i]);

      pos->nmov = 1;
      pos->gerado = 0;
      i = 0;
   }

   return pos->movimentos[i]; // retorna a melhor jogada
}

void ordena(Posicao* pos){
   int i, j, profundidade;
   Posicao* aux;
   if(pos->nmov == 0){
      fprintf(stderr, "[ordenador] nao existem movimentos disponiveis\n");
      exit(-1);
   }

   profundidade = pos->movimentos[0]->profundidade;
   for(i = 1; i < pos->nmov; i++)
      if(pos->movimentos[i]->profundidade > profundidade)
         profundidade = pos->movimentos[i]->profundidade;
   if(pos->profundidade >= 0 && pos->profundidade - 1 < profundidade)
      profundidade = pos->profundidade - 1;
   if(pos->profundidade == -19)
      profundidade = -20;
   // profundidade = min ( max(pos->movimentos[i]->profundidade) , pos->profundidade - 1 ), se pos ja tiver saido da quiescense

   for(i = 1; i < pos->nmov; i++)
      if(pos->movimentos[i]->profundidade < profundidade)
         break;

   for(j = i+1; j < pos->nmov; j++) // traz os movimentos com profundidade maior igual que profundidade para o inicio do vetor
      if(pos->movimentos[j]->profundidade >= profundidade){
         aux = pos->movimentos[i];
         pos->movimentos[i] = pos->movimentos[j];
         pos->movimentos[j] = aux;
         i++;
      }

   insertion(pos->movimentos, 0, i-1); // apenas ordena os movimentos com maior profundidade
}

void aval_dinamico(Posicao* pos, int profundidade, int alfa, int existe_alfa){
   int i, j, beta, existe_beta = 0, iguais = 0;
   Posicao *aux1;

   // verifica empate por repeticao tripla:
   aux1 = pos->pai;
   while(aux1 != NULL && aux1->pai != NULL && aux1->jogada[0] == 0 && aux1->pai->jogada[0] == 0 && iguais != 2){
      aux1 = aux1->pai;
      iguais += compara(aux1, pos);
      aux1 = aux1->pai;
   }
   if ( iguais == 2 ){
      pos->valor = 0;
      pos->profundidade = 20;
   }

   if(profundidade <= pos->profundidade) // nao faz nada, caso a posicao ja tenha sido avaliada com profundidade maior
      return;
   if(profundidade == 0){ // na profundidade 0, executa avaliacao quiescense
      aval_quiescence(pos, 2, alfa, existe_alfa);
      pos->profundidade = 0;
   }
   else{
      //HEURISTICAS DE PRE-ORDENACAO - usadas para agilizar o alfa-beta
      aval_dinamico(pos, profundidade-1, 0, 0); // aqui o aval_dinamico e chamado antes do gerador_completo, pois o movimento nulo pode ser considerado
      gerador_completo(pos);
      elimina_xeques(pos);
      if( pos->nmov == 0 )
         return;

      for(i = 0; i < pos->nmov; i++){
         if(profundidade - 1< PROF && table[profundidade-1][pos->movimentos[i]->jogada[1] - 2][pos->movimentos[i]->jogada[2]-2][pos->movimentos[i]->jogada[3]-2][pos->movimentos[i]->jogada[4]-2] == 1 ){
            if(pos->movimentos[i]->profundidade < profundidade -2)
               pos->movimentos[i]->profundidade = profundidade - 2;
            pos->movimentos[i]->valor += 2000*pos->turno; // da um bonus para as posicoes encontradas na tabela, de modo que elas sejam avaliadas primeiro
         }
         else if(pos->movimentos[i]->jogada[0] == 1){
            if(pos->movimentos[i]->profundidade < profundidade -2)
               pos->movimentos[i]->profundidade = profundidade - 2;
            pos->movimentos[i]->valor += 500*pos->turno; // da um bonus para posicoes de captura
         }
      }
      ordena(pos); // ordena posicoes segundo a pre-avaliacao e bonus dados anteriormente

      for(i = 0; i < pos->nmov; i++){
         aval_dinamico(pos->movimentos[i], profundidade - 1, beta, existe_beta); // avalia movimentos com profundidade-1

         // libera os netos
         for(j = 0; j < pos->movimentos[i]->nmov; j++)
            libera_posicao(pos->movimentos[i]->movimentos[j]);
         pos->movimentos[i]->nmov = 0;
         pos->movimentos[i]->gerado = 0;

         if(!existe_beta || pos->movimentos[i]->valor*pos->turno > beta*pos->turno){
            beta = pos->movimentos[i]->valor; // beta deve conter o valor do melhor movimento encontrado ate entao
            existe_beta = 1;
            if(existe_alfa && beta*pos->turno >= alfa*pos->turno){ // se beta maior que alfa (melhor movimento na instancia superior do aval_dinamico), para avaliacao (corte!)
               beta += pos->turno; // aumenta em 1 o valor de beta para desempatar posicao e garantir que ela nao sera escolhida na instancia superior de aval_dinamico
               if(profundidade - 1 < PROF)
                  table[profundidade-1][pos->movimentos[i]->jogada[1] - 2][pos->movimentos[i]->jogada[2]-2][pos->movimentos[i]->jogada[3]-2][pos->movimentos[i]->jogada[4]-2] = 1; // grava movimento gerador do corte na tabela, para ser testado primeiro nos ramos vizinhos da arvore
               break;
            }
         }
      }

      pos->valor = beta; // atribui o valor do melhor movimento a pos->valor
      pos->profundidade = profundidade;
   }
}

void aval_quiescence(Posicao* pos, int profundidade, int alfa, int existe_alfa){
   int i, j, beta, existe_beta = 0;

   if(pos->profundidade >= 0) // nao faz nada, caso posicao ja tenha sido avaliada por quiescense
      return;

   if(profundidade == 0) // executa avaliador estatico na profundidade 0 (folhas da arvore)
      aval_estatico(pos);
   else{
      gerador_parcial(pos); // em aval_quiescense, apenas movimentos de captura e nulos sao gerados

      //HEURISTICAS DE ORDENACAO
      /*for(i = 0; i < pos->nmov; i++){
         if(profundidade - 1< PROF && quiestable[profundidade-1][pos->movimentos[i]->jogada[1] - 2][pos->movimentos[i]->jogada[2]-2][pos->movimentos[i]->jogada[3]-2][pos->movimentos[i]->jogada[4]-2] == 1 ){
            if(pos->movimentos[i]->profundidade < -20 + profundidade)
               pos->movimentos[i]->profundidade = -20 + profundidade;
            pos->movimentos[i]->valor += 1000*pos->turno;
         }
         else if(pos->movimentos[i]->jogada[0] == 4){
            if(pos->movimentos[i]->profundidade < -20 + profundidade)
               pos->movimentos[i]->profundidade = -20 + profundidade;
            pos->movimentos[i]->valor += 1500*pos->turno;
         }
      }
      ordena(pos);*/

      for(i = 0; i < pos->nmov; i++){
         /*executa estatico nos movimentos nulos e quiescence nos outros filhos, recursivamente.*/
         if(pos->movimentos[i]->jogada[0] == 4)
            aval_estatico(pos->movimentos[i]);
         else{
            aval_quiescence(pos->movimentos[i], profundidade - 1, beta, existe_beta);
            /*libera os netos*/
            for(j = 0; j < pos->movimentos[i]->nmov; j++)
               libera_posicao(pos->movimentos[i]->movimentos[j]);
            pos->movimentos[i]->nmov = 0;
            pos->movimentos[i]->gerado = 0;
         }

         if(!existe_beta || pos->movimentos[i]->valor*pos->turno > beta*pos->turno){
            beta = pos->movimentos[i]->valor;
            existe_beta = 1;
            if(existe_alfa && beta*pos->turno >= alfa*pos->turno){
               beta += pos->turno;
             //  if(profundidade - 1 < PROF)
            //      quiestable[profundidade-1][pos->movimentos[i]->jogada[1] - 2][pos->movimentos[i]->jogada[2]-2][pos->movimentos[i]->jogada[3]-2][pos->movimentos[i]->jogada[4]-2] = 1;
               break;
            }
         }
      }
      pos->valor = beta;
   }
   pos->profundidade = -10 + profundidade;
}

void aval_estatico(Posicao* pos){
   int total = 0, brancas = 0, pretas = 0, i, j, k, l, chegada[2], cor, linhareib, linhareip;

   if(pos->profundidade >= -10) // nao faz nada caso a posicao ja tenha sido avaliada
      return;

   for( i = 2; i <= 9; i++)
      for( j = 2; j <= 9; j++)
         switch( pos->tabuleiro[i][j] ){
            case 1:  // Peao
            case -1:
               cor = pos->tabuleiro[i][j] ;
               // pontuacao pela existencia da peca
               if(cor == 1)
                  brancas += 100;
               else
                  pretas += 100;

               // pontuacao pelo desenvolvimento da peca
               if( cor == 1 )
                  total += 5*(j - 3)*(j - 3);
               else
                  total -= 5*(8 - j)*(8 - j);

               // pontuacao para peos passados
               if((cor == 1 && j >= 6) || (cor == -1 && j <= 5)){
                  for( k = i-1, l = 9; k <= i+1 && (l == 9 || l == 2); k++)
                     for(l = j + cor; (cor == 1 && l <= 8) || (cor == -1 && l >= 3) ; l += cor )
                        if(pos->tabuleiro[k][l]*cor == -1)
                           break;

                  if( l == 9 || l == 2)
                     total += 80*cor;
               }

               // penalizacao para peoes dobrados
               if( pos->tabuleiro[i][j - cor ] * cor == 1 )
                  total -= 30 * cor;
               else if ( pos->tabuleiro[ i ][ j - 2*cor ] * cor == 1 )
                  total -= 30 * cor;

               // pontuacao para o peao que estiver protegendo outra peca
               if( pos->tabuleiro[ i + 1 ][ j + cor ] * cor > 0 && pos->tabuleiro[ i + 1 ][ j + cor ] != 10)
                  total += 15 * cor;
               else if( pos->tabuleiro[ i - 1 ][ j + cor ] * cor > 0 && pos->tabuleiro[ i - 1 ][ j + cor ] != 10)
                  total += 15 * cor;

               break;

            case 2: // Torre
            case -2:
               cor = pos->tabuleiro[i][j] / 2;
               // pontuacao pela existencia da peca
               if( cor == 1 )
                  brancas += 500;
               else
                  pretas += 500;

               // pontuacao para torres ligadas
               for( k= 1; pos->tabuleiro[ i + k ][ j ] == 10; k++ );
               if( pos->tabuleiro[ i + k ][ j ] * cor == 2 )
                  total += 35*cor;
               for( k = 1; pos->tabuleiro[ i ][ j + k ] == 10; k++ );
               if( pos->tabuleiro[ i ][ j + k ] * cor == 2 )
                  total += 40*cor;

               // pontuacao para torres em colunas livres (sem peoes) ou parcialmente livres (peao do oponente)
               for( k = 1; pos->tabuleiro[ i ][ j + k*cor ] != 0 && abs( pos->tabuleiro[ i ][ j + k*cor ] ) != 1 ; k++ );
               if( pos->tabuleiro[ i ][ j + k*cor ] == 0 )
                  total += 30*cor;
               else if ( pos->tabuleiro[ i ][ j + k*cor ] * cor < 0 )
                  total += 15*cor; //AUMENTAR PONTUACAO SE FOR UM PEAO ISOLADO - PONTO FRACO

               break;

            case 3: // Cavalo
            case -3:
               cor = pos->tabuleiro[i][j] / 3;
               // pontuacao pela existencia da peca
               if( cor == 1 )
                  brancas += 300;
               else
                  pretas += 300;

               // penalizacao para cavalos nas colunas a ou h ou b ou g
               if( i == 2 || i == 9 )
                  total -= 30*cor;
               if( i == 3 || i == 8 )
                  total -= 10*cor;

               // pontuacao para cavalos no retangulo B3 B7 G3 G7 / B2 G2 B6 G6
               if( cor == 1 ){
                  if( i >= 3 && i <= 8 && j >= 4 && j <= 8 )
                     total += 15;
               }
               else{
                  if( i >= 3 && i <= 8 && j >= 3 && j <= 7 )
                     total -= 15;
               }

               break;

            case 4: // Bispo
            case -4:
               cor= pos->tabuleiro[i][j] / 4;
               // pontuacao pela existencia da peca
               if( cor == 1 )
                  brancas += 315;
               else
                  pretas += 315;

               // pontuacao se estiver em diagonal central
               if( j - i <= 1 && j - i >= -1 )
                  total += 10*cor;
               else if( j + i <= 12 && j + i >= 10 )
                  total += 10*cor;

               break;

            case 5: // Rainha
            case -5:
               cor= pos->tabuleiro[i][j] / 5;
               // pontuacao pela existencia da peca
               if( cor == 1 )
                  brancas += 950;
               else
                  pretas += 950;

               // desconto relativo a ataques e cravos
              // chegada[0] = i;
             //  chegada[1] = j;
               //total -= 15 * sob_ataque3(pos, chegada) * cor;

               break;

            case 6: // Rei
            case -6:
               cor = pos->tabuleiro[i][j] / 6;
               // pontuacao para rei rocado com peoes protegendo
               if( cor == 1 && i == 8 && j == 2 ){
                  for(k = 0; k < 3; k++)
                     if( pos->tabuleiro[7 + k][3] == 1 )
                        total += 60;
               }
               if( cor == 1 && i == 4 && j == 2 ){
                  for(k = 0; k < 3; k++)
                     if( pos->tabuleiro[3 + k][3] == 1 )
                        total += 50;
               }
               if( cor == -1 && i == 8 && j == 9 ){
                  for(k = 0; k < 3; k++)
                     if( pos->tabuleiro[7 + k][8] == -1 )
                        total -= 60;
               }
               if( cor == -1 && i == 4 && j == 9 ){
                  for(k = 0; k < 3; k++)
                     if( pos->tabuleiro[3+ k][9] == -1 )
                        total -= 50;
               }

               if( cor == 1 )
                  linhareib = j;
               else
                  linhareip = j;

               // desconto relativo a ataques e cravos
             //  chegada[0] = i;
            //   chegada[1] = j;
               //total -= 20 * sob_ataque3(pos, chegada) * cor;

               break;
         }

   // pontuacao pelo material
   total += 3*( brancas - pretas )/2;
   // pontuacao pela proporcao de material
   if( pretas + brancas != 0)
    total += (brancas - pretas) * 800 / (brancas + pretas) ;

   //penalizacao para rei fora da ultima linha no inicio e meio de jogo e penalizacao no fim
   if( linhareib != 2 && pretas > 1100 )
      total -= 25;
   else
      total += (linhareib - 2) * 15;
   if ( linhareip != 9 && brancas > 1100 )
      total += 25;
   else
      total -= (9 - linhareip) * 15;

   //controle de centro
//   for(k = 0; k < 4; k++){
  //    chegada[0] = 5 + k/2;
    //  chegada[1] = 5 + k%2;
      //total += 10*sob_ataque2(pos, chegada);
  // }

   pos->valor = total;
   pos->profundidade = -10;
}

/*void aval_estatico(Posicao* pos){
   cont4 = clock();
   cont11++;
   int total = 0, i, j;
   if(pos->profundidade >= -10) // nao faz nada caso a posicao ja tenha sido avaliada
      return;
   // por enquanto, aval_estatico apenas conta o material no tabuleiro, sem consideracoes de estrategia

   for( i = 2; i <= 9; i++)
      for( j = 2; j <= 9; j++)
         switch( pos->tabuleiro[i][j] ){
            case 1:
               total += 100;
               break;
            case 2:
               total += 500;
               break;
            case 3:
               total += 300;
               break;
            case 4:
               total += 300;
               break;
            case 5:
               total += 900;
               break;
            case -1:
               total -= 100;
               break;
            case -2:
               total -= 500;
               break;
            case -3:
               total -= 300;
               break;
            case -4:
               total -= 300;
               break;
            case -5:
               total -= 900;
               break;
      }

   pos->valor = total;
   pos->profundidade = -10;
   cont5 += clock() - cont4;
}
*/

int compara(Posicao* pos1, Posicao* pos2){
   int i, j;

   for( i = 2; i <= 9; i++ )
      for( j = 2; j <= 9; j++ )
         if( pos1->tabuleiro[i][j] != pos2->tabuleiro[i][j])
            return 0;

   return 1;
}

int sob_ataque2(Posicao* pos, int casa[]){
   int ataques = 0, k, chegada[2], i, j, cor;
   int v[2][8] = {{2, 1, -1, -2, -2, -1, 1, 2},{ 1, 2, 2, 1, -1, -2, -2, -1}};

   for(i = -1; i <= 1; i++)
      for(j = -1; j <= 1; j++){ // 8 direcoes
         if(i == 0 && j == 0)
            continue;
         k = 0;
         do{
            k++;
            chegada[0] = casa[0] + i*k;
            chegada[1] = casa[1] + j*k;
         } while(pos->tabuleiro[chegada[0]][chegada[1]] == 10); // ate encontrar alguma peca ou a borda do tabuleiro

         if( pos->tabuleiro[chegada[0]][chegada[1]] > 0 )
            cor = 1;
         else if( pos->tabuleiro[chegada[0]][chegada[1]] < 0 )
            cor = -1;
         else
            continue;

         if( i == 0 || j == 0 ){
            if(pos->tabuleiro[chegada[0]][chegada[1]] *cor == 2 || pos->tabuleiro[chegada[0]][chegada[1]] * cor == 5) // torre, rainha
               ataques += cor;
         }
         else{
            if( pos->tabuleiro[chegada[0]][chegada[1]] * cor == 4 || pos->tabuleiro[chegada[0]][chegada[1]] * cor == 5) // bispo, rainha
               ataques += cor;
         }

         if(k == 1 && pos->tabuleiro[chegada[0]][chegada[1]] *cor == 6) // rei
            ataques += cor;
         if(k == 1 &&  i != 0 && j == -cor && pos->tabuleiro[chegada[0]][chegada[1]] * cor == 1) //peao
            ataques += cor;
      }

   for(k = 0; k < 8; k++){
      chegada[0] = casa[0] + v[0][k];
      chegada[1] = casa[1] + v[1][k];

      if( pos->tabuleiro[chegada[0]][chegada[1]]  == 3)
         ataques++;
      else if( pos->tabuleiro[chegada[0]][chegada[1]]  == -3)
         ataques--;
   }

   return ataques;
}

int sob_ataque3(Posicao* pos, int casa[]){
   int ataques = 0, k, chegada[2], i, j, cor, aux;
   int v[2][8] = {{2, 1, -1, -2, -2, -1, 1, 2},{ 1, 2, 2, 1, -1, -2, -2, -1}};

   cor = pos->tabuleiro[casa[0]][casa[1]] / abs(pos->tabuleiro[casa[0]][casa[1]]);

   for(i = -1; i <= 1; i++)
      for(j = -1; j <= 1; j++){ // 8 direcoes
         if(i == 0 && j == 0)
            continue;
         k = 0;
         aux = 2;
         do{ // procura ataques diretos
            k++;
            chegada[0] = casa[0] + i*k;
            chegada[1] = casa[1] + j*k;
         } while(pos->tabuleiro[chegada[0]][chegada[1]] == 10); // ate encontrar alguma peca ou a borda do tabuleiro

         if( pos->tabuleiro[chegada[0]][chegada[1]] * cor > 1 ){ // procura cravos que não sejam em peoes
            aux = 1;
            do{
               k++;
               chegada[0] = casa[0] + i*k;
               chegada[1] = casa[1] + j*k;
            } while(pos->tabuleiro[chegada[0]][chegada[1]] == 10);
         }

         if( pos->tabuleiro[chegada[0]][chegada[1]] * cor >= 0 )
            continue;

         if( i == 0 || j == 0 ){
            if(pos->tabuleiro[chegada[0]][chegada[1]] *cor == -2 || pos->tabuleiro[chegada[0]][chegada[1]] * cor == -5) // torre, rainha
               ataques += aux;
         }
         else{
            if( pos->tabuleiro[chegada[0]][chegada[1]] * cor == -4 || pos->tabuleiro[chegada[0]][chegada[1]] * cor == -5) // bispo, rainha
               ataques += aux;
         }

         if(k == 1 &&  i != 0 && j == -cor && pos->tabuleiro[chegada[0]][chegada[1]] * cor == -1) //peao
            ataques += aux;
      }

   for(k = 0; k < 8; k++){
      chegada[0] = casa[0] + v[0][k];
      chegada[1] = casa[1] + v[1][k];

      if( pos->tabuleiro[chegada[0]][chegada[1]] * cor  == -3)
         ataques += 2;
   }

   return ataques;
}


void insertion (Posicao* v[], int p, int r) {
  int i, j;
  Posicao* chave;

  for(j = p+1; j <= r; j++){
     chave = v[j];
     for(i = j-1; i >= 0 && v[i]->valor*v[p]->turno > chave->valor*v[p]->turno; i--)
        v[i+1] = v[i];
     v[i+1] = chave;
  }
}

void zera_table(){
   int i, j, k, l, m;

   for(i = 0; i < PROF; i++)
      for(j = 0; j < 8; j++)
         for(k = 0; k < 8; k++)
            for(l = 0; l < 8; l++)
               for(m = 0; m < 8; m ++){
                  table[i][j][k][l][m] = 0;
               }
}
