/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"
#include "util.h"
#include "watek_glowny.c"
#include "watek_komunikacyjny.c"
#include "util.c"

// Zmienne wątku (komunikacyjne)
int ackCount = 0;
int clockLamporta = 1;
int lastRequestTS = 0;

int last = 0;
int ackQueue[ackQueue_SIZE];

// Zmienne pakietu
int rank;
int size;
int room = -1;
int game = 0;

// wątek komunikacyjny
pthread_t threadKom;

// Kolejka oczekujących graczy pokoju
packet_t rooms[ROOMS][4];

// Kolejka graczy pokoju
int roomsQueue[ROOMS];

void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
}

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
            fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
            MPI_Finalize();
            exit(-1);
            break;
        case MPI_THREAD_FUNNELED:
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	        break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	        break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
	        break;
        default: printf("Nikt nic nie wie\n");
    }
}


int main(int argc, char **argv)
{

    //Inicjalizuj (ackQueue)
    for (int i = 0; i < ackQueue_SIZE; i++) ackQueue[i] = -1;
   
    //Inicjalizuj (roomQueue)
//    bzero(roomsQueue[i], ROOMS);
    for (int i = 0; i < ROOMS; i++) roomsQueue[i] = 0;

    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);
    srand(rank);
    /* zob. util.c oraz util.h */
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    /* startKomWatek w watek_komunikacyjny.c 
     * w vi najedź kursorem na nazwę pliku i wciśnij klawisze gf
     * powrót po wciśnięciu ctrl+6
     * */

    pthread_create( &threadKom, NULL, startKomWatek , 0);

    /* mainLoop w watek_glowny.c 
     * w vi najedź kursorem na nazwę pliku i wciśnij klawisze gf
     * powrót po wciśnięciu ctrl+6
     * */

    mainLoop(); // możesz także wcisnąć ctrl-] na nazwie funkcji
		// działa, bo używamy ctags (zob Makefile)
		// jak nie działa, wpisz set tags=./tags :)
    
    finalizuj();
    return 0;
}

