#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

typedef struct Node {
    char *ruleIdentifier;
    char *productions;
    struct Node *next;
} Node;

Node* createNode(const char *ruleIdentifier, const char *production);
void appendNode(Node **head, const char *ruleIdentifier, const char *production);
void freeLinkedList(Node *head);
Node* createLinkedList(FILE *file);
void printList(Node *head);

void splitLine(const char *line, char *ruleIdentifier, char *production);
Node* findNode(Node *head, const char *ruleIdentifier);
void appendProduction(Node *node, const char *production);
void appendOrUpdateNode(Node **head, const char *ruleIdentifier, const char *ruleProduction);

void removeRecursion(Node *head);
void removeSecondRecursion(Node *head);
void replaceBracesInFirstNode(Node *head);

int main() {
    char filename[MAX_LINE_LENGTH];
    printf("Ingrese el nombre del archivo de gramática: ");
    scanf("%s", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    Node *head = createLinkedList(file);
    fclose(file);

    printf("Lista de producciones:\n");
    printList(head);

    printf("\nEliminando recursividad:\n");
    removeRecursion(head);

    printf("\nEliminando segunda recursividad:\n");
    removeSecondRecursion(head);

    printf("\nReemplazando llaves en la primera producción:\n");
    replaceBracesInFirstNode(head);

    freeLinkedList(head);
    return 0;
}

Node* createNode(const char *ruleIdentifier, const char *production) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->ruleIdentifier = strdup(ruleIdentifier);
    newNode->productions = strdup(production);
    newNode->next = NULL;
    return newNode;
}

Node* findNode(Node *head, const char *ruleIdentifier) {
    Node *current = head;
    while (current != NULL) {
        if (strcmp(current->ruleIdentifier, ruleIdentifier) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void appendProduction(Node *node, const char *production) {
    size_t newSize = strlen(node->productions) + strlen(production) + 4;
    node->productions = (char *)realloc(node->productions, newSize);
    strcat(node->productions, " | ");
    strcat(node->productions, production);
}

void appendOrUpdateNode(Node **head, const char *ruleIdentifier, const char *production) {
    Node *existingNode = findNode(*head, ruleIdentifier);
    if (existingNode != NULL) {
        appendProduction(existingNode, production);
    } else {
        Node *newNode = createNode(ruleIdentifier, production);
        if (*head == NULL) {
            *head = newNode;
        } else {
            Node *temp = *head;
            while (temp->next != NULL) {
                temp = temp->next;
            }
            temp->next = newNode;
        }
    }
}

void freeLinkedList(Node *head) {
    Node *current = head;
    Node *nextNode;
    while (current != NULL) {
        nextNode = current->next;
        free(current->ruleIdentifier);
        free(current->productions);
        free(current);
        current = nextNode;
    }
}

void splitLine(const char *line, char *ruleIdentifier, char *production) {
    const char *delimiter = strstr(line, "->");
    if (delimiter != NULL) {
        strncpy(ruleIdentifier, line, delimiter - line);
        ruleIdentifier[delimiter - line] = '\0';
        strcpy(production, delimiter + 2);
    }
}

Node* createLinkedList(FILE *file) {
    Node *head = NULL;
    char line[MAX_LINE_LENGTH];
    char ruleIdentifier[MAX_LINE_LENGTH];
    char production[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        splitLine(line, ruleIdentifier, production);
        appendOrUpdateNode(&head, ruleIdentifier, production);
    }
    return head;
}

void printList(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("%s -> %s\n", current->ruleIdentifier, current->productions);
        current = current->next;
    }
}

void removeRecursion(Node *head) {
    Node *current = head;
    while (current != NULL) {
        char *recursiveProd = strstr(current->productions, current->ruleIdentifier);
        if (recursiveProd) {
            char auxiliaryRecProd[MAX_LINE_LENGTH];
            auxiliaryRecProd[0] = '{';
            auxiliaryRecProd[1] = *(recursiveProd - 1);
            auxiliaryRecProd[2] = '}';
            auxiliaryRecProd[3] = '\0';

            char *otherAuxProduction = strchr(current->productions, '|');
            if (otherAuxProduction) {
                otherAuxProduction++;
                char otherProduction[MAX_LINE_LENGTH];
                strcpy(otherProduction, otherAuxProduction);
                strcat(auxiliaryRecProd, otherProduction);
            }

            free(current->productions);
            current->productions = strdup(auxiliaryRecProd);
        }
        printf("%s -> %s\n", current->ruleIdentifier, current->productions);
        current = current->next;
    }
}

void removeSecondRecursion(Node *head) {
    if (head == NULL || head->next == NULL)
        return;

    Node *current = head;
    Node *previous = NULL;
    while (current->next != NULL) {
        previous = current;
        current = current->next;
    }

    Node *temp = head;
    while (temp != NULL) {
        char *tempProductions = temp->productions;
        for (int i = 0; i < strlen(tempProductions); i++) {
            if (tempProductions[i] == *current->ruleIdentifier) {
                char newProductions[MAX_LINE_LENGTH] = "";
                strncat(newProductions, tempProductions, i);
                strcat(newProductions, current->productions);
                strcat(newProductions, tempProductions + i + 1);
                free(temp->productions);
                temp->productions = strdup(newProductions);
                i = -1;
            }
        }
        temp = temp->next;
    }

    char *currentProductions = current->productions;
    char *previousProductions = previous->productions;

    for (int i = 0; i < strlen(previousProductions); i++) {
        if (previousProductions[i] == *current->ruleIdentifier) {
            char newProductions[MAX_LINE_LENGTH] = "";
            strncat(newProductions, previousProductions, i);
            strcat(newProductions, currentProductions);
            strcat(newProductions, previousProductions + i + 1);
            free(previous->productions);
            previous->productions = strdup(newProductions);
            i = -1;
        }
    }

    Node *tempPrint = head;
    while (tempPrint != NULL) {
        printf("%s -> %s\n", tempPrint->ruleIdentifier, tempPrint->productions);
        tempPrint = tempPrint->next;
    }
}

void replaceBracesInFirstNode(Node *head) {
    if (head == NULL)
        return;

    char *firstProductions = head->productions;
    char modifiedProductions[MAX_LINE_LENGTH] = "";
    int j = 0;

    for (int i = 0; i < strlen(firstProductions); i++) {
        if (firstProductions[i] == '{') {
            i++;
            if (firstProductions[i] != '\0') {
                modifiedProductions[j++] = firstProductions[i];
                modifiedProductions[j++] = '*';
            }
        } else {
            modifiedProductions[j++] = firstProductions[i];
        }
    }
    modifiedProductions[j] = '\0';
    free(head->productions);
    head->productions = strdup(modifiedProductions);
    printf("\n%s -> %s\n", head->ruleIdentifier, head->productions);
}

