/* TODO : Setup MIT LICENSES ON ALL SCRIPTS */


#include <stdlib.h>
#include <string.h>
#include "table.h"


/*-- Memory Pool Api --*/

MemoryPool_t* memory_pool_init(MemoryPool_t* pool){
    if (pool == NULL){
        /* If NULL, give the pool some new memory to use... */
        MemoryPool_t* new_pool = (MemoryPool_t*)malloc(sizeof(MemoryPool_t));
        memset(new_pool, 0 , sizeof(MemoryPool_t));
        return new_pool;
    };

    memset(pool, 0 , sizeof(MemoryPool_t));
    return pool;
}

/*-- Default Memory Pool Api Functions --*/

/* TODO: (Vizonex) Optimize malloc and other function apis by 
looking into libc's own malloc function... */

void* pool_malloc(size_t size){
    return malloc(size);
}

void* pool_calloc(size_t size, size_t element_size){
    return calloc(element_size, size);
}

void* pool_realloc(void* ptr , size_t new_size){
    return realloc(ptr, new_size);
}

void pool_free(void* ptr){
    if (ptr != NULL)
        free(ptr);
}


void memory_pool_set_default_settings(MemoryPool_t* pool){
    pool->malloc = pool_malloc;
    pool->calloc = pool_calloc;
    pool->realloc = pool_realloc;
    pool->free = pool_free;
}

void memory_pool_free(MemoryPool_t* pool){
    if (pool != NULL)
        free(pool);
}

/* --- NODE API --- */

node_t *node_init(MemoryPool_t* pool, size_t default_cap, size_t expand_by){
    node_t* new_node;
    new_node = (node_t*)pool->malloc(sizeof(node_t));
    if (new_node == NULL){
        return NULL;
    }
    /* expand_by cannot be zero... */
    new_node->expand_by = expand_by == 0 ? default_cap: expand_by;

    new_node->data = (unsigned char*)pool->calloc(sizeof(unsigned char), default_cap);
    
    if (new_node->data == NULL){
        /* FALLBACK! */
        pool_free(new_node);
        return NULL;
    }

    /* record the current capacity to the new object just made for it... */
    new_node->capacity = default_cap;

    /* Set length to zero, Very important...*/
    new_node->length = 0;
    
    /* last but not least, pass off the currently used memory pool to make it easier to reach... */
    new_node->pool = pool;
    
    return new_node;
}


/* You might be wondering why I have 1 set to being the error , 
the truth is that I am working with the llparse library I had 
all translated over to python and one of it's checks says 
that 1 is an error , it's also a better way for me to say 

if (node_exapand(node, istr, isize)){
    raise error...
}

Hopefully that clears up why this is the case - Vizonex */

int node_expand(node_t* node, unsigned char* istr, size_t isize){
    size_t new_size = node->length + isize;
    if (new_size > node->capacity){
        node->data = node->pool->realloc(node->data, new_size + node->expand_by);
        if (node->data == NULL){
            return 1;
        }
        node->capacity = new_size + node->expand_by;
    };
    memcpy(node->data + node->length, istr, isize);
    node->length = new_size;
    return 0;
}

/* This is extremely simple allows for the currently memory 
being held to be used elsewhere */

void node_clear(node_t* node){
    node->length = 0; 
}

void node_free(node_t* node){
    /* we ask for the pool that is also used 
    by the table so it will be freed a bit later... */
    MemoryPool_t* pool;
    if (node != NULL){
        pool = node->pool;
        pool->free(node);
    }
}

/* --- TABLE API --- */

table_t * table_init(MemoryPool_t* pool , size_t table_len , size_t node_default_cap, size_t exapnsion_size){
    table_t* new_table = (table_t*)pool->malloc(sizeof(table_t));
    if (new_table == NULL)
        return NULL; 
    
    size_t last_allocated;
    
    /* Should double pointers be used here at all? */
    node_t** node_table = (node_t**)pool->calloc(sizeof(node_t), table_len);

    new_table->table_capacity = table_len;
    new_table->pool = pool;
    new_table->default_expansion_size = exapnsion_size;

    for (size_t i = 0;  i < table_len ; i++){
        node_table[i] = node_init(pool, node_default_cap, exapnsion_size);
        if (node_table[i] == NULL){
            last_allocated = i;
            goto FAILED;
        }
    }

    new_table->pool = pool;
    new_table->table = node_table;
    return new_table;


    FAILED:
        /* Free all objects used up and return NULL */
        for (size_t j = 0; j < last_allocated;  j++)
            pool->free(node_table[j]);
        pool->free(new_table);
        return NULL;


}

int table_node_append(table_t* table, size_t index, unsigned char* istr, size_t isize){
    return node_expand(table->table[index], istr, isize);
}

void table_free(table_t* table){
    MemoryPool_t * pool;
    if (table != NULL){
        pool = table->pool;
        for (size_t i = 0; i < table->table_capacity; i++){
            node_free(table->table[i]);
        }
        /* now free table... */
        pool->free(table);
    }
}


