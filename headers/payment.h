#ifndef PAYMENT_H
#define PAYMENT_H

struct PaymentNode {
    char block;
    char flatNo[10];
    float amountDue;
    struct PaymentNode* prev;
    struct PaymentNode* next;
};

void addPendingBill(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[], float amount);
int processPayment(struct PaymentNode** head, struct PaymentNode** tail, char block, char flatNo[]);
int processPaymentAtIndex(struct PaymentNode** head, struct PaymentNode** tail, int queueIndex);
void displayPendingPayments(struct PaymentNode* head);
void savePayments(struct PaymentNode* head);
void loadPayments(struct PaymentNode** head, struct PaymentNode** tail);
int hasPendingPayment(struct PaymentNode* head, char block, char flatNo[]);
void updatePaymentReferences(struct PaymentNode* head, char oldBlock, char oldFlatNo[], char newBlock, char newFlatNo[]);

#endif
