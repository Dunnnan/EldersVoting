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

    // Losuj grę
    game = random() % 3;

    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch ( status.MPI_TAG ) {
	    case REQUEST: 

            // Sprawdź czy możesz wysłać ACK
            pthread_mutex_lock(&ackQueue_mutex);
            if
            (
                pakiet.room != room ||
                (
                    stan == InRun ||
                    (
                        (pakiet.ts < lastRequestTS && pakiet.room == room) ||                       // != ts (porównanie ts)
                        (pakiet.ts == lastRequestTS && pakiet.src < rank && pakiet.room == room)    // == ts (porównanie id)
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

                // Zapisz pakiet jako oczekujący na ACK
                ackQueue[last] = pakiet.src;
                last++;
            }

            pthread_mutex_unlock(&ackQueue_mutex);
	        break;
	    case ACK:

            incrementACK();
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);

	        break;

        case ADD_QUEUE:

            isAdded = false;
            int index = 0;

            // Maksymalnie 4 procesy uprawnione do grania (4 miejsca)
            while (index < 4) {
                // Jeżeli natrafiłeś na puste miejsce
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
