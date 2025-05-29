#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10

// Stałe
#define ROOT 0
#define ROOMS 3             // liczba pokoi
#define GAMES 1             // liczba gier
#define ackQueue_SIZE 10000 // rozmiar kolejki przechowującej id procesów, którym początkowo nie wysłaliśmy ACK ( > size)

// Zmienne wątku (komunikacyjne)
extern int ackCount;
extern int clockLamporta;
extern int lastRequestTS;
extern int last;
extern int ackQueue[ackQueue_SIZE];

// Zmienne pakietu
extern int rank;
extern int size;
extern int room;
extern int game;

// wątek komunikacyjny
extern pthread_t threadKom;

// Kolejka oczekujących graczy pokoju
extern packet_t rooms[ROOMS][4];

// Kolejka graczy pokoju
extern int roomsQueue[ROOMS];

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("clock[%d] %c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n", clockLamporta,  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("clock[%d] %c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n", clockLamporta,  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);


#endif
