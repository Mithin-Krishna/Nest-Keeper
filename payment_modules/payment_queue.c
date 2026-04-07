#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/payment.h"

struct PaymentNode* front = NULL;
struct PaymentNode* rear = NULL;

void enqueue(char block, char flatNo[], float amount) 
{
    struct PaymentNode* newNode = (struct PaymentNode*)malloc(sizeof(struct PaymentNode));
    newNode->block = block;
    strcpy(newNode->flatNo, flatNo);
    newNode->amountOwed = amount;
    newNode->next = NULL;
    if (rear == NULL) 
    {
        front = rear = newNode;
        printf("Added Block %c, Flat %s to the payment queue (Owes: INR. %.2f).\n", block, flatNo, amount);
        return;
    }
    rear->next = newNode;
    rear = newNode;
    printf("Added Block %c, Flat %s to the payment queue (Owes: INR. %.2f).\n", block, flatNo, amount);
}

void dequeue() 
{
    if (front == NULL)
    {
        printf("The payment queue is currently empty. No pending dues!\n");
        return;
    }
    struct PaymentNode* temp = front;
    printf("SUCCESS: Payment of INR. %.2f received from Block %c, Flat %s. Cleared from queue!\n", temp->amountOwed, temp->block, temp->flatNo);
    front = front->next;
    if (front == NULL) 
    {
        rear = NULL;
    }
    free(temp);
}

void displayPaymentQueue() 
{
    if (front == NULL) 
    {
        printf("\n--- Payment Queue is Empty ---\n");
        return;
    }
    struct PaymentNode* current = front;
    int position = 1;
    printf("\n--- Pending Payment Queue ---\n");
    while (current != NULL) 
    {
        printf("%d. Block: %c | Flat: %s | Amount: INR. %.2f\n", position, current->block, current->flatNo, current->amountOwed);
        current = current->next;
        position++;
    }
    printf("-----------------------------\n");
}