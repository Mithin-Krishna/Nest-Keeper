#ifndef COMPLAINT_H
#define COMPLAINT_H

struct ComplaintRecord {
    int id;
    char block;
    char flatNo[10];
    char type[40];
    char description[160];
    int priority;
    char status[20];
    long createdOrder;
};

struct ComplaintHeap {
    struct ComplaintRecord* items;
    int size;
    int capacity;
    int nextId;
    long nextOrder;
};

void initComplaintHeap(struct ComplaintHeap* heap);
void freeComplaintHeap(struct ComplaintHeap* heap);
int addComplaint(
    struct ComplaintHeap* heap,
    char block,
    char flatNo[],
    char type[],
    char description[],
    int priority,
    char status[]
);
int updateComplaintStatus(struct ComplaintHeap* heap, int id, char status[]);
int removeComplaintById(struct ComplaintHeap* heap, int id);
int getComplaintCount(const struct ComplaintHeap* heap);
int fillComplaintsByPriority(const struct ComplaintHeap* heap, struct ComplaintRecord* out, int maxItems);
void saveComplaints(const struct ComplaintHeap* heap);
void loadComplaints(struct ComplaintHeap* heap);

#endif
