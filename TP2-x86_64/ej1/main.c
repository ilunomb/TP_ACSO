#include "ej1.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>

#if USE_ASM_IMPL
    #define CREATE_LIST string_proc_list_create_asm
    #define CREATE_NODE string_proc_node_create_asm
    #define ADD_NODE string_proc_list_add_node_asm
    #define CONCAT string_proc_list_concat_asm
#else
    #define CREATE_LIST string_proc_list_create
    #define CREATE_NODE string_proc_node_create
    #define ADD_NODE string_proc_list_add_node
    #define CONCAT string_proc_list_concat
#endif

/**
*	crea y destruye una lista vacía
*/
void test_create_destroy_list(){
	string_proc_list * list	= CREATE_LIST();
	if (list == NULL || list->first != NULL || list->last != NULL) {
		printf("test_create_destroy_list: ERROR\n");
	} else {
		printf("test_create_destroy_list: OK\n");
	}
	string_proc_list_destroy(list);
}

/**
*	crea y destruye un nodo
*/
void test_create_destroy_node(){
	string_proc_node* node = CREATE_NODE(42, "hash_example");
	if (node == NULL || node->type != 42 || strcmp(node->hash, "hash_example") != 0) {
		printf("test_create_destroy_node: ERROR\n");
	} else {
		printf("test_create_destroy_node: OK\n");
	}
	string_proc_node_destroy(node);
}

/**
 * 	crea una lista y le agrega nodos
*/
void test_create_list_add_nodes(){	
	string_proc_list * list	= CREATE_LIST();
	ADD_NODE(list, 0, "hola");
	ADD_NODE(list, 1, "a");
	ADD_NODE(list, 0, "todos!");
	
	string_proc_node* current = list->first;
	int count = 0;
	while (current != NULL) {
		count++;
		current = current->next;
	}
	if (count == 3 && list->last != NULL && strcmp(list->last->hash, "todos!") == 0) {
		printf("test_create_list_add_nodes: OK\n");
	} else {
		printf("test_create_list_add_nodes: ERROR\n");
	}

	string_proc_list_destroy(list);
}

/**
 * 	crea una lista, agrega nodos y concatena
*/
void test_list_concat(){
	string_proc_list * list = CREATE_LIST();
	ADD_NODE(list, 1, "uno");
	ADD_NODE(list, 1, "dos");
	ADD_NODE(list, 2, "tres");

	char* result = CONCAT(list, 1, "inicio-");

	if (result != NULL && strstr(result, "inicio-") && strstr(result, "uno") && strstr(result, "dos") && !strstr(result, "tres")) {
		printf("test_list_concat: OK\n");
	} else {
		printf("test_list_concat: ERROR\n");
	}
	
	free(result);
	string_proc_list_destroy(list);
}

/**
* Corre los tests escritos por lxs alumnxs	
*/
void run_tests(){
	test_create_destroy_list();
	test_create_destroy_node();
	test_create_list_add_nodes();
	test_list_concat();
}

int main(void){
	run_tests();
	return 0;
}
