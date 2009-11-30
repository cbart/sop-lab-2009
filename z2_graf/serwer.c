/** Multi-threaded server.                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

#include "err.h"

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
            obsłuż_rozkaz();
            zwróć_wynik();
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
 * Odpowiedź po IPC jest prosta. Dajemy po prostu typ taki jak PID pytającego
 * procesu. Ale jak wysłać zapytanie do serwera? w sensie, skąd klient ma
 * wiedzieć jaki jest numer IPC serwera?
 * ODP:
 * long cwd_string_size;
 * char *cwd;
 * char *buf;
 * cwd_string_size = pathconf(".", _PC_PATH_MAX);
 * if((buf = (char *) malloc((size_t) cwd_string_size)) == NULL)
 *     syserr("malloc");
 * cwd = getcwd(buf, (size_t) cwd_string_size);
 *
 * key_t ipc_key;
 * if((ipc_key = ftok(cwd, '*')) == (key_t) -1)
 *     syserr("ftok");
 * to trzeba zamknąć w jakiejś funkcji get_ipc_key...
 */
int main(void)
{
    while(TRUE) {
        pobierz_rozkaz_z_IPC();
        dodaj_rozkaz_do_kolejki_rozkazów();
        if(wątek czeka)
            obudź wątek;
        else if(liczba wątków < maksymalna liczba wątkow)
            utwórz wątek;
    }
    return 0;
}

