#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/payment.h"

void addPendingBill(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[], float amount) {
    struct PaymentNode* newNode = (struct PaymentNode*)malloc(sizeof(struct PaymentNode));
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    newNode->amountDue = amount;
    newNode->prev = NULL;
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
        *tail = newNode;
        return;
    }

    (*tail)->next = newNode;
    newNode->prev = *tail;
    *tail = newNode; 
}

int processPayment(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[]) {
    struct PaymentNode* current = *head;

    while (current != NULL) 
    {
        if (current->block == block && strcmp(current->flatNo, flatNo) == 0) {
            
            if (current->prev == NULL && current->next == NULL) {
                *head = NULL;
                *tail = NULL;
            }
            else if (current->prev == NULL) {
                *head = current->next;
                (*head)->prev = NULL;
            }
            else if (current->next == NULL) {
                *tail = current->prev;
                (*tail)->next = NULL;
            }
            else {
                current->prev->next = current->next; // Connect left neighbor to right neighbor
                current->next->prev = current->prev; // Connect right neighbor to left neighbor
            }
            free(current);
            return 1; 
        }
        current = current->next;
    }
    return 0; 
}

int processPaymentAtIndex(struct PaymentNode** head, struct PaymentNode** tail, int queueIndex) {
    int currentIndex = 0;
    struct PaymentNode* current = *head;
    while (current != NULL) {
        if (currentIndex == queueIndex) {
            if (current->prev == NULL && current->next == NULL) {
                *head = NULL;
                *tail = NULL;
            } else if (current->prev == NULL) {
                *head = current->next;
                (*head)->prev = NULL;
            } else if (current->next == NULL) {
                *tail = current->prev;
                (*tail)->next = NULL;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            free(current);
            return 1;
        }
        current = current->next;
        currentIndex++;
    }
    return 0;
}

void displayPendingPayments(struct PaymentNode* head)
{
    if (head == NULL) 
    {
        return;
    }
    struct PaymentNode* current = head;
    while (current != NULL) 
    {
        current = current->next;
    }
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

void updatePaymentReferences(struct PaymentNode* head, char oldBlock, char oldFlatNo[], char newBlock, char newFlatNo[]) {
    struct PaymentNode* current = head;
    while (current != NULL) {
        if (current->block == oldBlock && strcmp(current->flatNo, oldFlatNo) == 0) {
            current->block = newBlock;
            strcpy(current->flatNo, newFlatNo);
        }
        current = current->next;
    }
}