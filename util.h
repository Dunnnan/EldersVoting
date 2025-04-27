#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;             /* timestamp   (zegar lamporta */
    int src;            /* source      (id procesu wysyłającego) */
    int game;           /* game        (id gry) */
    int room;           /* source      (id pokoju) */
    int request_id;     /* request_id  (id requesta) */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 5

// kolejka
struct list_element{
    int ts;
    int src;
    int game;
    int room;
    struct list_element *next;
};


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

extern state_t stan;
extern pthread_mutex_t stateMut;


extern pthread_mutex_t ackQueue_mutex;


/* zmiana stanu, obwarowana muteksem */
void changeState( state_t );

// obsługa REQ
void handleRequest(packet_t);

// kolejka
void sortList(struct list_element** queues, int room);
void insertNode(struct list_element** queues, int room, int timestamp, int src, int game);
void removeNode(struct list_element** queues, int room, int source_rank);
void printList(struct list_element* queues, int room);
int isElementInNElements(struct list_element* queues, int room, int src, int x, int game);

#endif
