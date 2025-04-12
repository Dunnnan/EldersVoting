#include "main.h"
#include "watek_glowny.h"



void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;
    sem_init(&semaphore, 0, 1);
    room = random()%ROOMS;

    while (stan != InFinish) {
	switch (stan) {
	    case InRun: 
            perc = random()%100;
            if ( perc < 25 ) {
                debug("Perc: %d", perc);
                println("Ubiegam się o sekcję krytyczną");
                debug("Zmieniam stan na wysyłanie");

                // Semafor na czekanie aż zbiorą się 4 osoby.
                sem_wait(&semaphore);

                //Losowanie gry
                game = random()%3;
                room = random()%ROOMS;

                packet_t *pkt = malloc(sizeof(packet_t));
                //pkt->data = perc;

                pthread_mutex_lock(&stateMut);
                ackCount = 0;
                pthread_mutex_unlock(&stateMut);

                for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, i, REQUEST);
                    changeState( InWant );
                free(pkt);
                debug("Skończyłem myśleć I CHCĘ WEJŚĆ DO SEKCJI BO MAM FAJNY PROCENT");
            }
            else debug("Skończyłem myśleć");
            break;
	    case InWant:
            println("Czekam na wejście do sekcji krytycznej")

            // tutaj zapewne jakiś semafor albo zmienna warunkowa
            // bo aktywne czekanie jest BUE

            // Podniesienie semaforu

            if ( ackCount > size - 4) {
                debug("ZDOBYŁEM ackCOUNT i czekam na kolegów.");

    		    packet_t *pkt = malloc(sizeof(packet_t));

                for (int i=0;i<=size-1;i++) {
                    sendPacket( pkt, i, ADD_QUEUE);
                }
    		    free(pkt);


                println("Wysłałem ADD_QUEUE i czekam na pozostałe osoby.")
                sem_wait(&semaphore);
                changeState(InSection);
            }
            break;

	    case InSection:
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej")
            sleep(15);
		    //if ( perc < 25 ) {
		    debug("Perc: %d", perc);
		    println("Wychodzę z sekcji krytycznej")
		    debug("Zmieniam stan na wysyłanie");

		    packet_t *pkt = malloc(sizeof(packet_t));
		    //pkt->data = perc;

		    for (int i=0;i<=size-1;i++){
			    sendPacket( pkt, i, RELEASE);
            }

		    changeState( InRun );

            // Zwolnienie semaforu

		    free(pkt);
            sem_post(&semaphore);
            debug("WŁAŚNIE ZWOLNIŁEM SEMAFOR");

		//}
		    break;
	    default: 
		    break;
        }
        sleep(SEC_IN_STATE);
    }
    sem_destroy(&semaphore);
}
