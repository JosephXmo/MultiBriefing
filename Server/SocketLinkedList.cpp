#include "SocketLinkedList.hpp"

MBClientNode* createClientList(MBClient* mbClient) {
    MBClientNode* newNode = (MBClientNode*)malloc(sizeof(MBClientNode));

    if (!newNode) return NULL;

    newNode->mbClient = mbClient;
    newNode->next = NULL;

    return newNode;
}

MBClientNode* appendClient(MBClientNode* head, MBClientNode* newNode) {
    if (!head) return newNode;

    MBClientNode* current = head;
    while (current->next) {
        current = current->next;
    }
    current->next = newNode;

    return head;
}

MBClientNode* dequeue(MBClientNode* head, MBClient* target) {
    if (!head) return NULL;

    if (head->mbClient == target) {
        if (head->next == NULL) {
            free(head);

            return NULL;
        }

        MBClientNode* tempNode = head;
        head = head->next;
        tempNode->next = NULL;

        return tempNode;
    }

    MBClientNode* probe = head;
    while (probe->next != NULL) {
        MBClientNode* toProbe = NULL;
        // Search for target MBClient
        if (probe->mbClient != target) {
            toProbe = probe;
            probe = probe->next;
            continue;
        }
        // Found target MBClient. Stitch, pop, delete
        else {
            toProbe->next = probe->next;        // Stitch
            probe->next = NULL;                 // Pop
            free(probe);                        // Delete
        }
    }

    return head;
}