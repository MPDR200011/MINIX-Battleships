#include "stdbool.h"
#include "stdint.h"

/** @defgroup List
 * @{
 * 
 * Implementation of a doubly linked list.
 * 
 */

/**
 * @brief List node.
 * 
 */
typedef struct list_node {
  void* data; /**< @brief Pointer to data held by node. */
  struct list_node* prev; /**< @brief Pointer to previous node in the list. */
  struct list_node* next; /**< @brief Pointer to next node in the list. */
} Node;

/**
 * @brief Struct representing a doubly linked list.
 * 
 */
typedef struct list_obj {
  uint32_t size; /**< @brief Size of the list. */
  Node* head; /**< @brief Pointer to the head of the list. */
} List;

/**
 * @brief Create a list
 * 
 * Initializes a List and returns the created list.
 * 
 * @return List The created list.
 */
List list_create();
/**
 * @brief Add a value to a list.
 * 
 * @param self Pointer to the list in which the element will be added.
 * @param val Pointer to the value being added.
 */
void list_add(List* self, void* val);
/**
 * @brief Remove a value from the list by index.
 * 
 * If index doesn't exist in the list, it does nothing.
 * 
 * @param self Pointer to the list from which the element will be removed.
 * @param index Index of the element to be removed.
 */
void list_remove(List* self, uint32_t index);
/**
 * @brief Clear a list.
 * 
 * @param self Pointer to the list to be cleared.
 */
void list_clear(List* self);
/**
 * @brief Get element from list.
 * 
 * Gets the value in the list at the provided index.
 * If the index doesn't exist, it returns 0.
 * 
 * @param self Pointer to the list from which the element is being retrieved.
 * @param index Index of the element.
 * @return void* Pointer to the element.
 */
void* list_get(List* self, uint32_t index);

/**@}*/
