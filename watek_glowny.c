#include "main.h"
#include "watek_glowny.h"


void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    // Semafor na czekanie aż zbiorą się 4 osoby.
    sem_init(&inSection, 0, 1);
    room = -1;

    while (stan != InFinish) {
	switch (stan) {
	    case InRun: 
            perc = random()%100;

            if ( perc < 25 ) {
                debug("Zmieniam stan na wysyłanie");

                pthread_mutex_lock(&stateMut);

                // Semafor na czekanie aż zbiorą się 4 osoby.
                sem_wait(&inSection);

                // Inicjalizuj ubieganie się
                resetACK();
                incrementClock();
                rememberRequestTS();        // <- changeState( InWant)

                // Losuj grę
                game = random() % GAMES;

                // Wybierz pokój z najkrótszą kolejką
                room = pickRoom();

                // Wyślij requesty
                packet_t *pkt = malloc(sizeof(packet_t));
                for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, i, REQUEST);
                free(pkt);

                pthread_mutex_unlock(&stateMut);

                debug("Skończyłem myśleć I CHCĘ WEJŚĆ DO SEKCJI BO MAM FAJNY PROCENT");
            }
            else debug("Skończyłem myśleć");

            break;
	    case InWant:

            // Czekaj aż uzyskasz ackCount
            if (ackCount >= size - 4) {

                debug("ZDOBYŁEM ackCOUNT i czekam na kolegów.");
                changeState(Waiting);

                // Wyślij dodanie do kolejki pokoju
                packet_t *pkt = malloc(sizeof(packet_t));
                for (int i=0;i<=size-1;i++) {
                    sendPacket( pkt, i, ADD_QUEUE);
                }
                free(pkt);

                // Czekaj na wejście do sekcji
                sem_wait(&inSection);

                // Wejdź do sekcji
                changeState(InSection);

            }

            break;

	    case InSection:
//            println("Jestem w sekcji krytycznej. room: %d", room);
            sleep(3);

            // Wyzeruj ACK
            resetACK();

		    println("Wychodzę z sekcji krytycznej. room: %d", room)

            // Zresetuj pokój
            resetPickedRoom();

            // Wyjdź z sekcji
	        changeState( InRun );

            // Roześlij zapamiętane ACK
            resendACK();

            // Zwolnienie semaforu
            sem_post(&inSection);
            debug("WŁAŚNIE ZWOLNIŁEM SEMAFOR");


	    break;

	    default: 
		    break;
        }
        sleep(SEC_IN_STATE);
    }
    sem_destroy(&inSection);
}
