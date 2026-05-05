#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/payment.h"

void addPendingBill(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[], float amount) {
    // 1. Create the new bill
    struct PaymentNode* newNode = (struct PaymentNode*)malloc(sizeof(struct PaymentNode));
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    newNode->amountDue = amount;
    newNode->prev = NULL;
    newNode->next = NULL;

    // 2. If the list is empty, this is the first bill
    if (*head == NULL) {
        *head = newNode;
        *tail = newNode;
        printf("System: Added first pending bill for Block %c, Flat %s.\n", block, flatNo);
        return;
    }

    // 3. Otherwise, attach it to the end of the queue (tail)
    (*tail)->next = newNode;
    newNode->prev = *tail;
    *tail = newNode; // Update the tail pointer
    
    printf("System: Added pending bill for Block %c, Flat %s.\n", block, flatNo);
}

int processPayment(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[]) {
    struct PaymentNode* current = *head;

    // Search through the queue
    while (current != NULL) 
    {
        if (current->block == block && strcmp(current->flatNo, flatNo) == 0) {
            
            // TARGET FOUND! Now we safely unlink it from the DLL.
            
            // Case A: Target is the ONLY node in the list
            if (current->prev == NULL && current->next == NULL) {
                *head = NULL;
                *tail = NULL;
            }
            // Case B: Target is the HEAD (front of the queue)
            else if (current->prev == NULL) {
                *head = current->next;
                (*head)->prev = NULL;
            }
            // Case C: Target is the TAIL (end of the queue)
            else if (current->next == NULL) {
                *tail = current->prev;
                (*tail)->next = NULL;
            }
            // Case D: Target is in the MIDDLE (The exact problem you wanted to solve!)
            else {
                current->prev->next = current->next; // Connect left neighbor to right neighbor
                current->next->prev = current->prev; // Connect right neighbor to left neighbor
            }

            printf("\n[SUCCESS] Payment of $%.2f processed for Block %c, Flat %s.\n", current->amountDue, block, flatNo);
            
            free(current);
            return 1; 
        }
        current = current->next;
    }
    printf("\n[ERROR] No pending bills found for Block %c, Flat %s.\n", block, flatNo);
    return 0; 
}

void displayPendingPayments(struct PaymentNode* head)
{
    if (head == NULL) 
    {
        printf("\nAll caught up! No pending payments in the system.\n");
        return;
    }
    struct PaymentNode* current = head;
    printf("\n--- PENDING PAYMENTS QUEUE ---\n");
    while (current != NULL) 
    {
        printf("Block: %c | Flat: %-5s | Amount Due: $%.2f\n", current->block, current->flatNo, current->amountDue);
        current = current->next;
    }
    printf("------------------------------\n");
}

#pragma pack(push, 1)
struct PaymentDiskRecord {
    char block;
    char flatNo[10];
    float amountDue;
};
#pragma pack(pop)

void savePayments(struct PaymentNode* head) {
    FILE* file = fopen("payments.dat", "wb"); // Wipes and rewrites the current snapshot
    if (file == NULL) {
        printf("[ERROR] Could not save payment snapshot.\n");
        return;
    }

    struct PaymentNode* current = head;
    while (current != NULL) {
        struct PaymentDiskRecord record;
        record.block = current->block;
        strcpy(record.flatNo, current->flatNo);
        record.amountDue = current->amountDue;
        
        fwrite(&record, sizeof(struct PaymentDiskRecord), 1, file);
        current = current->next;
    }
    fclose(file);
}

void loadPayments(struct PaymentNode** head, struct PaymentNode** tail) {
    FILE* file = fopen("payments.dat", "rb");
    if (file == NULL) return; 

    struct PaymentDiskRecord record;
    int count = 0;
    
    while (fread(&record, sizeof(struct PaymentDiskRecord), 1, file)) {
        // Manually recreate the nodes to avoid triggering the "Added bill" printf spam on boot
        struct PaymentNode* newNode = (struct PaymentNode*)malloc(sizeof(struct PaymentNode));
        newNode->block = record.block;
        strcpy(newNode->flatNo, record.flatNo);
        newNode->amountDue = record.amountDue;
        newNode->prev = NULL;
        newNode->next = NULL;

        if (*head == NULL) {
            *head = newNode;
            *tail = newNode;
        } else {
            (*tail)->next = newNode;
            newNode->prev = *tail;
            *tail = newNode;
        }
        count++;
    }
    
    fclose(file);
    if (count > 0) {
        printf("[SYSTEM] Boot Sequence: Loaded %d pending payments into the queue.\n", count);
    }
}

int hasPendingPayment(struct PaymentNode* head, char block, char flatNo[]) {
    struct PaymentNode* current = head;
    while (current != NULL) {
        if (current->block == block && strcmp(current->flatNo, flatNo) == 0) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}