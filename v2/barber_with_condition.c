#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NUM_CHAIRS 2
#define MAX_HAIRCUT_TIME 5

pthread_mutex_t mutex;
pthread_cond_t cond_barber_ready;
pthread_cond_t cond_customer_ready;

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
        while (waiting_customers == 0) {
            printf("Fryzjer: Czekam na klienta...\n");
            pthread_cond_wait(&cond_barber_ready, &mutex);
        }

        int current_customer = waiting_queue[0];
        for (int i = 0; i < waiting_count - 1; i++) {
            waiting_queue[i] = waiting_queue[i + 1];
        }
        waiting_count--;
        waiting_customers--;
        
        printf("Poczekalnia: %d/%d [Fotel: %d]\n", waiting_customers, NUM_CHAIRS, current_customer);

        if (info_mode) {
            printf("Aktualna kolejka: ");
            for (int i = 0; i < waiting_count; i++) {
                printf("%d ", waiting_queue[i]);
            }
            printf("\nRezygnacje: %d\n", customers_rejected);
        }
        
        pthread_cond_signal(&cond_customer_ready);
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
        pthread_cond_signal(&cond_barber_ready);

        if (info_mode) {
            printf("Aktualna kolejka: ");
            for (int i = 0; i < waiting_count; i++) {
                printf("%d ", waiting_queue[i]);
            }
            printf("\nRezygnacje: %d\n", customers_rejected);
        }

        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while (customer_id != waiting_queue[0]) {
            pthread_cond_wait(&cond_customer_ready, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    } else {
        customers_rejected++;
        rejected_customers[rejected_count++] = customer_id;
        printf("Klient %d: Brak miejsc w poczekalni. Odszedłem. Rezygnacje: %d\n", customer_id, customers_rejected);

        if (info_mode) {
            printf("Aktualna kolejka: ");
            for (int i = 0; i < waiting_count; i++) {
                printf("%d ", waiting_queue[i]);
            }
            printf("\nRezygnacje: %d\n", customers_rejected);
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t barber_thread;
    pthread_t customer_threads[15];
    int customer_ids[15];

    if (argc > 1 && strcmp(argv[1], "-info") == 0) {
        info_mode = 1;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_barber_ready, NULL);
    pthread_cond_init(&cond_customer_ready, NULL);

    srand(time(NULL));

    pthread_create(&barber_thread, NULL, barber, NULL);

    for (int i = 0; i < 15; i++) {
        customer_ids[i] = i + 1;
        pthread_create(&customer_threads[i], NULL, customer, &customer_ids[i]);
        sleep(rand() % (MAX_HAIRCUT_TIME + 1));
    }

    for (int i = 0; i < 15; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    pthread_cancel(barber_thread);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_barber_ready);
    pthread_cond_destroy(&cond_customer_ready);

    return 0;
}
