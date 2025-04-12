#include "main.h"
#include "util.h"
#include <semaphore.h>

MPI_Datatype MPI_PAKIET_T;

/* 
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan=InRun;

/* zamek wokół zmiennej współdzielonej między wątkami. 
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;


struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = {
        { "pakiet aplikacyjny", APP_PKT },
        { "finish", FINISH},
        { "potwierdzenie", ACK},
        {"prośbę o sekcję krytyczną", REQUEST},
        {"czekanie na sekcję krytyczną", RELEASE},
        {"dodaj mnie do kolejki wskazanego pokoju", ADD_QUEUE}
};

const char *const tag2string( int tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == tag )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint     offsets[NITEMS];

    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, game);
    offsets[3] = offsetof(packet_t, room);



    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    int freepkt=0;

    if (pkt==0) {
        pkt = malloc(sizeof(packet_t));
        freepkt=1;
    }

    pkt->src = rank;
    pkt->ts = clockLamporta;
    pkt->room = room;
    pkt->game = game;

    pthread_mutex_lock( &stateMut );
    clockLamporta += 1;
	pthread_mutex_unlock( &stateMut );

    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

void changeState( state_t newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
        pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

/*
void sortList(struct list_element** queues, int room) {
    struct list_element* current;
    struct list_element* next;
    int swapped;

    if (queues[room] == NULL) {
        return;
    }

    do {
        swapped = 0;
        current = queues[room];

        while (current->next != NULL) {
            next = current->next;

            if (current->ts > next->ts) {
                if (current == queues[room]) {
                    queues[room] = next;
                } else {
                    struct list_element* prev = queues[room];
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    prev->next = next;
                }
                current->next = next->next;
                next->next = current;
                swapped = 1;
            } else if (current->ts == next->ts && current->src > next->src) {
                if (current == queues[room]) {
                    queues[room] = next;
                } else {
                    struct list_element* prev = queues[room];
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    prev->next = next;
                }
                current->next = next->next;
                next->next = current;
                swapped = 1;
            }

            current = next;
        }
    } while (swapped);
}


void insertNode(struct list_element** queues, int room, int ts, int src, int game, int room) {
    struct list_element* new_node = malloc(sizeof(struct list_element));
    new_node->src = src;
    new_node->ts = ts;
    new_node->game = game;
    new_node->room = room;
    new_node->next = NULL;

    if (queues[room] == NULL) {
        queues[room] = new_node;
    } else {
        struct list_element* current = queues[room];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void removeNode(struct list_element** queues, int room, int src) {
    if (queues[room] == NULL) {
        return;
    }

    struct list_element* current = queues[room];
    struct list_element* prev = NULL;

    if (current != NULL && current->src == src) {
        queues[room] = current->next;
        free(current);
        return;
    }

    while (current != NULL && current->src != src) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return;
    }

    prev->next = current->next;
    free(current);
}


void printList(struct list_element* queues, int room) {
    debug("---------------");
    debug("Queue in Room %d", room);

    struct list_element* current = queues[room];

    while (current != NULL) {
        debug("Source: %d, ts: %d, type: %s, room: %d", current->src, current->ts, type_array[current->type], current->room);
        current = current->next;
    }
    debug("---------------");
}


int isElementInNElements(struct list_element* queues, int room, int src, int n, int game, int room) {
    struct list_element* current = queues[room];
    int count = 0;

    while (current != NULL && count < n) {
        if (current->room == room && current->game != game) {
            return 0;
        }

        if (current->src == src) {
            return 1;
        }

        current = current->next;
        if (current != NULL && current->room == room) {
            count++;
        }
    }

    return 0;
}
*/