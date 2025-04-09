# EldersVoting

## Algorytm

### Podejście 1.
1. Przydział procesom (dziadkom) preferowanej gry. (Rozkład normalny).
2. Inicjalizacja zegarów logicznych procesów na 0.
3. Losowo przydzielenie miejsca do zebrania się każdemu procesowi.
4. Każde z miejsc czeka na 4 kolejne procesy.
5. Po zebraniu się 4 procesów następuje głosowanie: (każdy z procesów komunikuje się ze sobą).
  5.1 Większościowa zgoda.
  5.2 Pełna zgoda.
/5. Ewentualnie komunikacja z miejscem (podliczanie głosów).
6. Wylosowanie nowego miejsca do głosowania oraz inkrementacja zegara logicznego. (gra została rozegrana).<br>
/6. Wylosowanie nowego miejsca do głosowania. (gra nie została rozegrana).

### Podejście 2.
1.  Wartość zegara logicznego procesu j-tego zainicjalizowana jako 0. Wartości wektora znaczników czasowych ostatnio otrzymanych wiadomości od każdego innego procesu zainicjalizowane jako 0. Priorytety ustalane są na podstawie znaczników czasowych. W przypadku, kiedy są one równe, decyduje id procesu.
2. Jeśli po otrzymaniu wiadomości od innego procesu wartość znacznika czasowego wiadomości jest wyższa niż zegara procesu odbiorcy, zastępuje on wartość swojego zegara wartością przychodzącą.
3. Aby zająć miejsce do gry, proces j-ty zwiększa wartość swojego zegara logicznego o 1 oraz wysyła REQ ze swoim znacznikiem czasowym oraz id miejsca, do którego chce wejść, do wszystkich innych procesów łącznie z samym sobą.
4. Proces j-ty otrzymuje dostęp do miejsca, jeśli:
  <br>(W1) Od wszystkich pozostałych procesów otrzymał wiadomości o starszej etykiecie czasowej.
  <br>(W2) Żądanie znajduje się w 4 najstarszych żądaniach spośród ubiegających się o to samo miejsce.
5. Po uzyskaniu dostępu do miejsca, proces j-ty wysyła wiadomość o swoim głosie na wybraną grę do wszystkich procesów w miejscu. Głosowanie odbywa się poprzez prostą większość – gra z największą liczbą głosów zostaje wybrana.
6. Po zakończeniu gry proces j-ty zwiększa wartość swojego zegara, wysyła RELEASE do wszystkich innych procesów, informując o zwolnieniu miejsca.
7. Proces i-ty po otrzymaniu REQ wstawia żądanie do lokalnej kolejki żądań (posortowanej po znacznikach czasowych/priorytetach), zwiększa o jeden wartość swojego zegara, następnie wysyła ACK do procesu j-tego.
8. Proces i-ty po otrzymaniu RELEASE od procesu j-tego usuwa wszystkie żądania procesu j-tego z kolejki.
9. W wersji uproszczonej (dla trójki): Istnieje tylko jedno miejsce o pojemności 4, a głosowanie nie jest wymagane – zawsze wybierana jest gra w rozbieranego pokera.


### Podejście 3.
1. Po otrzymaniu REQUEST proces wysyła ACK tylko kiedy spełniony jest którykolwiek z warunków:
   1. self.ts > REQ.ts && self.room == REQ.room
   2. self.room != REQ.room
   3. not (self.InWant)
2. Bez kolejek.

1.  Wartość zegara logicznego procesu j-tego zainicjalizowana jako 0. Wartości wektora znaczników czasowych ostatnio otrzymanych wiadomości od każdego innego procesu zainicjalizowane jako 0. Priorytety ustalane są na podstawie znaczników czasowych. W przypadku, kiedy są one równe, decyduje id procesu.
2. Jeśli po otrzymaniu wiadomości od innego procesu wartość znacznika czasowego wiadomości jest wyższa niż zegara procesu odbiorcy, zastępuje on wartość swojego zegara wartością przychodzącą.
3. Aby zająć miejsce do gry, proces j-ty zwiększa wartość swojego zegara logicznego o 1 oraz wysyła REQ ze swoim znacznikiem czasowym oraz id miejsca, do którego chce wejść, do wszystkich innych procesów łącznie z samym sobą.
4. Proces j-ty otrzymuje dostęp do miejsca, jeśli:
  <br>(W1) Od wszystkich pozostałych procesów otrzymał wiadomości o starszej etykiecie czasowej.
5. Po uzyskaniu dostępu do miejsca, proces j-ty wysyła wiadomość o swoim głosie na wybraną grę do wszystkich procesów w miejscu. Głosowanie odbywa się poprzez prostą większość – gra z największą liczbą głosów zostaje wybrana.
6. Po zakończeniu gry proces j-ty zwiększa wartość swojego zegara, wysyła RELEASE do wszystkich innych procesów, informując o zwolnieniu miejsca.
7. Po otrzymaniu RELEASE, wszystkie procesy ubiegające się o miejsce zwolnione resetują ackCount i ponawiają wysłanie REQUEST.
   
