#include "main.h"
#include "watek_komunikacyjny.h"

int ackCount = 0;

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock( &stateMut );
        if (pakiet.ts > clockLamporta) clockLamporta = pakiet.ts + 1;
        else clockLamporta += 1;
	    pthread_mutex_unlock( &stateMut );

        switch ( status.MPI_TAG ) {
	    case REQUEST: 
            debug("Ktoś coś prosi. A niech ma!")
		    sendPacket( 0, status.MPI_SOURCE, ACK );
	        break;
	    case ACK: 
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            pthread_mutex_lock(&stateMut);
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            pthread_mutex_unlock(&stateMut);
	        break;
            
        //case NACK:
        //    debug("Dostałem NACK od %d, mam już %d", status.MPI_SOURCE);


	    default:
	        break;
        }
    }
}
