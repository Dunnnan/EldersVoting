#include "main.h"
#include "watek_glowny.h"



void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    // Semafor na czekanie aż zbiorą się 4 osoby.
    sem_init(&inSection, 0, 1);
    room = random()%ROOMS;

    while (stan != InFinish) {
	switch (stan) {
	    case InRun: 
            perc = random()%100;

            if ( perc < 25 ) {
                //println("Ubiegam się o sekcję krytyczną");
                debug("Zmieniam stan na wysyłanie");

                // Semafor na czekanie aż zbiorą się 4 osoby.
                sem_wait(&inSection);

                //Losuj grę i pokój
                game = random()%3;
                room = random()%ROOMS;

                // Inicjalizuj ubieganie się
                resetACK();
                incrementClock();
                rememberRequestTS();

                // Wyślij requesty
                packet_t *pkt = malloc(sizeof(packet_t));
                for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, i, REQUEST);
                free(pkt);

                // Zacznij ubiegać się o sekcję
                changeState( InWant );

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
            println("Jestem w sekcji krytycznej. room: %d", room);
            sleep(1);

            // Wyzeruj ACK
            resetACK();

		    println("Wychodzę z sekcji krytycznej. room: %d", room)

            // Zresetuj pokój
            room = -1;

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
