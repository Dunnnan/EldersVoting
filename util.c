#include "main.h"
#include "util.h"
#include <semaphore.h>
#include <limits.h>


MPI_Datatype MPI_PAKIET_T;

// Zmienne synchronizacyjne
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;         // do bezpiecznej  zmiany stanu i zmiennych z nim związanych
pthread_mutex_t ackQueue_mutex = PTHREAD_MUTEX_INITIALIZER;   // do bezpiecznego operowania na procesach, którym początkowo nie wysłaliśmy ACK
pthread_mutex_t roomsQueue_mutex = PTHREAD_MUTEX_INITIALIZER; // do bezpiecznego operowania na długościach kolejek pokojów
sem_t inSection;                                              // do bezpiecznego wchodzenia do sekcji

state_t stan=InRun;


struct tagNames_t{
    const char *name;
    int tag;
} tagNames[] = {
        { "pakiet aplikacyjny", APP_PKT },
        { "finish", FINISH},
        { "potwierdzenie", ACK},
        {"prośbę o sekcję krytyczną", REQUEST},
        {"dodaj mnie do kolejki wskazanego pokoju", ADD_QUEUE},
        {"czekam na kolegów, bo zebrałem wystarczająco ack", WAITING},
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

    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string( tag), destination);
    if (freepkt) free(pkt);
}

// Zmień stan
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

// Nadpisz clock w przypadku otrzymania wiadomości o większym .ts
void pickHigherClock( packet_t pakiet) {
    pthread_mutex_lock( &stateMut );
    if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts+1;
    else clockLamporta++;
    pthread_mutex_unlock( &stateMut );
}

// Inkrementuj ackCount
void incrementACK() {
    pthread_mutex_lock(&stateMut);
    ackCount++;
    pthread_mutex_unlock(&stateMut);
}

// Inkrementuj Clock
void incrementClock() {
    pthread_mutex_lock( &stateMut );
    clockLamporta += 1;
    pthread_mutex_unlock( &stateMut );
}

// Wyzeruj ackCount
void resetACK() {
    pthread_mutex_lock(&stateMut);
    ackCount = 0;
    pthread_mutex_unlock(&stateMut);
}

// Wyczyść kolejkę pokoju
void resetRoom( packet_t pakiet ) {
    memset(&rooms[pakiet.room][0], 0, sizeof(packet_t));
    memset(&rooms[pakiet.room][1], 0, sizeof(packet_t));
    memset(&rooms[pakiet.room][2], 0, sizeof(packet_t));
    memset(&rooms[pakiet.room][3], 0, sizeof(packet_t));
}

// Zapamiętaj moment rozpoczęcia ubiegania się
void rememberRequestTS() {
    pthread_mutex_lock(&stateMut);
    lastRequestTS = clockLamporta;
    pthread_mutex_unlock(&stateMut);
}

// Zwiększ długość kolejki pokoju
void incrementQueue(packet_t pakiet) {
    pthread_mutex_lock(&roomsQueue_mutex);
    roomsQueue[pakiet.room]++;
    pthread_mutex_unlock(&roomsQueue_mutex);
}

// Zmniejsz długość kolejki pokoju (-4 - grupa weszła do sekcji)
void decrement4Queue(packet_t pakiet) {
    pthread_mutex_lock(&roomsQueue_mutex);
    roomsQueue[pakiet.room] -= 4;
    pthread_mutex_unlock(&roomsQueue_mutex);
}

// Zresetuj id pokoju wybranego do gry
void resetPickedRoom() {
    pthread_mutex_lock(&stateMut);
    room = -1;
    pthread_mutex_unlock(&stateMut);
}

// Zlicz głosy z kolejki i napisz w co gramy
void vote( packet_t pakiet ) {
    int oldestGame = -1;
    int gameOne = 0;
    int gameTwo = 0;
    int gameThree = 0;

    for (int i = 0; i < 4; i++) {
        if (rooms[pakiet.room][i].game == 0) gameOne++;
        if (rooms[pakiet.room][i].game == 1) gameTwo++;
        if (rooms[pakiet.room][i].game == 2) gameThree++;
        if (rooms[pakiet.room][i].src > oldestGame) oldestGame = rooms[pakiet.room][i].src;
    }

    if (gameOne >= 2 && gameTwo < 2 && gameThree < 2) {
        println("Rżniemy w karty: Rozbierany poker %d Room: %d", ackCount, room);
    }
    else if (gameTwo >= 2 && gameOne < 2 && gameThree < 2) {
        println("Rżniemy w karty: Brydż %d Room: %d", ackCount, room);
    }
    else if (gameThree >= 2 && gameOne < 2 && gameTwo < 2) {
        println("Rżniemy w karty: Wist %d Room: %d", ackCount, room);
    }
    else {
        println("Rżniemy w cokolwiek podyktuje młodziak : %d %d %d %d %d", rooms[pakiet.room][0].room, rooms[pakiet.room][1].game, rooms[pakiet.room][2].game, rooms[pakiet.room][3].game, ackCount);
    }
}

// Roześlij ACK procesom zapisanym na liście
void resendACK() {
    pthread_mutex_lock(&ackQueue_mutex);

    // Roześlij ACK procesom zapisanym na liście
	packet_t *pkt = malloc(sizeof(packet_t));
    for (int i = 0; i < ackQueue_SIZE; i++) {
        if (ackQueue[i] == -1) {
            break;
        }
        else if (ackQueue[i] == -10) {
			ackQueue[i] = -1;
            continue;
        }

		sendPacket( pkt, ackQueue[i], ACK);
		ackQueue[i] = -1;
    }
    // Zresetuj wskaźnik ostatniego elementu
    last = 0;

    pthread_mutex_unlock(&ackQueue_mutex);
}

// Wybierz najkorzystniejszy pokój
int pickRoom() {
    pthread_mutex_lock(&roomsQueue_mutex);

    int min = INT_MAX;
    int minRest = -1;
    int room = 0;

    for (int i = 0; i < ROOMS; i++) {
        if (roomsQueue[i] == 3) {
            room = i;
            break;
        }
        if (roomsQueue[i] < min) {
            room = i;
            min = roomsQueue[i];
            minRest = roomsQueue[i] % 4;
        }
        else if (roomsQueue[i] == min && (roomsQueue[i] % 4) > minRest) {
            room = i;
            minRest = roomsQueue[i] % 4;
        }
    }

    roomsQueue[room]++;

    pthread_mutex_unlock(&roomsQueue_mutex);

    return room;
}