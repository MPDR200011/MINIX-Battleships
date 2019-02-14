#include "List.h"
#include "stdlib.h"

List list_create() {
  List list;
  list.size = 0;
  list.head = 0;
  return list;
}

void list_add(List* self, void* val) {
  Node* temp = self->head;

  Node* newNode = malloc(sizeof(Node));
  newNode->data = val;
  newNode->next = NULL;
  newNode->prev = NULL;

  if (self->head == NULL) {
    self->head = newNode;
    self->size++;
    return;
  }

  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = newNode;
  newNode->prev = temp;
  self->size++;
}

void list_remove(List* self, uint32_t index) {
  Node* curr = self->head;
  if (curr == NULL) {
    return;
  }

  uint currIndex = 0;
  while (currIndex != index) {
    currIndex++;
    curr = curr->next;
    if (curr == NULL) {
      return;
    }
  }

  Node* prev = curr->prev;
  Node* next = curr->next;
  if (prev == NULL) {
    self->head = next;
    if (next != NULL) {
      next->prev = NULL;
    }
  } else if (next == NULL) {
    prev->next = NULL;
  } else {
    prev->next = next;
    next->prev = prev;
  }

  self->size -= 1;
  free(curr->data);
  free(curr);
}

void list_clear(List* self) {
  while (self->size > 0) {
    list_remove(self, 0);
  }
}

void* list_get(List* self, uint32_t index) {
  Node* curr = self->head;
  if (curr == 0) {
    return 0;
  }

  uint32_t currIndex = 0;
  while (currIndex != index) {
    curr = curr->next;
    if (curr == 0) {
      return 0;
    }
    currIndex++;
  }

  return curr->data;
}

