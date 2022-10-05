#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ 256

struct Node{
    int start_time;
    int end_time;
    struct Node* next;
    char event_name[BUF_SZ];
};

void add_event(struct Node** head, char* event, int start_time, int end_time){
    
    
    struct Node* new_event = malloc(sizeof(struct Node));
            
            new_event->start_time = start_time;
            new_event->end_time = end_time;
            new_event->next = NULL;
            strcpy(new_event->event_name, event);

            // Checks start_time is less than end_time
            if(new_event->start_time < new_event->end_time && new_event->start_time >= 0 && new_event->end_time <= 23){
                // Creates the link list if first node
                if(*head == NULL){
                    *head = new_event;
                }
                // Adds Node as the new head if the new event's end time is < head's start_time
                else if(new_event->end_time < (*head)->start_time){
                    new_event->next = *head;
                    *head = new_event;
                }
                // For cases after first node
                else {
                    struct Node* temp = *head;
                    while(temp != NULL){
                        // Adds node to end of the linked list
                        if(new_event->start_time > temp->end_time && temp->next == NULL){
                            temp->next = new_event;
                            break;
                        }
                        // Adds nodes inbetween nodes i.e. temp -> new event -> (temp->next)
                        else if(new_event->start_time > temp->end_time && new_event->end_time < temp->next->start_time){
                            new_event->next = temp->next;
                            temp->next = new_event;
                            break;
                        }
                        temp = temp->next;
                    }
                    // If program fully traverses linked list and could not find spot, prints event error
                    if(temp == NULL)
                        printf("Event overlap error\n");
                }
            }
            else
                printf("Event start and end time error\n");
}

void delete_event(struct Node** head, char* event){

    // if head is empty
    if(*head == NULL){
        return;
    }

    struct Node* temp = *head;

    // if event is head
    if(strcmp((*head)->event_name, event) == 0){
        *head = (*head)->next;
        free(temp);
        return;
    }
    
    struct Node* deleted = NULL;

    while (temp->next != NULL){
        // if event is last node
        if(temp->next->next == NULL && strcmp(temp->next->event_name, event) == 0){
            deleted = temp->next;
            temp->next = NULL;
            free(deleted);
            return;
        }   
        // if event is middle node
        else if(temp->next->next != NULL && strcmp(temp->next->event_name, event) == 0){
            deleted = temp->next;
            temp->next = temp->next->next;
            free(deleted);
            return;
        }
        temp = temp->next;
    } 
}

void print_string(struct Node** head){
    
    struct Node* temp = *head;
    while (temp != NULL){
        printf("%s %d %d\n", temp->event_name, temp->start_time, temp->end_time);
        temp = temp->next;
    }
}

int main(int argc, char** argv){
    char command[BUF_SZ];

    struct Node* head = NULL;
    do{
        printf("$ ");
        scanf("%s", command);

        if(strcmp(command, "add") == 0){
            char event[BUF_SZ];
            int start, end;
            scanf("%s %d %d", event, &start, &end);
            add_event(&head, event, start, end);
        }
        else if(strcmp(command, "delete") == 0){
            char event[BUF_SZ];
            scanf("%s", event);
            delete_event(&head, event);
        }
        else if(strcmp(command, "printcalendar") == 0){
            print_string(&head);
        }
    } while(strcmp(command, "quit") != 0);

    while(head != NULL){
        struct Node* temp = NULL;
        temp = head;
        head = head->next;
        free(temp);
    }
}