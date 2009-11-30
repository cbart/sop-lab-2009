/** Multi-threaded server.                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "err.h"
#include "ipc_msg.h"

void * server_thread(void *arg)
/* Co trzeba przekazać tej funkcji?
 *   numer tego wątku w tablicy wątków.
 *   adres stosu indeksów wolnych wątków.
 *   */
{
    thread_register();
    pthread_cleanup_push(thread_unregister);
    int timedwait_result;
    while() { /* dopóki są jeszcze nieobsłużone rozkazy. */
        while() { /* dopóki są jeszcze nieobsłużone rozkazy. */
            if(główny_wątek_dał_sygnał_do_zakończenia)
                return koniec;
            pthread_cleanup_push(odpowiedz_klientowi_bledem());
            pobierz_rozkaz();
            if(!rozkaz_jest_poprawny())
                zwróć_wynik(błąd);
            else {
                obsłuż_rozkaz();
                zwróć_wynik();
            }
            pthread_cleanup_pop(FALSE);  /* odpowiedz_klientowi_bledem(); */
        }
        timedwait_result = pthread_cond_timedwait(cond, mutex, abstime);
        if(timedwait_result != 0 && timedwait != ETIMEDOUT)
            syserr("Error in thread: phtread_cond_timedwait.");
    }
    pthread_cleanup_pop(TRUE);  /* thread_unregister(); */
}
/*
 * Czy w wypadku SIGINTa i innych takich ustawiamy sobie zmienną: zakończcie się
 * wszyscy i czytamy ją w wątkach, czy raczej dajemy cancel i w wątku dajemy
 * przed obsłużeniem każdego zapytania sztuczny cancelation point?
 *
 * Jak podczas zakończenia wątku informujemy główny wątek o tym, że "zwolniło
 * się miejsce"?
 * ODP: W procedurze thread_unregister() wykonujemy co następuje:
 * 1. odejmujemy jedynkę od licznika odpalonych wątków.
 * 2. dodajemy swój indeks wątku (w tablicy wątków) na stos "wolnych indeksów".
 * w momencie, gdy główny wątek chce utworzyć nowy proces robi tak:
 * jak stos jest niepusty to bierze indeks z góry i tworzy wątek.
 * wpp. jeżeli liczba odpalonych procesów jest < maksymalnej to
 * bierze liczbę odpalonych procesów, tworzy wątek o indeksie równym tej liczbie
 * i dodaje jedynkę do liczby odpalonych procesów.
 *
 */
int main(void)
{
    int msg_id;
    struct msgbuf *buffer;
    size_t buf_size;
    key_t ipc_key = get_ipc_key();

    /* Create IPC queue. */
    msg_id = msgget(ipc_key, IPC_CREAT|IPC_EXCL);

    while(TRUE) {
        /* Get request from IPC. */
        if(msgrcv(msg_id, buffer, buf_size, IPC_ORDERS_RESERVED, 0) == -1)
            syserr("msgrcv: While receiving IPC message.");
        
        dodaj_rozkaz_do_kolejki_rozkazów();
        if(wątek czeka)
            obudź wątek;
        else if(liczba wątków < maksymalna liczba wątkow)
            utwórz wątek;
    }

    // zakończ wszystkie wątki

    /* Remove IPC queue. */
    if(msgctl(msg_id, IPC_RMID, NULL) != 0)
        syserr("msgctl: While removing IPC message queue.");
    return 0;
}

