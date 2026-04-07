#ifndef PAYMENT_H
#define PAYMENT_H

struct PaymentNode {
    char block;
    char flatNo[10];
    float amountOwed;
    struct PaymentNode* next;
};

void enqueue(char block, char flatNo[], float amount);
void dequeue();
void displayPaymentQueue();

#endif