/**
 * @file stack.c
 * @author Abdelhakim RAFIK
 * 
 * @version 1.0.1
 * @date 2021-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "../include/fc_stack.h"

/**
 * Push given content to given stack
 * 
 * @param stack 
 * @param content
 * @return uint8_t
 */
uint8_t fc_stack_push(fc_stack_t **stack, void *content) {
	// create stack node
	fc_stack_node_t *node = (fc_stack_node_t*) malloc(sizeof(fc_stack_node_t));
	// fill the node
	if(node) {
		node->content = content;
		node->next = *stack;
		*stack = node;
		return 1;
	}
	return 0;
}

/**
 * Return and remove stack head content
 * 
 * @param stack 
 * @return void* 
 */
void* fc_stack_pop(fc_stack_t **stack) {
	// when empty stack
	if(!(*stack))
		return NULL;
	// get head node
	fc_stack_node_t *node = *stack;
	// point stack to next node
	*stack = node->next;
	// get node content
	void *content = node->content;
	// free node memory
	free(node);
	// return node content
	return content;

}

/**
 * Return stack head content
 * 
 * @param stack 
 * @return void* 
 */
void* fc_stack_peek(fc_stack_t *stack) {
	return stack ? stack->content : NULL;
}

/**
 * Check if stack is empty
 * 
 * @param stack 
 * @return uint8_t 
 */
uint8_t fc_stack_is_empty(fc_stack_t *stack) {
	return stack == NULL ? 1 : 0;
}