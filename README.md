# EldersVoting

## Algorytm
1.  Wartość zegara logicznego procesu j-tego zainicjalizowana jako 1. Wartości wektora znaczników czasowych ostatnio otrzymanych wiadomości od każdego innego procesu zainicjalizowane jako 1. Priorytety ustalane są na podstawie znaczników czasowych. W przypadku, kiedy są one równe, decyduje id procesu.
2. Jeśli po otrzymaniu wiadomości od innego procesu wartość znacznika czasowego wiadomości jest wyższa niż zegara procesu odbiorcy, zastępuje on wartość swojego zegara wartością przychodzącą.
3. Aby zająć miejsce do gry, proces j-ty zwiększa wartość swojego zegara logicznego o 1 oraz wysyła REQ ze swoim znacznikiem czasowym oraz id miejsca, do którego chce wejść, do wszystkich innych procesów łącznie z samym sobą.
4. Proces j-ty otrzymuje dostęp do miejsca, jeśli:
  <br>(W1) Od n-4 procesów otrzymał wiadomości o starszej etykiecie czasowej.
5. Po uzyskaniu dostępu do miejsca, proces j-ty wysyła wiadomość w celu dodania się na listę graczy danego pokoju (wraz z numerem preferowanej gry).
6. Po dodaniu ostatniego (czwartego) gracza do listy pokoju odbywa się głosowanie na podstawie prostej większości. W przypadku remisu wybierana jest gra wskazana przez najmłodszy proces.
7. Po przeprowadzeniu głosowania każdy z procesów czyści listę graczy danego pokoju.
8. Po zakończeniu gry proces j-ty rozsyła ACK wszystkim procesom, które wcześniej zignorował.
9. Proces i-ty po otrzymaniu REQ odsyła ACK, bądź ignoruje żądanie i zapamiętuje id procesu żądającego w tablicy, w celu wysłania ACK w późniejszym czasie.
10. W wersji uproszczonej (dla trójki): Istnieje tylko jedno miejsce o pojemności 4, a głosowanie nie jest wymagane – zawsze wybierana jest gra w rozbieranego pokera.
