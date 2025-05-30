#include "main.h"
#include "watek_komunikacyjny.h"


/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;

    // Czy dodano gracza do kolejki pokoju
    bool isAdded = false;

    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch ( status.MPI_TAG ) {
	    case REQUEST: 

            // Zwiększ długość kolejki oczekujących graczy pokoju
            incrementQueue(pakiet);

            pthread_mutex_lock(&ackQueue_mutex);
            pthread_mutex_lock(&stateMut);

            if
            (
                pakiet.room != room ||
                (
                    stan == InRun ||
                    (
                        (pakiet.ts < lastRequestTS) ||                       // != ts (porównanie ts)
                        (pakiet.ts == lastRequestTS && pakiet.src < rank)    // == ts (porównanie id)
                    )
                    &&
                    (
                        stan != Waiting &&
                        stan != InSection
                    )
                )
            ) {

		        // Podmień .ts
		        pickHigherClock(pakiet);

                // Odeślij ACK
		        sendPacket( 0, status.MPI_SOURCE, ACK );
            }
            else {

		        // Podmień .ts
		        pickHigherClock(pakiet);

                // Zapisz pakiet jako oczekujący na ACK
                ackQueue[last] = pakiet.src;
                last++;
            }

            pthread_mutex_unlock(&stateMut);
            pthread_mutex_unlock(&ackQueue_mutex);

	        break;
	    case ACK:

            incrementACK();
//            println("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);

	        break;

        case ADD_QUEUE:

            isAdded = false;
            int index = 0;

            // Maksymalnie 4 procesy uprawnione do grania (4 miejsca)
            while (index < 4) {
                // Jeżeli natrafiłeś na puste miejsce (wstaw na 1. puste miejsce)
                if (rooms[pakiet.room][index].ts == 0) {
                    isAdded = true;

                    // Zapisz proces jako uprawniony do grania
                    rooms[pakiet.room][index] = pakiet;

                    // Po znalezieniu ostatniego gracza
                    if (index == 3) {
                        // Sprawdź czy jesteś wsród graczy
                        if
                        (
                            rooms[pakiet.room][0].src == rank ||
                            rooms[pakiet.room][1].src == rank ||
                            rooms[pakiet.room][2].src == rank ||
                            rooms[pakiet.room][3].src == rank
                        ) {
                            
                            // Przeprowadź głosowanie
                            vote(pakiet);

                            // Wejdź do sekcji
                            stan = InSection;
                            sem_post(&inSection);
                        }

                        // Wyczyść kolejkę uprawnionych do grania
                        resetRoom(pakiet);

                        // Zmniejsz długość kolejki oczekujących graczy pokoju (4 weszło do sekcji)
                        decrement4Queue(pakiet);
                    }

                }
                // Po dodaniu gracza wyjdź
                if (isAdded) break;

                // Idź dalej
                index++;
            }

            break;

	    default:
	        break;
        }

    }
}
