# XL-713

This is a chess game that I wrote in C as a personal project back in 2012.

It uses a minimax algorithm with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning) with overall depth 6 and plays at a beginner's strength (~1200).

To compile, run 

```
gcc xl713.c gerador.c avaliador.c -o xl713
```

It has a terminal interface. To play, choose B for white or P for black (the interface is in portuguese) and then use [algebraic notation](https://en.wikipedia.org/wiki/Algebraic_notation_(chess)) to write your moves.
