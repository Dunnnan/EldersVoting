#include "main.h"
#include "watek_komunikacyjny.h"

int ackCount = 0;
int relCount = 0;

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    bool isAdded = false;
    packet_t pakiet;

    game = random() % 3;

    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch ( status.MPI_TAG ) {
	    case REQUEST: 
            debug("Ktoś coś prosi. A niech ma!")

            if
            (
                (pakiet.ts < clockLamporta && pakiet.room == room) ||
                pakiet.room != room                                ||
                stan != InWant
            ) {
		        // Podmień .ts
                pthread_mutex_lock( &stateMut );
                if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
                else clockLamporta += 1;
                pthread_mutex_unlock( &stateMut );

		        sendPacket( 0, status.MPI_SOURCE, ACK );
            }
            else {
                // Podmień .ts
                pthread_mutex_lock( &stateMut );
                if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
                else clockLamporta += 1;
                pthread_mutex_unlock( &stateMut );
            }

	        break;
	    case ACK:
	        // Podmień .ts
            pthread_mutex_lock( &stateMut );
            if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
            else clockLamporta += 1;
            pthread_mutex_unlock( &stateMut );

            pthread_mutex_lock(&stateMut);
	        ackCount++;
            pthread_mutex_unlock(&stateMut);

            println("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount)
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);

	        break;
	    case RELEASE:

            if (pakiet.room == room) {
                 relCount++;
            }

	        // Podmień .ts
            pthread_mutex_lock( &stateMut );
            if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
            else clockLamporta += 1;
            pthread_mutex_unlock( &stateMut );

            println("Dostałem RELEASE od %d, mam już %d", status.MPI_SOURCE, relCount);
            debug("Dostałem RELEASE od %d", status.MPI_SOURCE);
            if (pakiet.room == room && relCount == 4) {
                debug("Ponawia rozesłanie REQUEST");
                ackCount = 0;

                println("Czyszczę rooms !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! relCount: %d", relCount);


                // Wyzeruj room
                memset(&rooms[pakiet.room][0], 0, sizeof(packet_t));
                memset(&rooms[pakiet.room][1], 0, sizeof(packet_t));
                memset(&rooms[pakiet.room][2], 0, sizeof(packet_t));
                memset(&rooms[pakiet.room][3], 0, sizeof(packet_t));

       		    packet_t *pkt = malloc(sizeof(packet_t));

                for (int i=0;i<=size-1;i++) {
                    if (i!=rank)
                        sendPacket( pkt, i, REQUEST);
                    }

                relCount = 0;
    		    free(pkt);
            }
            break;

        case ADD_QUEUE:

            println("Otrzymałem add QUEUE, dodaję go do tablicy oczekujących: src: %d | room: %d | game: %d ", pakiet.src, pakiet.room, pakiet.game);

            isAdded = false;
            int index = 0;
            while (index != 4) {
                println("index w ADD_QUEUE: %d : .ts = %d, .src = %d, room: %d ", index, rooms[room][index].ts, rooms[room][index].src, room);
                if (rooms[pakiet.room][index].ts == 0) {
                    isAdded = true;
                    rooms[pakiet.room][index] = pakiet;
                    if (index == 3) {
                        if
                        (
                            rooms[pakiet.room][0].src == rank ||
                            rooms[pakiet.room][1].src == rank ||
                            rooms[pakiet.room][2].src == rank ||
                            rooms[pakiet.room][3].src == rank
                        ) {
                            stan = InSection;
                            sem_post(&semaphore);
                            println("Rżniemy w karty - wszedłem do sekcji krytycznej. Pokój: %d ", rooms[pakiet.room][0].room);
                        }
                    }
                }
                if (isAdded) break;
                index++;
            }

            isAdded = false;

            break;

	    default:
	        break;
        }

    }
}
