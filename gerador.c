#include "gerador.h"
#include <stdlib.h>
#include <stdio.h>

/*************  Funcoes auxiliares para alocacao de memoria **************/
// Essas funcoes gerenciam a alocacao dinamica de posicoes e pecas atraves de um sistema
//    de reciclagem, que evita o uso excessivo das funcoes malloc e free.
Posicao* pilha_global = NULL;
Posicao* aloca_posicao(); // retorna uma posicao com o tabuleiro vazio (inclui 0 na moldura)
void desaloca_posicao(Posicao* pos);

/***********************  Funcoes auxiliares   **************************/
char letra(int n);    // retorna a letra a partir do codigo, ou 0 caso entrada invalida
int codigo(char c); // retorna o codigo da peca, ou 0 caso entrada invalida
Posicao* copia_posicao(Posicao* pos); // copia uma posicao
void move_peca(Posicao* pos, int saida[], int chegada[]); // Funcao auxiliar usada na geracao de movimentos. Movimenta uma peca, com captura, se necessário. Supoe o movimento valido.

void gera_peao_completo(Posicao* pos, int casa[]); // gera movimentos relacionados ao peao em casa[]
void gera_cavalo_completo(Posicao* pos, int casa[]);
int aux_gera_completo(Posicao* pos, int casa[], int chegada[]); // auxiliar, usado na geracao dos movimentos da torre, bispo e rainha.
void gera_torre_completo(Posicao* pos, int casa[]);
void gera_bispo_completo(Posicao* pos, int casa[]);
void gera_rainha_completo(Posicao* pos, int casa[]);
void gera_rei_completo(Posicao* pos, int casa[]);
void gera_roque(Posicao* pos);

void gera_peao_parcial(Posicao* pos, int casa[]); // gera apenas os movimentos de captura relacionados ao peao em casa[]
void gera_cavalo_parcial(Posicao* pos, int casa[]);
int aux_gera_parcial(Posicao* pos, int casa[], int chegada[]);
void gera_torre_parcial(Posicao* pos, int casa[]);
void gera_bispo_parcial(Posicao* pos, int casa[]);
void gera_rainha_parcial(Posicao* pos, int casa[]);
void gera_rei_parcial(Posicao* pos, int casa[]);

/*************** Funcoes principais  ****************/
void gerador_completo(Posicao* pos){
   if (pos->gerado == 2) // nao faz nada, caso os movimentos ja tenham sido gerados
      return;

   int i, j, casa[2];

   for(i = 2; i <= 9; i++)
      for( j = 2; j <= 9; j++){
         switch( pos->tabuleiro[i][j] * pos->turno ){ // para cada peca, chama a funcao apropriada para seu tipo
            case 1:
               casa[0] = i;
               casa[1] = j;
               gera_peao_completo(pos, casa);
               break;
            case 2:
               casa[0] = i;
               casa[1] = j;
               gera_torre_completo(pos, casa);
               break;
            case 3:
               casa[0] = i;
               casa[1] = j;
               gera_cavalo_completo(pos, casa);
               break;
            case 4:
               casa[0] = i;
               casa[1] = j;
               gera_bispo_completo(pos, casa);
               break;
            case 5:
               casa[0] = i;
               casa[1] = j;
               gera_rainha_completo(pos, casa);
               break;
            case 6:
               casa[0] = i;
               casa[1] = j;
               gera_rei_completo(pos, casa);
               break;
         }
      }
   gera_roque(pos);
   elimina_xeques(pos);

   pos->gerado = 2;
}

void gerador_parcial(Posicao* pos){
   if(pos->gerado != 0) // nao faz nada, caso ja tenham sido gerados movimentos
      return;

   //cria movimento nulo
   pos->movimentos[pos->nmov] = copia_posicao(pos);
   pos->movimentos[pos->nmov]->jogada[0] = 4;
   pos->movimentos[pos->nmov]->jogada[1] = 2;
   pos->movimentos[pos->nmov]->jogada[2] = 2;
   pos->movimentos[pos->nmov]->jogada[3] = 2;
   pos->movimentos[pos->nmov]->jogada[4] = 2;
   pos->nmov++;

   int i, j, casa[2];

   for(i = 2; i <= 9; i++)
      for( j = 2; j <= 9; j++){
         switch( pos->tabuleiro[i][j] * pos->turno ){ // para cada peca, chama a funcao apropriada para seu tipo
            case 1:
               casa[0] = i;
               casa[1] = j;
               gera_peao_parcial(pos, casa);
               break;
            case 2:
               casa[0] = i;
               casa[1] = j;
               gera_torre_parcial(pos, casa);
               break;
            case 3:
               casa[0] = i;
               casa[1] = j;
               gera_cavalo_parcial(pos, casa);
               break;
            case 4:
               casa[0] = i;
               casa[1] = j;
               gera_bispo_parcial(pos, casa);
               break;
            case 5:
               casa[0] = i;
               casa[1] = j;
               gera_rainha_parcial(pos, casa);
               break;
            case 6:
               casa[0] = i;
               casa[1] = j;
               gera_rei_parcial(pos, casa);
               break;
         }
      }
   pos->gerado = 1;
}

void elimina_xeques(Posicao* pos){
   int i, j, k, casa[2];

   for( k = 0; k < pos->nmov; k++){
      // procura rei
      for( i = 2, j = 10; i <= 9 && j == 10 ; i++)
         for( j = 2; j <= 9; j++)
            if( pos->movimentos[k]->tabuleiro[i][j] * pos->turno == 6)
               break;

      if ( j == 10 ){ // rei nao encontrado
         fprintf(stderr, "[elimina_xeques] rei nao encontrado\n");
         exit(-1);
      }

      casa[0] = i-1;
      casa[1] = j;
      // se rei esta em xeque, elimina posicao
      pos->movimentos[k]->turno *= -1;
      if( sob_ataque(pos->movimentos[k], casa) ){
         libera_posicao( pos->movimentos[k] );
         pos->nmov--;
         for(i = k; i < pos->nmov; i++)
            pos->movimentos[i] = pos->movimentos[i+1];
         k--;
      }
      else
         pos->movimentos[k]->turno *= -1;
   }

   if( pos->nmov == 0){
      // procura rei
      for( i = 2, j = 10; i <= 9 && j == 10 ; i++)
         for( j = 2; j <= 9; j++)
            if( pos->tabuleiro[i][j] * pos->turno == 6)
               break;

      casa[0] = i-1;
      casa[1] = j;
      if( sob_ataque(pos, casa) )
         pos->valor = -10000*pos->turno;
      else
         pos->valor = 0;

      pos->profundidade = 20;
   }
}

void libera_posicao(Posicao* pos){
   int i;

   for(i = 0; i < pos->nmov; i++)
      libera_posicao(pos->movimentos[i]);
   desaloca_posicao(pos);
}

void libera_pilha(){
   Posicao* aux;

   while(pilha_global != NULL){
      aux = pilha_global->pai;
      free(pilha_global);
      pilha_global = aux;
   }
}

int sob_ataque(Posicao* pos, int casa[]){
   int ataques = 0, k, chegada[2], i, j;
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
         } while(pos->tabuleiro[chegada[0]][chegada[1]] == 10);

         if( pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno < 0){ //se a peca encontrada for do oponente
            if( i == 0 || j == 0 ){
               if( pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -2 || pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -5) // torre ou rainha
                  ataques++;
            } else{
               if( pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -4 || pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -5) // bispo ou rainha
                  ataques++;
            }

            if(k == 1 && pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -6 ) // rei
               ataques++;
            if(k == 1 &&  i != 0 && j == pos->turno && pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -1 ) // peao
               ataques++;
         }
      }

   for(k = 0; k < 8; k++){
      chegada[0] = casa[0] + v[0][k];
      chegada[1] = casa[1] + v[1][k];

      if( pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -3 ) // cavalo
         ataques++;
   }

   return ataques;
}

/***************** Funcoes Auxiliares ************************/

char letra(int n){
   switch(n){
      case 1:
         return 'P';
      case 2:
         return 'R';
      case 3:
         return 'N';
      case 4:
         return 'B';
      case 5:
         return 'Q';
      case 6:
         return 'K';
      case -1:
         return 'p';
      case -2:
         return 'r';
      case -3:
         return 'n';
      case -4:
         return 'b';
      case -5:
         return 'q';
      case -6:
         return 'k';
      case 10:
         return '-';
      default:
         return 0;
   }
}

int codigo(char c){
   switch(c){
      case 'P':
         return 1;
      case 'R':
         return 2;
      case 'N':
         return 3;
      case 'B':
         return 4;
      case 'Q':
         return 5;
      case 'K':
         return 6;
      case 'p':
         return -1;
      case 'r':
         return -2;
      case 'n':
         return -3;
      case 'b':
         return -4;
      case 'q':
         return -5;
      case 'k':
         return -6;
      case '-':
         return 10;
      default :
         return 0;
   }
}

Posicao* aloca_posicao(){
   Posicao* aux;
   int i, j;

   if(pilha_global == NULL){ // aloca posicao apenas se a pilha estiver vazia
      aux = (Posicao*) malloc(sizeof(Posicao));

      // inclui 0 na moldura
      for(i = 0; i < 2; i++)
         for(j = 0; j < 12; j++)
            aux->tabuleiro[i][j] = 0;

      for(i = 10; i < 12; i++)
         for(j = 0; j < 12; j++)
            aux->tabuleiro[i][j] = 0;

      for(i = 2; i < 10; i++)
         for(j = 0; j < 2; j++)
            aux->tabuleiro[i][j] = 0;

      for(i = 2; i < 10; i++)
         for(j = 10; j < 12; j++)
            aux->tabuleiro[i][j] = 0;

      return aux;
   }

   aux = pilha_global; //caso existam estruturas disponiveis, apenas remove do topo da pilha
   pilha_global = aux->pai;

   return aux;
}

void desaloca_posicao(Posicao* pos){
   pos->pai = pilha_global;
   pilha_global = pos; // coloca pos no topo da pilha
}

Posicao* copia_posicao(Posicao* pos){
   // nao define jogada
   int i, j;
   Posicao* nova = aloca_posicao();

   for(i = 2; i <= 9; i++)
      for( j = 2; j <= 9; j++)
         nova->tabuleiro[i][j] = pos->tabuleiro[i][j];

   nova->turno = -pos->turno;
   for(i=0; i<4; i++)
      nova->roque[i] = pos->roque[i];
   nova->enpassant[0] = 0;
   nova->enpassant[1] = 0;
   nova->nmov = 0;
   nova->gerado = 0;
   nova->pai = pos;
   nova->profundidade = -20;
   nova->valor = 0;

   return nova;
}

void move_peca(Posicao* pos, int saida[], int chegada[]){
   pos->movimentos[pos->nmov] = copia_posicao(pos);
   if(pos->movimentos[pos->nmov]->tabuleiro[chegada[0]][chegada[1]] == 10) // nao eh uma captura
      pos->movimentos[pos->nmov]->jogada[0] = 0;
   else{ /* remove peca */
      pos->movimentos[pos->nmov]->jogada[0] = 1;
      if( pos->movimentos[pos->nmov]->tabuleiro[chegada[0]][chegada[1]] * pos->turno == -6){ //rei capturado => MATE
         pos->movimentos[pos->nmov]->valor = 10000*pos->turno;
         pos->movimentos[pos->nmov]->profundidade = 20;
      }
   }

   pos->movimentos[pos->nmov]->tabuleiro[chegada[0]][chegada[1]] = pos->movimentos[pos->nmov]->tabuleiro[saida[0]][saida[1]]; // coloca em chegada[] a peca de saida[]
   pos->movimentos[pos->nmov]->tabuleiro[saida[0]][saida[1]] = 10; // remove a peca de saida[]
   pos->movimentos[pos->nmov]->jogada[1] = saida[0];
   pos->movimentos[pos->nmov]->jogada[2] = saida[1];
   pos->movimentos[pos->nmov]->jogada[3] = chegada[0];
   pos->movimentos[pos->nmov]->jogada[4] = chegada[1];
   pos->nmov++;
}

void gera_peao_completo(Posicao* pos, int casa[]){
   int chegada[2];

   if(pos->tabuleiro[casa[0]][casa[1]+pos->turno] == 10){ // move uma casa para frente
      chegada[0] = casa[0];
      chegada[1] = casa[1]+pos->turno;
      if(pos->gerado == 0 || !( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) ) )
         move_peca(pos, casa, chegada);
      if(pos->gerado == 0 && ( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) ) )
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] =  5*pos->turno ; // promove o peao para rainha
   }

   if(pos->tabuleiro[casa[0]][casa[1]+2*pos->turno] == 10 && (pos->turno*casa[1]==3 || pos->turno*casa[1]==-8) && pos->tabuleiro[casa[0]][casa[1]+pos->turno] == 10){ // move duas casas para frente, no inicio
      chegada[0] = casa[0];
      chegada[1] = casa[1]+2*pos->turno;
      move_peca(pos, casa, chegada);
      pos->movimentos[pos->nmov-1]->enpassant[0] = casa[0];
      pos->movimentos[pos->nmov-1]->enpassant[1] = casa[1]+pos->turno;
   }

   if(pos->gerado == 0){
      if( pos->tabuleiro[casa[0]+1][casa[1]+pos->turno] * pos->turno < 0 && pos->tabuleiro[casa[0]+1][casa[1]+pos->turno] != 10){ //captura para direita
         chegada[0] = casa[0]+1;
         chegada[1] = casa[1]+ pos->turno;
         move_peca(pos, casa, chegada);
         if( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) )
            pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] =  5*pos->turno; // promove o peao para rainha
      }
      if( pos->tabuleiro[casa[0]-1][casa[1]+pos->turno] * pos->turno < 0 && pos->tabuleiro[casa[0]-1][casa[1]+pos->turno] != 10){ // captura para esquerda
         chegada[0] = casa[0]-1;
         chegada[1] = casa[1]+ pos->turno;
         move_peca(pos, casa, chegada);
         if( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) )
            pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 5*pos->turno ; // promove o peao para rainha
      }
      if(pos->enpassant[0] == casa[0]+1 && pos->enpassant[1] == casa[1]+pos->turno){ // capturas enpassant
         chegada[0] = casa[0]+1;
         chegada[1] = casa[1];
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]+pos->turno] = pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]];
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 10;
         pos->movimentos[pos->nmov-1]->jogada[4] = chegada[1]+pos->turno;
      }
      if(pos->enpassant[0] == casa[0]-1 && pos->enpassant[1] == casa[1]+pos->turno){
         chegada[0] = casa[0]-1;
         chegada[1] = casa[1];
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]+pos->turno] = pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]];
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 10;
         pos->movimentos[pos->nmov-1]->jogada[4] = chegada[1]+pos->turno;
      }
   }
}

void gera_cavalo_completo(Posicao* pos, int casa[]){
   int v[2][8] = {{2, 1, -1, -2, -2, -1, 1, 2},{ 1, 2, 2, 1, -1, -2, -2, -1}}; // vetor de movimentos do cavalo
   int k, chegada[2];

   for(k = 0; k < 8; k++){
      if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] == 10 ){ // movimento normal
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
      }
      else if( pos->gerado == 0 && pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] * pos->turno < 0 ){ // captura
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
      }
   }
}

int aux_gera_completo(Posicao* pos, int casa[], int chegada[]){ // retorna 1 quando a peca nao pode continuar se movimentando em certa direcao
   if(pos->tabuleiro[chegada[0]][chegada[1]] != 10 &&  pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno >= 0) // encontrou borda do tabuleiro ou peca aliada
      return 1;

   if(pos->tabuleiro[chegada[0]][chegada[1]] == 10 || pos->gerado == 0){ //encontrou casa vazia ou peca do oponente
      move_peca(pos, casa, chegada);

      // verifica roque
      if(pos->turno == 1){
         if(pos->movimentos[pos->nmov-1]->roque[0] == 1)
            if(casa[0] == 9 && casa[1] == 2)
               pos->movimentos[pos->nmov-1]->roque[0] = 0;
         if(pos->movimentos[pos->nmov-1]->roque[1] == 1)
            if(casa[0] == 2 && casa[1] == 2)
               pos->movimentos[pos->nmov-1]->roque[1] = 0;
      }
      else{
         if(pos->movimentos[pos->nmov-1]->roque[2] == 1)
            if(casa[0] == 9 && casa[1] == 9)
               pos->movimentos[pos->nmov-1]->roque[2] = 0;
         if(pos->movimentos[pos->nmov-1]->roque[3] == 1)
            if(casa[0] == 2 && casa[1] == 9)
               pos->movimentos[pos->nmov-1]->roque[3] = 0;
      }
   }

   if( pos->tabuleiro[chegada[0]][chegada[1]] != 10 ) // se a jogada foi uma captura, para de se mover na direcao
      return 1;

   return 0;
}

void gera_torre_completo(Posicao* pos, int casa[]){
   int i, chegada[2];

   i = 1;
   while(1){ // movimenta-se para a direita ate funcao auxiliar retornar 1
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1];
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){ //esquerda
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1];
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){ //frente
      chegada[0] = casa[0];
      chegada[1] = casa[1]+i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){ // tras
      chegada[0] = casa[0];
      chegada[1] = casa[1]-i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }
}

void gera_bispo_completo(Posicao* pos, int casa[]){
   int i, chegada[2];

   i = 1;
   while(1){
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1]+i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1]+i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1]-i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1]-i;
      if( aux_gera_completo(pos, casa, chegada) )
         break;
      i++;
   }
}

void gera_rainha_completo(Posicao* pos, int casa[]){
   gera_torre_completo(pos, casa);
   gera_bispo_completo(pos, casa);
}

void gera_rei_completo(Posicao* pos, int casa[]){
   int v[2][8] = {{1, 1, 0, -1, -1, -1, 0, 1},{ 0, 1, 1, 1, 0, -1, -1, -1}};
   int k, chegada[2], aux;

   if(pos->turno == 1)
      aux = 0;
   else
      aux = 2;

   for(k = 0; k < 8; k++){
      if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] == 10 ){ // casa vazia
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->roque[aux] = 0;
         pos->movimentos[pos->nmov-1]->roque[aux+1] = 0;
      }
      else if( pos->gerado == 0 && pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] * pos->turno < 0 ){ //captura
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->roque[aux] = 0;
         pos->movimentos[pos->nmov-1]->roque[aux+1] = 0;
      }
   }
}

void gera_roque(Posicao* pos){
   int aux, casa1[2], casa2[2], casa3[2], casa4[2], saida[2];

   if(pos->turno == 1)
      aux = 0;
   else
      aux = 2;

   if(pos->roque[aux] == 1){ // O-O
      if(aux == 0){
         casa1[0] = 7;
         casa1[1] = 2;
         casa2[0] = 8;
         casa2[1] = 2;
         casa3[0] = 6;
         casa3[1] = 2;
      } else{
         casa1[0] = 7;
         casa1[1] = 9;
         casa2[0] = 8;
         casa2[1] = 9;
         casa3[0] = 6;
         casa3[1] = 9;
      }

      if( pos->tabuleiro[casa1[0]][casa1[1]] == 10 && pos->tabuleiro[casa2[0]][casa2[1]] == 10 ){
         if( !sob_ataque(pos, casa1) && !sob_ataque(pos, casa2) && !sob_ataque(pos, casa3) ){
            saida[0] = 6;
            saida[1] = casa1[1];
            move_peca(pos, saida, casa2);
            saida[0] = 9;
            pos->movimentos[pos->nmov-1]->tabuleiro[casa1[0]][casa1[1]] = pos->movimentos[pos->nmov-1]->tabuleiro[saida[0]][saida[1]];
            pos->movimentos[pos->nmov-1]->tabuleiro[saida[0]][saida[1]] = 10;
            pos->movimentos[pos->nmov-1]->roque[aux] = 0;
            pos->movimentos[pos->nmov-1]->roque[aux + 1] = 0;
            pos->movimentos[pos->nmov-1]->jogada[0] = 2;
            pos->movimentos[pos->nmov-1]->jogada[1] = 6;
            pos->movimentos[pos->nmov-1]->jogada[2] = casa1[1];
            pos->movimentos[pos->nmov-1]->jogada[3] = 8;
            pos->movimentos[pos->nmov-1]->jogada[4] = casa1[1];
         }
      }
   }

   if(pos->roque[aux+1] == 1){ //O-O-O
      if(aux == 0){
         casa1[0] = 5;
         casa1[1] = 2;
         casa2[0] = 4;
         casa2[1] = 2;
         casa3[0] = 3;
         casa3[1] = 2;
         casa4[0] = 6;
         casa4[1] = 2;
      } else{
         casa1[0] = 5;
         casa1[1] = 9;
         casa2[0] = 4;
         casa2[1] = 9;
         casa3[0] = 3;
         casa3[1] = 9;
         casa4[0] = 6;
         casa4[1] = 9;
      }

      if( pos->tabuleiro[casa1[0]][casa1[1]] == 10 && pos->tabuleiro[casa2[0]][casa2[1]] == 10 && pos->tabuleiro[casa3[0]][casa3[1]] == 10){
         if( !sob_ataque(pos, casa1) && !sob_ataque(pos, casa2) && !sob_ataque(pos, casa4) ){
            saida[0] = 6;
            saida[1] = casa1[1];
            move_peca(pos, saida, casa2);
            saida[0] = 2;
            pos->movimentos[pos->nmov-1]->tabuleiro[casa1[0]][casa1[1]] = pos->movimentos[pos->nmov-1]->tabuleiro[saida[0]][saida[1]];
            pos->movimentos[pos->nmov-1]->tabuleiro[saida[0]][saida[1]] = 10;
            pos->movimentos[pos->nmov-1]->roque[aux+1] = 0;
            pos->movimentos[pos->nmov-1]->roque[aux] = 0;
            pos->movimentos[pos->nmov-1]->jogada[0] = 3;
            pos->movimentos[pos->nmov-1]->jogada[1] = 6;
            pos->movimentos[pos->nmov-1]->jogada[2] = casa1[1];
            pos->movimentos[pos->nmov-1]->jogada[3] = 4;
            pos->movimentos[pos->nmov-1]->jogada[4] = casa1[1];
         }
      }
   }
}

void gera_peao_parcial(Posicao* pos, int casa[]){
   int chegada[2];

   if(pos->tabuleiro[casa[0]][casa[1]+pos->turno] == 10){
      chegada[0] = casa[0];
      chegada[1] = casa[1]+pos->turno;
      if( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) ){
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 5 * pos->turno;
      }
   }

   if( pos->tabuleiro[casa[0]+1][casa[1]+pos->turno] * pos->turno < 0 && pos->tabuleiro[casa[0]+1][casa[1]+pos->turno] != 10){
      chegada[0] = casa[0]+1;
      chegada[1] = casa[1]+ pos->turno;
      move_peca(pos, casa, chegada);
      if( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) )
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 5 * pos->turno ;
   }
   if( pos->tabuleiro[casa[0]-1][casa[1]+pos->turno] * pos->turno < 0 && pos->tabuleiro[casa[0]-1][casa[1]+pos->turno] != 10){
      chegada[0] = casa[0]-1;
      chegada[1] = casa[1]+ pos->turno;
      move_peca(pos, casa, chegada);
      if( (pos->turno == 1 && chegada[1] == 9) || (pos->turno == -1 && chegada[1] == 2) )
         pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 5 * pos->turno ;
   }
   if(pos->enpassant[0] == casa[0]+1 && pos->enpassant[1] == casa[1]+pos->turno){
      chegada[0] = casa[0]+1;
      chegada[1] = casa[1];
      move_peca(pos, casa, chegada);
      pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]+pos->turno] = pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]];
      pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 10;
      pos->movimentos[pos->nmov-1]->jogada[4] = chegada[1]+pos->turno;
   }
   if(pos->enpassant[0] == casa[0]-1 && pos->enpassant[1] == casa[1]+pos->turno){
      chegada[0] = casa[0]-1;
      chegada[1] = casa[1];
      move_peca(pos, casa, chegada);
      pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]+pos->turno] = pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]];
      pos->movimentos[pos->nmov-1]->tabuleiro[chegada[0]][chegada[1]] = 10;
      pos->movimentos[pos->nmov-1]->jogada[4] = chegada[1]+pos->turno;
   }
}

void gera_cavalo_parcial(Posicao* pos, int casa[]){
   int v[2][8] = {{2, 1, -1, -2, -2, -1, 1, 2},{ 1, 2, 2, 1, -1, -2, -2, -1}};
   int k, chegada[2];

   for(k = 0; k < 8; k++){
      if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] == 10 ){
      }
      else if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] * pos->turno < 0 ){
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
      }
   }
}

int aux_gera_parcial(Posicao* pos, int casa[], int chegada[]){
   if(pos->tabuleiro[chegada[0]][chegada[1]] == 10)
      return 0;

   if( pos->tabuleiro[chegada[0]][chegada[1]] * pos->turno >= 0 )
      return 1;

   move_peca(pos, casa, chegada);

   /*Verifica roque*/
   if(pos->turno == 1){
      if(pos->movimentos[pos->nmov-1]->roque[0] == 1)
         if(casa[0] == 9 && casa[1] == 2)
            pos->movimentos[pos->nmov-1]->roque[0] = 0;
      if(pos->movimentos[pos->nmov-1]->roque[1] == 1)
         if(casa[0] == 2 && casa[1] == 2)
            pos->movimentos[pos->nmov-1]->roque[1] = 0;
   }
   else{
      if(pos->movimentos[pos->nmov-1]->roque[2] == 1)
         if(casa[0] == 9 && casa[1] == 9)
            pos->movimentos[pos->nmov-1]->roque[2] = 0;
      if(pos->movimentos[pos->nmov-1]->roque[3] == 1)
         if(casa[0] == 2 && casa[1] == 9)
            pos->movimentos[pos->nmov-1]->roque[3] = 0;
   }

   return 1;
}

void gera_torre_parcial(Posicao* pos, int casa[]){
   int i, chegada[2];

   i = 1;
   while(1){
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1];
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1];
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0];
      chegada[1] = casa[1]+i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0];
      chegada[1] = casa[1]-i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }
}

void gera_bispo_parcial(Posicao* pos, int casa[]){
   int i, chegada[2];

   i = 1;
   while(1){
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1]+i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1]+i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]+i;
      chegada[1] = casa[1]-i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }

   i = 1;
   while(1){
      chegada[0] = casa[0]-i;
      chegada[1] = casa[1]-i;
      if( aux_gera_parcial(pos, casa, chegada) )
         break;
      i++;
   }
}

void gera_rainha_parcial(Posicao* pos, int casa[]){
   gera_torre_parcial(pos, casa);
   gera_bispo_parcial(pos, casa);
}

void gera_rei_parcial(Posicao* pos, int casa[]){
   int v[2][8] = {{1, 1, 0, -1, -1, -1, 0, 1},{ 0, 1, 1, 1, 0, -1, -1, -1}};
   int k, chegada[2], aux;

   if(pos->turno == 1)
      aux = 0;
   else
      aux = 2;

   for(k = 0; k < 8; k++){
      if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] == 10 ){
      }
      else if( pos->tabuleiro[casa[0]+v[0][k]][casa[1]+v[1][k]] * pos->turno < 0 ){
         chegada[0] = casa[0]+v[0][k];
         chegada[1] = casa[1]+v[1][k];
         move_peca(pos, casa, chegada);
         pos->movimentos[pos->nmov-1]->roque[aux] = 0;
         pos->movimentos[pos->nmov-1]->roque[aux+1] = 0;
      }
   }
}
