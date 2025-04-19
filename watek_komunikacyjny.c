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
            //println("Dostałem REQ. Czy wyślę ACK? REQ.ts: %d ? %d : myLastREQ.ts | stan: %d | room: %d | mojRoom: %d | od: %d",pakiet.ts, lastRequestTS, stan, pakiet.room, room, pakiet.src);
            if
            (
                (
                ((pakiet.ts < lastRequestTS && pakiet.room == room) || (pakiet.ts == lastRequestTS && pakiet.src < rank && pakiet.room == room) ||
                stan != InWant)
                &&
                stan != Waiting && stan != InSection) || pakiet.room != room
            ) {

                packet_t *pkt = malloc(sizeof(packet_t));

		        // Podmień .ts
                pthread_mutex_lock( &stateMut );
                if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts;

                pkt->request_id = pakiet.request_id;

                //else clockLamporta += 1;
                pthread_mutex_unlock( &stateMut );



		        sendPacket( pkt, status.MPI_SOURCE, ACK );
            }
/*
            else {
                // Podmień .ts
                pthread_mutex_lock( &stateMut );
                if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
                else clockLamporta += 1;
                pthread_mutex_unlock( &stateMut );
            }
*/
	        break;
	    case ACK:

/*
	        // Podmień .ts
            pthread_mutex_lock( &stateMut );
            if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
            else clockLamporta += 1;
            pthread_mutex_unlock( &stateMut );
*/
            pthread_mutex_lock(&stateMut);
	        if (pakiet.request_id == request_id) {
                ackCount++;
                //println("Dostałem ACK od %d", pakiet.src);
                }
            else {
                println("Zignorowano ACK od %d - stary request_id", pakiet.src);
            }
            pthread_mutex_unlock(&stateMut);

            //println("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount)
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);

	        break;
	    case RELEASE:

            // Wyzeruj room
            memset(&rooms[pakiet.room][0], 0, sizeof(packet_t));
            memset(&rooms[pakiet.room][1], 0, sizeof(packet_t));
            memset(&rooms[pakiet.room][2], 0, sizeof(packet_t));
            memset(&rooms[pakiet.room][3], 0, sizeof(packet_t));



            if (pakiet.room == room ) { // Jeżeli zaczniemy się ubiegać w trakcie przetwarzania Releasów, to część z nich
                 relCount++;           // zwiększy relCount, a część nie = nie będzie wyzerowania ackCount i ponownego rozesłania REQ

//                if (relCount == 1) {
//
//                }

            }

            /*
	        // Podmień .ts
            pthread_mutex_lock( &stateMut );
            if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
            else clockLamporta += 1;
            pthread_mutex_unlock( &stateMut );
            */

            //println("Dostałem RELEASE od %d, mam już %d", status.MPI_SOURCE, relCount);
            debug("Dostałem RELEASE od %d", status.MPI_SOURCE);
            if (pakiet.room == room && relCount == 4) {
                debug("Ponawia rozesłanie REQUEST");

                pthread_mutex_lock(&stateMut);
                ackCount = 0;
                pthread_mutex_unlock(&stateMut);

                //println("Czyszczę rooms !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! relCount: %d", relCount);


       		    packet_t *pkt = malloc(sizeof(packet_t));


                if (stan == InWant) {

                    pthread_mutex_lock(&stateMut);
                    request_id++;
                    pkt->request_id = request_id;
                    pthread_mutex_unlock(&stateMut);

                    for (int i=0;i<=size-1;i++) {
                        if (i!=rank)
                            sendPacket( pkt, i, REQUEST);
                        }
                }


                relCount = 0;
    		    free(pkt);
            }
            break;

        case ADD_QUEUE:

            //println("Otrzymałem add QUEUE, dodaję go do tablicy oczekujących: src: %d | room: %d | game: %d ", pakiet.src, pakiet.room, pakiet.game);

            isAdded = false;
            int index = 0;
            while (index != 4) {
                //println("index w ADD_QUEUE: %d : .ts = %d, .src = %d, room: %d ", index, rooms[pakiet.room][index].ts, rooms[pakiet.room][index].src, pakiet.room);
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
                            //println("Rżniemy w karty - wszedłem do sekcji krytycznej. Pokój: %d ", rooms[pakiet.room][0].room);
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
