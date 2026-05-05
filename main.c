#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "headers/file_io.h"
#include "headers/house.h"
#include "headers/payment.h"
#include "headers/resident.h"

#define SERVER_PORT 8080
#define REQUEST_BUFFER_SIZE 32768

struct ResidentNode* gResidentRoot = NULL;
struct PaymentNode* gPaymentHead = NULL;
struct PaymentNode* gPaymentTail = NULL;

struct StringBuilder {
    char* data;
    size_t length;
    size_t capacity;
};

static void sbInit(struct StringBuilder* sb) {
    sb->capacity = 1024;
    sb->length = 0;
    sb->data = (char*)malloc(sb->capacity);
    if (sb->data != NULL) {
        sb->data[0] = '\0';
    }
}

static void sbEnsure(struct StringBuilder* sb, size_t extra) {
    while (sb->length + extra + 1 > sb->capacity) {
        sb->capacity *= 2;
        sb->data = (char*)realloc(sb->data, sb->capacity);
    }
}

static void sbAppend(struct StringBuilder* sb, const char* text) {
    size_t textLength = strlen(text);
    sbEnsure(sb, textLength);
    memcpy(sb->data + sb->length, text, textLength + 1);
    sb->length += textLength;
}

static void sbAppendFormat(struct StringBuilder* sb, const char* format, ...) {
    va_list args;
    va_list argsCopy;
    int required;

    va_start(args, format);
    va_copy(argsCopy, args);
    required = vsnprintf(NULL, 0, format, argsCopy);
    va_end(argsCopy);

    if (required < 0) {
        va_end(args);
        return;
    }

    sbEnsure(sb, (size_t)required);
    vsnprintf(sb->data + sb->length, sb->capacity - sb->length, format, args);
    sb->length += (size_t)required;
    va_end(args);
}

static void sbAppendJsonString(struct StringBuilder* sb, const char* text) {
    size_t i;

    sbAppend(sb, "\"");
    for (i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c == '\\' || c == '"') {
            char escaped[3];
            escaped[0] = '\\';
            escaped[1] = c;
            escaped[2] = '\0';
            sbAppend(sb, escaped);
        } else if (c == '\n') {
            sbAppend(sb, "\\n");
        } else if (c == '\r') {
            sbAppend(sb, "\\r");
        } else if (c == '\t') {
            sbAppend(sb, "\\t");
        } else {
            char plain[2];
            plain[0] = c;
            plain[1] = '\0';
            sbAppend(sb, plain);
        }
    }
    sbAppend(sb, "\"");
}

static void sbFree(struct StringBuilder* sb) {
    free(sb->data);
    sb->data = NULL;
    sb->length = 0;
    sb->capacity = 0;
}

static int countFlats(void) {
    int count = 0;
    int index;

    for (index = 0; index < 5; index++) {
        struct Flat* current = communityBlocks[index];
        while (current != NULL) {
            count++;
            current = current->next;
        }
    }
    return count;
}

static int countOccupiedFlats(void) {
    int count = 0;
    int index;

    for (index = 0; index < 5; index++) {
        struct Flat* current = communityBlocks[index];
        while (current != NULL) {
            if (current->status == 1) {
                count++;
            }
            current = current->next;
        }
    }
    return count;
}

static int countResidents(struct ResidentNode* root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + countResidents(root->left) + countResidents(root->right);
}

static int countPendingPayments(struct PaymentNode* head) {
    int count = 0;
    struct PaymentNode* current = head;

    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

static double sumPendingPayments(struct PaymentNode* head) {
    double total = 0.0;
    struct PaymentNode* current = head;

    while (current != NULL) {
        total += current->amountDue;
        current = current->next;
    }
    return total;
}

static void appendFlatsJson(struct StringBuilder* sb) {
    int index;
    int first = 1;

    sbAppend(sb, "[");
    for (index = 0; index < 5; index++) {
        struct Flat* current = communityBlocks[index];
        while (current != NULL) {
            if (!first) {
                sbAppend(sb, ",");
            }

            sbAppend(sb, "{\"block\":");
            {
                char blockText[2];
                blockText[0] = current->block;
                blockText[1] = '\0';
                sbAppendJsonString(sb, blockText);
            }
            sbAppend(sb, ",\"flatNo\":");
            sbAppendJsonString(sb, current->flatNo);
            sbAppendFormat(sb, ",\"bhk\":%d,\"status\":%d}", current->bhk, current->status);

            first = 0;
            current = current->next;
        }
    }
    sbAppend(sb, "]");
}

static void appendResidentsJsonNode(struct ResidentNode* root, struct StringBuilder* sb, int* first) {
    if (root == NULL) {
        return;
    }

    appendResidentsJsonNode(root->left, sb, first);
    if (!*first) {
        sbAppend(sb, ",");
    }

    sbAppend(sb, "{\"block\":");
    {
        char blockText[2];
        blockText[0] = root->block;
        blockText[1] = '\0';
        sbAppendJsonString(sb, blockText);
    }
    sbAppend(sb, ",\"flatNo\":");
    sbAppendJsonString(sb, root->flatNo);
    sbAppend(sb, ",\"name\":");
    sbAppendJsonString(sb, root->info.name);
    sbAppend(sb, ",\"phone\":");
    sbAppendJsonString(sb, root->info.phone);
    sbAppend(sb, "}");

    *first = 0;
    appendResidentsJsonNode(root->right, sb, first);
}

static void appendResidentsJson(struct StringBuilder* sb) {
    int first = 1;

    sbAppend(sb, "[");
    appendResidentsJsonNode(gResidentRoot, sb, &first);
    sbAppend(sb, "]");
}

static void appendPaymentsJson(struct StringBuilder* sb) {
    int first = 1;
    struct PaymentNode* current = gPaymentHead;

    sbAppend(sb, "[");
    while (current != NULL) {
        if (!first) {
            sbAppend(sb, ",");
        }

        sbAppend(sb, "{\"block\":");
        {
            char blockText[2];
            blockText[0] = current->block;
            blockText[1] = '\0';
            sbAppendJsonString(sb, blockText);
        }
        sbAppend(sb, ",\"flatNo\":");
        sbAppendJsonString(sb, current->flatNo);
        sbAppendFormat(sb, ",\"amountDue\":%.2f}", current->amountDue);

        first = 0;
        current = current->next;
    }
    sbAppend(sb, "]");
}

static char* buildStateJson(void) {
    struct StringBuilder sb;
    int totalFlats = countFlats();
    int occupiedFlats = countOccupiedFlats();
    int residentCount = countResidents(gResidentRoot);
    int paymentCount = countPendingPayments(gPaymentHead);
    double pendingAmount = sumPendingPayments(gPaymentHead);

    sbInit(&sb);
    sbAppend(&sb, "{\"summary\":{");
    sbAppendFormat(
        &sb,
        "\"flats\":%d,\"occupied\":%d,\"available\":%d,\"residents\":%d,\"pendingPayments\":%d,\"pendingAmount\":%.2f",
        totalFlats,
        occupiedFlats,
        totalFlats - occupiedFlats,
        residentCount,
        paymentCount,
        pendingAmount
    );
    sbAppend(&sb, "},\"flats\":");
    appendFlatsJson(&sb);
    sbAppend(&sb, ",\"residents\":");
    appendResidentsJson(&sb);
    sbAppend(&sb, ",\"payments\":");
    appendPaymentsJson(&sb);
    sbAppend(&sb, "}");

    return sb.data;
}

static void resetFlatStatuses(void) {
    int index;
    for (index = 0; index < 5; index++) {
        struct Flat* current = communityBlocks[index];
        while (current != NULL) {
            current->status = 0;
            current = current->next;
        }
    }
}

static void markOccupiedFlats(struct ResidentNode* root) {
    struct Flat* flat;

    if (root == NULL) {
        return;
    }

    markOccupiedFlats(root->left);
    flat = findFlat(root->block, root->flatNo);
    if (flat != NULL) {
        flat->status = 1;
    }
    markOccupiedFlats(root->right);
}

static void syncFlatOccupancy(void) {
    resetFlatStatuses();
    markOccupiedFlats(gResidentRoot);
    saveFlats();
}

static char hexValue(char c) {
    if (c >= '0' && c <= '9') {
        return (char)(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return (char)(10 + c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return (char)(10 + c - 'A');
    }
    return 0;
}

static void urlDecode(const char* source, char* dest, size_t destSize) {
    size_t readIndex = 0;
    size_t writeIndex = 0;

    while (source[readIndex] != '\0' && writeIndex + 1 < destSize) {
        if (source[readIndex] == '%' &&
            isxdigit((unsigned char)source[readIndex + 1]) &&
            isxdigit((unsigned char)source[readIndex + 2])) {
            dest[writeIndex++] = (char)((hexValue(source[readIndex + 1]) << 4) | hexValue(source[readIndex + 2]));
            readIndex += 3;
        } else if (source[readIndex] == '+') {
            dest[writeIndex++] = ' ';
            readIndex++;
        } else {
            dest[writeIndex++] = source[readIndex++];
        }
    }

    dest[writeIndex] = '\0';
}

static int getParamValue(const char* payload, const char* key, char* out, size_t outSize) {
    size_t keyLength = strlen(key);
    const char* current = payload;

    while (current != NULL && *current != '\0') {
        const char* keyStart = current;
        const char* valueStart;
        const char* valueEnd;
        size_t rawLength;
        char rawValue[512];

        valueStart = strchr(keyStart, '=');
        if (valueStart == NULL) {
            return 0;
        }

        if ((size_t)(valueStart - keyStart) == keyLength && strncmp(keyStart, key, keyLength) == 0) {
            valueStart++;
            valueEnd = strchr(valueStart, '&');
            rawLength = (valueEnd == NULL) ? strlen(valueStart) : (size_t)(valueEnd - valueStart);
            if (rawLength >= sizeof(rawValue)) {
                rawLength = sizeof(rawValue) - 1;
            }

            memcpy(rawValue, valueStart, rawLength);
            rawValue[rawLength] = '\0';
            urlDecode(rawValue, out, outSize);
            return 1;
        }

        current = strchr(valueStart, '&');
        if (current != NULL) {
            current++;
        }
    }

    return 0;
}

static void sendAll(SOCKET clientSocket, const char* data, int length) {
    int totalSent = 0;

    while (totalSent < length) {
        int sent = send(clientSocket, data + totalSent, length - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            return;
        }
        totalSent += sent;
    }
}

static void sendResponse(
    SOCKET clientSocket,
    const char* status,
    const char* contentType,
    const char* body,
    int bodyLength
) {
    char header[512];
    int headerLength = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n",
        status,
        contentType,
        bodyLength
    );

    sendAll(clientSocket, header, headerLength);
    sendAll(clientSocket, body, bodyLength);
}

static void sendJsonMessage(SOCKET clientSocket, const char* status, int success, const char* message) {
    struct StringBuilder sb;

    sbInit(&sb);
    sbAppend(&sb, "{\"success\":");
    sbAppend(&sb, success ? "true" : "false");
    sbAppend(&sb, ",\"message\":");
    sbAppendJsonString(&sb, message);
    sbAppend(&sb, "}");

    sendResponse(clientSocket, status, "application/json; charset=utf-8", sb.data, (int)sb.length);
    sbFree(&sb);
}

static void serveStaticFile(SOCKET clientSocket, const char* path) {
    const char* mappedPath = NULL;
    const char* contentType = "text/plain; charset=utf-8";
    FILE* file;
    long fileSize;
    char* content;

    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        mappedPath = "frontend/index.html";
        contentType = "text/html; charset=utf-8";
    } else if (strcmp(path, "/styles.css") == 0) {
        mappedPath = "frontend/styles.css";
        contentType = "text/css; charset=utf-8";
    } else if (strcmp(path, "/app.js") == 0) {
        mappedPath = "frontend/app.js";
        contentType = "application/javascript; charset=utf-8";
    }

    if (mappedPath == NULL) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "Route not found.");
        return;
    }

    file = fopen(mappedPath, "rb");
    if (file == NULL) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "Static asset is missing.");
        return;
    }

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    rewind(file);

    content = (char*)malloc((size_t)fileSize);
    if (content == NULL) {
        fclose(file);
        sendJsonMessage(clientSocket, "500 Internal Server Error", 0, "Could not allocate memory for asset.");
        return;
    }

    fread(content, 1, (size_t)fileSize, file);
    fclose(file);

    sendResponse(clientSocket, "200 OK", contentType, content, (int)fileSize);
    free(content);
}

static int parseRequest(
    SOCKET clientSocket,
    char* method,
    size_t methodSize,
    char* target,
    size_t targetSize,
    char* body,
    size_t bodySize
) {
    char buffer[REQUEST_BUFFER_SIZE];
    int totalRead = 0;
    int contentLength = 0;
    char* headersEnd;
    char* contentLengthHeader;
    int headerLength;
    int bodyBytesAlreadyRead;

    while (totalRead < (int)(sizeof(buffer) - 1)) {
        int received = recv(clientSocket, buffer + totalRead, (int)(sizeof(buffer) - 1 - totalRead), 0);
        if (received <= 0) {
            return 0;
        }
        totalRead += received;
        buffer[totalRead] = '\0';
        headersEnd = strstr(buffer, "\r\n\r\n");
        if (headersEnd != NULL) {
            break;
        }
    }

    headersEnd = strstr(buffer, "\r\n\r\n");
    if (headersEnd == NULL) {
        return 0;
    }

    if (sscanf(buffer, "%15s %255s", method, target) != 2) {
        return 0;
    }

    contentLengthHeader = strstr(buffer, "Content-Length:");
    if (contentLengthHeader != NULL) {
        sscanf(contentLengthHeader, "Content-Length: %d", &contentLength);
    }

    headerLength = (int)((headersEnd + 4) - buffer);
    bodyBytesAlreadyRead = totalRead - headerLength;
    if ((size_t)contentLength >= bodySize) {
        contentLength = (int)bodySize - 1;
    }

    if (bodyBytesAlreadyRead > 0) {
        memcpy(body, buffer + headerLength, (size_t)bodyBytesAlreadyRead);
    }

    while (bodyBytesAlreadyRead < contentLength) {
        int received = recv(clientSocket, body + bodyBytesAlreadyRead, contentLength - bodyBytesAlreadyRead, 0);
        if (received <= 0) {
            return 0;
        }
        bodyBytesAlreadyRead += received;
    }

    body[bodyBytesAlreadyRead] = '\0';
    return 1;
}

static void handleGetState(SOCKET clientSocket) {
    char* json = buildStateJson();
    sendResponse(clientSocket, "200 OK", "application/json; charset=utf-8", json, (int)strlen(json));
    free(json);
}

static void handleAddFlat(SOCKET clientSocket, const char* body) {
    char blockText[8];
    char flatNo[32];
    char bhkText[16];
    char block;
    int bhk;

    if (!getParamValue(body, "block", blockText, sizeof(blockText)) ||
        !getParamValue(body, "flatNo", flatNo, sizeof(flatNo)) ||
        !getParamValue(body, "bhk", bhkText, sizeof(bhkText))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Block, flat number, and BHK are required.");
        return;
    }

    block = (char)toupper((unsigned char)blockText[0]);
    bhk = atoi(bhkText);

    if (block < 'A' || block > 'E') {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Block must be between A and E.");
        return;
    }
    if (bhk < 1 || bhk > 3) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "BHK must be 1, 2, or 3.");
        return;
    }
    if (doesFlatExist(block, flatNo)) {
        sendJsonMessage(clientSocket, "409 Conflict", 0, "That flat already exists.");
        return;
    }

    addFlat(flatNo, block, bhk);
    sendJsonMessage(clientSocket, "200 OK", 1, "Flat added successfully.");
}

static void handleAddResident(SOCKET clientSocket, const char* body) {
    char blockText[8];
    char flatNo[32];
    char name[64];
    char phone[32];
    char block;
    struct Flat* flat;

    if (!getParamValue(body, "block", blockText, sizeof(blockText)) ||
        !getParamValue(body, "flatNo", flatNo, sizeof(flatNo)) ||
        !getParamValue(body, "name", name, sizeof(name)) ||
        !getParamValue(body, "phone", phone, sizeof(phone))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Resident details are incomplete.");
        return;
    }

    block = (char)toupper((unsigned char)blockText[0]);
    flat = findFlat(block, flatNo);

    if (flat == NULL) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "Create the flat before assigning a resident.");
        return;
    }
    if (searchResident(gResidentRoot, block, flatNo) != NULL) {
        sendJsonMessage(clientSocket, "409 Conflict", 0, "That flat already has a resident.");
        return;
    }

    gResidentRoot = insertResident(gResidentRoot, block, flatNo, name, phone);
    appendTransaction(block, flatNo, name, phone);
    updateFlatStatus(block, flatNo, 1);
    sendJsonMessage(clientSocket, "200 OK", 1, "Resident added successfully.");
}

static void handleDeleteResident(SOCKET clientSocket, const char* body) {
    char blockText[8];
    char flatNo[32];
    char block;

    if (!getParamValue(body, "block", blockText, sizeof(blockText)) ||
        !getParamValue(body, "flatNo", flatNo, sizeof(flatNo))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Block and flat number are required.");
        return;
    }

    block = (char)toupper((unsigned char)blockText[0]);
    if (searchResident(gResidentRoot, block, flatNo) == NULL) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "No resident was found for that flat.");
        return;
    }

    gResidentRoot = deleteResident(gResidentRoot, block, flatNo);
    appendTransaction(block, flatNo, "DELETED", "");
    updateFlatStatus(block, flatNo, 0);
    sendJsonMessage(clientSocket, "200 OK", 1, "Resident removed successfully.");
}

static void handleAddPayment(SOCKET clientSocket, const char* body) {
    char blockText[8];
    char flatNo[32];
    char amountText[32];
    char block;
    float amount;

    if (!getParamValue(body, "block", blockText, sizeof(blockText)) ||
        !getParamValue(body, "flatNo", flatNo, sizeof(flatNo)) ||
        !getParamValue(body, "amount", amountText, sizeof(amountText))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Payment details are incomplete.");
        return;
    }

    block = (char)toupper((unsigned char)blockText[0]);
    amount = (float)atof(amountText);

    if (searchResident(gResidentRoot, block, flatNo) == NULL) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "Only occupied flats can receive bills.");
        return;
    }
    if (amount <= 0.0f) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Amount must be greater than zero.");
        return;
    }
    if (hasPendingPayment(gPaymentHead, block, flatNo)) {
        sendJsonMessage(clientSocket, "409 Conflict", 0, "There is already a pending payment for that flat.");
        return;
    }

    addPendingBill(&gPaymentHead, &gPaymentTail, block, flatNo, amount);
    savePayments(gPaymentHead);
    sendJsonMessage(clientSocket, "200 OK", 1, "Pending payment added successfully.");
}

static void handleProcessPayment(SOCKET clientSocket, const char* body) {
    char blockText[8];
    char flatNo[32];
    char block;

    if (!getParamValue(body, "block", blockText, sizeof(blockText)) ||
        !getParamValue(body, "flatNo", flatNo, sizeof(flatNo))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Block and flat number are required.");
        return;
    }

    block = (char)toupper((unsigned char)blockText[0]);
    if (!processPayment(&gPaymentHead, &gPaymentTail, block, flatNo)) {
        sendJsonMessage(clientSocket, "404 Not Found", 0, "No pending payment was found for that flat.");
        return;
    }

    savePayments(gPaymentHead);
    sendJsonMessage(clientSocket, "200 OK", 1, "Payment processed successfully.");
}

static void handleApiRoute(SOCKET clientSocket, const char* method, const char* path, const char* body) {
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/state") == 0) {
        handleGetState(clientSocket);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/flats/add") == 0) {
        handleAddFlat(clientSocket, body);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/residents/add") == 0) {
        handleAddResident(clientSocket, body);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/residents/delete") == 0) {
        handleDeleteResident(clientSocket, body);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/payments/add") == 0) {
        handleAddPayment(clientSocket, body);
        return;
    }

    if (strcmp(method, "POST") == 0 && strcmp(path, "/api/payments/process") == 0) {
        handleProcessPayment(clientSocket, body);
        return;
    }

    sendJsonMessage(clientSocket, "404 Not Found", 0, "API route not found.");
}

static void handleClient(SOCKET clientSocket) {
    char method[16];
    char target[256];
    char body[16384];
    char path[256];
    char* queryStart;

    if (!parseRequest(clientSocket, method, sizeof(method), target, sizeof(target), body, sizeof(body))) {
        sendJsonMessage(clientSocket, "400 Bad Request", 0, "Could not parse the incoming request.");
        return;
    }

    strncpy(path, target, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    queryStart = strchr(path, '?');
    if (queryStart != NULL) {
        *queryStart = '\0';
    }

    if (strncmp(path, "/api/", 5) == 0) {
        handleApiRoute(clientSocket, method, path, body);
    } else {
        serveStaticFile(clientSocket, path);
    }
}

int main(void) {
    WSADATA wsaData;
    SOCKET serverSocket;
    struct sockaddr_in serverAddress;

    setupBlocks();
    loadFlats();
    gResidentRoot = loadAllData();
    loadPayments(&gPaymentHead, &gPaymentTail);
    syncFlatOccupancy();

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Failed to create server socket.\n");
        WSACleanup();
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Failed to bind server to port %d.\n", SERVER_PORT);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        printf("Failed to start listening on port %d.\n", SERVER_PORT);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Community management web app is running at http://localhost:%d\n", SERVER_PORT);
    printf("Press Ctrl+C to stop the server.\n");

    while (1) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        handleClient(clientSocket);
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
