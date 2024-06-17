#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NUM_CHAIRS 5
#define MAX_HAIRCUT_TIME 2
#define MAX_THREADS 50

pthread_mutex_t mutex;
pthread_cond_t barber_ready;
pthread_cond_t customer_ready;

int waiting_queue[NUM_CHAIRS];
int waiting_count = 0;
int rejected_customers[100];
int rejected_count = 0;

int info_mode = 0;

int waiting_customers = 0;
int customers_rejected = 0;

void* barber(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Fryzjer idzie spać tylko jeśli nie ma klientów w poczekalni
        if (waiting_count == 0) {
            printf("Fryzjer: Brak klientów. Idę spać.\n");
            pthread_cond_wait(&barber_ready, &mutex);
        }

        int current_customer = waiting_queue[0];
        for (int i = 0; i < waiting_count - 1; i++) {
            waiting_queue[i] = waiting_queue[i + 1];
        }
        waiting_count--;
        waiting_customers--;

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

        printf("Fryzjer: Rozpoczynam strzyżenie klienta %d\n", current_customer);
        sleep(rand() % MAX_HAIRCUT_TIME + 1);
        printf("Fryzjer: Zakończyłem strzyżenie klienta %d\n", current_customer);
    }
    return NULL;
}

void* customer(void* arg) {
    int customer_id = *(int*)arg;
    printf("Klient %d: Przyszedłem do salonu.\n", customer_id);

    pthread_mutex_lock(&mutex);

    if (waiting_customers < NUM_CHAIRS) {
        waiting_customers++;
        waiting_queue[waiting_count++] = customer_id;
        printf("Klient %d: Siadam w poczekalni. Klientów w poczekalni: %d\n", customer_id, waiting_customers);

        if (waiting_count == 1) {
            pthread_cond_signal(&barber_ready);
            printf("Klient %d: Obudziłem fryzjera.\n", customer_id);
        }

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

        pthread_cond_wait(&customer_ready, &mutex);
    } else {
        customers_rejected++;
        rejected_customers[rejected_count++] = customer_id;
        printf("Klient %d: Brak miejsc w poczekalni. Odszedłem. Rezygnacje: %d\n", customer_id, customers_rejected);

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

    if (argc > 1 && strcmp(argv[1], "-info") == 0) {
        info_mode = 1;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&barber_ready, NULL);
    pthread_cond_init(&customer_ready, NULL);

    srand(time(NULL));

    pthread_create(&barber_thread, NULL, barber, NULL);

    // Tworzenie wątków klientów
    for (int i = 0; i < MAX_THREADS; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_threads[i], NULL, customer, &customer_ids[i]);
        sleep(rand() % (1 + 1)); // Losowe opóźnienie dla symulacji przyjścia klientów
    }

    // Oczekiwanie na zakończenie wątków klientów
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Zakończenie wątku fryzjera
    pthread_cancel(barber_thread);

    // Usunięcie mutexu i zmiennych warunkowych
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&barber_ready);
    pthread_cond_destroy(&customer_ready);

    return 0;
}
