#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/complaint.h"

#define INITIAL_COMPLAINT_CAPACITY 8

#pragma pack(push, 1)
struct ComplaintFileHeader {
    int size;
    int nextId;
    long nextOrder;
};
#pragma pack(pop)

static int complaintComesFirst(const struct ComplaintRecord* left, const struct ComplaintRecord* right) {
    if (left->priority != right->priority) {
        return left->priority > right->priority;
    }
    return left->createdOrder < right->createdOrder;
}

static void swapComplaints(struct ComplaintRecord* left, struct ComplaintRecord* right) {
    struct ComplaintRecord temp = *left;
    *left = *right;
    *right = temp;
}

static void siftUp(struct ComplaintHeap* heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (complaintComesFirst(&heap->items[parent], &heap->items[index])) {
            break;
        }

        swapComplaints(&heap->items[parent], &heap->items[index]);
        index = parent;
    }
}

static void siftDown(struct ComplaintHeap* heap, int index) {
    while (1) {
        int left = (index * 2) + 1;
        int right = left + 1;
        int best = index;

        if (left < heap->size && complaintComesFirst(&heap->items[left], &heap->items[best])) {
            best = left;
        }
        if (right < heap->size && complaintComesFirst(&heap->items[right], &heap->items[best])) {
            best = right;
        }
        if (best == index) {
            break;
        }

        swapComplaints(&heap->items[index], &heap->items[best]);
        index = best;
    }
}

static int ensureCapacity(struct ComplaintHeap* heap, int minimumCapacity) {
    struct ComplaintRecord* resized;
    int newCapacity = heap->capacity;

    if (newCapacity == 0) {
        newCapacity = INITIAL_COMPLAINT_CAPACITY;
    }

    while (newCapacity < minimumCapacity) {
        newCapacity *= 2;
    }

    resized = (struct ComplaintRecord*)realloc(heap->items, (size_t)newCapacity * sizeof(struct ComplaintRecord));
    if (resized == NULL) {
        return 0;
    }

    heap->items = resized;
    heap->capacity = newCapacity;
    return 1;
}

void initComplaintHeap(struct ComplaintHeap* heap) {
    heap->items = NULL;
    heap->size = 0;
    heap->capacity = 0;
    heap->nextId = 1;
    heap->nextOrder = 0;
}

void freeComplaintHeap(struct ComplaintHeap* heap) {
    free(heap->items);
    initComplaintHeap(heap);
}

int addComplaint(
    struct ComplaintHeap* heap,
    char block,
    char flatNo[],
    char type[],
    char description[],
    int priority,
    char status[]
) {
    struct ComplaintRecord* complaint;

    if (!ensureCapacity(heap, heap->size + 1)) {
        return 0;
    }

    complaint = &heap->items[heap->size];
    complaint->id = heap->nextId++;
    complaint->block = block;
    strncpy(complaint->flatNo, flatNo, sizeof(complaint->flatNo) - 1);
    complaint->flatNo[sizeof(complaint->flatNo) - 1] = '\0';
    strncpy(complaint->type, type, sizeof(complaint->type) - 1);
    complaint->type[sizeof(complaint->type) - 1] = '\0';
    strncpy(complaint->description, description, sizeof(complaint->description) - 1);
    complaint->description[sizeof(complaint->description) - 1] = '\0';
    complaint->priority = priority;
    strncpy(complaint->status, status, sizeof(complaint->status) - 1);
    complaint->status[sizeof(complaint->status) - 1] = '\0';
    complaint->createdOrder = heap->nextOrder++;

    heap->size++;
    siftUp(heap, heap->size - 1);
    return complaint->id;
}

int updateComplaintStatus(struct ComplaintHeap* heap, int id, char status[]) {
    int index;

    for (index = 0; index < heap->size; index++) {
        if (heap->items[index].id == id) {
            strncpy(heap->items[index].status, status, sizeof(heap->items[index].status) - 1);
            heap->items[index].status[sizeof(heap->items[index].status) - 1] = '\0';
            return 1;
        }
    }

    return 0;
}

int removeComplaintById(struct ComplaintHeap* heap, int id) {
    int index;

    for (index = 0; index < heap->size; index++) {
        if (heap->items[index].id == id) {
            heap->size--;
            if (index != heap->size) {
                heap->items[index] = heap->items[heap->size];
                if (index > 0 && !complaintComesFirst(&heap->items[(index - 1) / 2], &heap->items[index])) {
                    siftUp(heap, index);
                } else {
                    siftDown(heap, index);
                }
            }
            return 1;
        }
    }

    return 0;
}

int getComplaintCount(const struct ComplaintHeap* heap) {
    return heap->size;
}

int fillComplaintsByPriority(const struct ComplaintHeap* heap, struct ComplaintRecord* out, int maxItems) {
    int count;
    int writeIndex = 0;
    struct ComplaintHeap clone;

    if (maxItems <= 0 || heap->size == 0) {
        return 0;
    }

    count = heap->size < maxItems ? heap->size : maxItems;
    clone.size = heap->size;
    clone.capacity = heap->size;
    clone.nextId = heap->nextId;
    clone.nextOrder = heap->nextOrder;
    clone.items = (struct ComplaintRecord*)malloc((size_t)heap->size * sizeof(struct ComplaintRecord));
    if (clone.items == NULL) {
        return 0;
    }

    memcpy(clone.items, heap->items, (size_t)heap->size * sizeof(struct ComplaintRecord));

    while (clone.size > 0 && writeIndex < count) {
        out[writeIndex++] = clone.items[0];
        clone.size--;
        if (clone.size > 0) {
            clone.items[0] = clone.items[clone.size];
            siftDown(&clone, 0);
        }
    }

    free(clone.items);
    return writeIndex;
}

void saveComplaints(const struct ComplaintHeap* heap) {
    FILE* file = fopen("complaints.dat", "wb");
    struct ComplaintFileHeader header;

    if (file == NULL) {
        return;
    }

    header.size = heap->size;
    header.nextId = heap->nextId;
    header.nextOrder = heap->nextOrder;
    fwrite(&header, sizeof(header), 1, file);

    if (heap->size > 0) {
        fwrite(heap->items, sizeof(struct ComplaintRecord), (size_t)heap->size, file);
    }

    fclose(file);
}

void loadComplaints(struct ComplaintHeap* heap) {
    FILE* file = fopen("complaints.dat", "rb");
    struct ComplaintFileHeader header;
    int index;

    if (file == NULL) {
        return;
    }

    if (fread(&header, sizeof(header), 1, file) != 1 || header.size < 0) {
        fclose(file);
        return;
    }

    if (header.size > 0) {
        if (!ensureCapacity(heap, header.size)) {
            fclose(file);
            return;
        }

        if (fread(heap->items, sizeof(struct ComplaintRecord), (size_t)header.size, file) != (size_t)header.size) {
            heap->size = 0;
            fclose(file);
            return;
        }
    }

    heap->size = header.size;
    heap->nextId = header.nextId > 0 ? header.nextId : 1;
    heap->nextOrder = header.nextOrder >= 0 ? header.nextOrder : header.size;

    for (index = (heap->size / 2) - 1; index >= 0; index--) {
        siftDown(heap, index);
    }

    fclose(file);
}
