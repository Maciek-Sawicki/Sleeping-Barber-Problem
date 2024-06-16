#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Liczba krzeseł w poczekalni
#define NUM_CHAIRS 5
// Maksymalny czas strzyżenia
#define MAX_HAIRCUT_TIME 2
// Maksymalna liczba wątków
#define MAX_THREADS 50

// Semafory i mutex
pthread_mutex_t mutex;
sem_t barber_ready;  // Fryzjer gotowy do pracy lub czeka na klienta
sem_t customer_ready;  // Klient gotowy do obsługi przez fryzjera

// Kolejki FIFO na klientów czekających i rezygnujących
int waiting_queue[NUM_CHAIRS];
int waiting_count = 0;
int rejected_customers[100];
int rejected_count = 0;

int info_mode = 0;

// Liczba klientów w poczekalni i rezygnacji
int waiting_customers = 0;
int customers_rejected = 0;

// Funkcja fryzjera
void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        if (waiting_count == 0) {
            printf("Fryzjer: Brak klientów. Idę spać.\n");
            pthread_mutex_unlock(&mutex);
            sem_wait(&barber_ready);  // Fryzjer czeka, aż klient go obudzi
        } else {
            int current_customer = waiting_queue[0];
            for (int i = 0; i < waiting_count - 1; i++) {
                waiting_queue[i] = waiting_queue[i + 1];
            }
            waiting_count--;
            waiting_customers--;

            // Wyświetlanie stanu poczekalni i obsługiwanego klienta
            printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", customers_rejected, waiting_customers, NUM_CHAIRS, current_customer);
            if (info_mode) {
                printf("Aktualna kolejka: ");
                for (int i = 0; i < waiting_count; i++) {
                    printf("%d ", waiting_queue[i]);
                }
                printf("\nRezygnacje: ");
                for (int i = 0; i < rejected_count; i++) {
                    printf("%d ", rejected_customers[i]);
                }
                printf("\n");
            }

            pthread_mutex_unlock(&mutex);

            // Powiadomienie klienta o możliwości wejścia na fotel
            sem_post(&customer_ready);

            // Symulacja strzyżenia
            printf("Fryzjer: Rozpoczynam strzyżenie klienta %d\n", current_customer);
            sleep(rand() % MAX_HAIRCUT_TIME + 1);
            printf("Fryzjer: Zakończyłem strzyżenie klienta %d\n", current_customer);
        }
    }
    return NULL;
}

// Funkcja klienta
void* customer(void* arg) {
    int customer_id = *(int*)arg;
    printf("Klient %d: Przyszedłem do salonu.\n", customer_id);

    pthread_mutex_lock(&mutex);
    if (waiting_customers < NUM_CHAIRS) {
        waiting_customers++;
        waiting_queue[waiting_count++] = customer_id;
        printf("Klient %d: Siadam w poczekalni. Klientów w poczekalni: %d\n", customer_id, waiting_customers);

        // Jeśli fryzjer śpi (czyli nie obsługuje klienta i nie czeka na klienta), obudź go
        if (waiting_count == 1) {
            sem_post(&barber_ready);  // Obudź fryzjera, gdy klient zajmie miejsce w poczekalni
            printf("Klient %d: Obudziłem fryzjera.\n", customer_id);
        }

        // Wyświetlanie stanu po zajęciu miejsca w poczekalni
        printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: -]\n", customers_rejected, waiting_customers, NUM_CHAIRS);
        if (info_mode) {
            printf("Aktualna kolejka: ");
            for (int i = 0; i < waiting_count; i++) {
                printf("%d ", waiting_queue[i]);
            }
            printf("\nRezygnacje: ");
            for (int i = 0; i < rejected_count; i++) {
                printf("%d ", rejected_customers[i]);
            }
            printf("\n");
        }

        pthread_mutex_unlock(&mutex);

        // Klient czeka na sygnał od fryzjera, żeby usiąść na fotelu
        sem_wait(&customer_ready);
    } else {
        customers_rejected++;
        rejected_customers[rejected_count++] = customer_id;
        printf("Klient %d: Brak miejsc w poczekalni. Odszedłem. Rezygnacje: %d\n", customer_id, customers_rejected);

        // Wyświetlanie stanu po rezygnacji klienta
        printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: -]\n", customers_rejected, waiting_customers, NUM_CHAIRS);
        if (info_mode) {
            printf("Aktualna kolejka: ");
            for (int i = 0; i < waiting_count; i++) {
                printf("%d ", waiting_queue[i]);
            }
            printf("\nRezygnacje: ");
            for (int i = 0; i < rejected_count; i++) {
                printf("%d ", rejected_customers[i]);
            }
            printf("\n");
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t barber_thread;
    pthread_t customer_threads[MAX_THREADS];
    int customer_ids[MAX_THREADS];

    // Sprawdzenie, czy tryb informacji jest włączony
    if (argc > 1 && strcmp(argv[1], "-info") == 0) {
        info_mode = 1;
    }

    // Inicjalizacja semaforów i mutexa
    pthread_mutex_init(&mutex, NULL);
    sem_init(&barber_ready, 0, 0);
    sem_init(&customer_ready, 0, 0);

    // Inicjalizacja generatora liczb losowych
    srand(time(NULL));

    // Utworzenie wątku fryzjera
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Utworzenie wątków klientów
    for (int i = 0; i < MAX_THREADS; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_threads[i], NULL, customer, &customer_ids[i]);
        sleep(rand() % (1 + 1));  // Klienci przychodzą w losowych odstępach czasu
    }

    // Czekanie na zakończenie wątków klientów
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Zakończenie wątku fryzjera (nigdy nie nastąpi w tej symulacji)
    pthread_cancel(barber_thread);

    // Sprzątanie zasobów
    pthread_mutex_destroy(&mutex);
    sem_destroy(&barber_ready);
    sem_destroy(&customer_ready);

    return 0;
}
