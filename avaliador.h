#ifndef AVALIADOR_H
#define AVALIADOR_H

Posicao* engine(Posicao* pos); // Funcao que executa a engine, a partir do main
void ordena(Posicao* pos); // Funcao que ordena vetor pos->movimentos, de acordo com os valores de pos->turno e pos->movimentos[i]->valor. Supoe pos->nmov != 0

#endif /* AVALIADOR_H */
