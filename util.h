#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;             /* timestamp   (zegar lamporta */
    int src;            /* source      (id procesu wysyłającego) */
    int game;           /* game        (id gry) */
    int room;           /* source      (id pokoju) */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=4. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 4



/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK     1
#define REQUEST 2
#define RELEASE 3
#define APP_PKT 4
#define FINISH  5
#define ADD_QUEUE 6
#define WAITING 7


extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum {
    InRun,      // 0
    InMonitor,  // 1
    InWant,     // 2
    InSection,  // 3
    InFinish,   // 4
    Add_Queue,  // 5
    Waiting     // 6
   } state_t;   // 7


// Zmienne synchronizacyjne
extern state_t stan;
extern pthread_mutex_t stateMut;         // do bezpiecznej  zmiany stanu i zmiennych z nim związanych
extern pthread_mutex_t ackQueue_mutex;   // do bezpiecznego operowania na procesach, którym początkowo nie wysłaliśmy ACK
extern pthread_mutex_t roomsQueue_mutex; // do bezpiecznego operowania na długościach kolejek pokojów
extern sem_t inSection;                  // do bezpiecznego wchodzenia do sekcji krytycznej
extern sem_t enoughACK;                  // do bezpiecznego wchodzenia w stan oczekiwania na innych graczy

// zmiana stanu
void changeState( state_t );

// obsługa REQ
void handleRequest(packet_t);

// Bezpieczne zmiany wartości
void pickHigherClock(packet_t);
void incrementClock();
void incrementACK();
void resetACK();
void resetRoom(packet_t);
void rememberRequestTS();
void incrementQueue(packet_t);
void decrement4Queue(packet_t);
void resetPickedRoom();

// Inne
void vote(packet_t);    // proces głosowania
void resendACK();       // proces rozesłania ACK procesom, którym początkowo go nie wysłaliśmy
int pickRoom();         // proces wyboru najkorzystniejszego pokoju (najszybsza szansa na grę)

#endif
