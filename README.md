# Problem Śpiącego Fryzjera
## Wprowadzenie
Symulacja Salonu Fryzjerskiego to projekt demonstracyjny ilustrujący zasady synchronizacji wątków w języku C, z wykorzystaniem semaforów i mutexów. Program emuluje działanie salonu fryzjerskiego z jednym fryzjerem i ograniczoną liczbą miejsc w poczekalni.

## Opis Projektu
Projekt symuluje:

- Gabinet z jednym fotelem: Gdzie jednocześnie może być obsługiwany tylko jeden klient.
- Poczekalnię z ograniczoną liczbą krzeseł: Klienci czekają na swoją kolej na wolnych miejscach. Gdy nie ma wolnych miejsc, odchodzą.
- Fryzjera: Fryzjer strzyże klientów i w przypadku braku klientów zasypia.

## Mechanizmy Synchronizacji
### Mutex (pthread_mutex_t mutex):

Przeznaczenie: Mutex zapewnia wzajemne wykluczanie podczas modyfikowania wspólnych zasobów takich jak
```
waiting_queue (kolejka czekających klientów)
waiting_count (liczba oczekujących klientów) 
rejected_count (liczba rezygnacji).
```
Sposób wykorzystania: Fryzjer i klienci używają mutexa, aby zapewnić, że tylko jeden wątek na raz może modyfikować wspólne zasoby. Mutex jest blokowany przy wchodzeniu do sekcji krytycznej (pthread_mutex_lock) i odblokowywany po zakończeniu operacji (pthread_mutex_unlock).

### Semafor (sem_t barber_ready):

Przeznaczenie: Semafor barber_ready sygnalizuje, że fryzjer jest gotowy do pracy lub czeka na klienta. Pozwala on fryzjerowi "spać", gdy nie ma klientów, i być obudzonym przez przychodzącego klienta.

Sposób wykorzystania: Klient wchodzący do poczekalni sprawdza, czy fryzjer śpi. Jeśli tak, klient zwiększa semafor (sem_post), budząc fryzjera. Fryzjer czeka (sem_wait), aż klient go obudzi.

### Semafor (sem_t customer_ready):

Przeznaczenie: Semafor customer_ready sygnalizuje, że klient jest gotowy do wejścia na fotel fryzjerski.
Sposób wykorzystania: Gdy fryzjer wybiera klienta do strzyżenia, zwiększa semafor (sem_post), sygnalizując klientowi, że może usiąść na fotelu. Klient czeka (sem_wait), aż fryzjer będzie gotowy go przyjąć