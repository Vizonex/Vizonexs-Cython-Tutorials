/* TODO : Setup MIT LICENSES ON ALL SCRIPTS */

#ifndef TABLE_H
#define TABLE_H


/* These structures are based off of how yyjson and C-Algorythms are put together 
and was made with to be a more optimized approch than just using a list or numpy arrays 
in python */


/* --- Memory Pool API --- */

/* Inspired by yyjson's allocator structure / Memory 
* handler and is used to customize memory allocations */
typedef struct MemoryPool_s {
    void* (*malloc)(size_t size);
    void* (*calloc)(size_t size, size_t element_size);
    void* (*realloc)(void* ptr, size_t new_size);
    void (*free)(void* ptr);
} MemoryPool_t;

/* Used to allocate memory to the memory pool structure on the heap however the functions need to be set 
if null memory will be allocated to it... */
MemoryPool_t* memory_pool_init(MemoryPool_t* pool);

/* used to set it's own default api functions, this can be ignored if you have made your own settings*/
void memory_pool_set_default_settings(MemoryPool_t* pool);

/* Used to free a memory pool object */
void memory_pool_free(MemoryPool_t* pool);


/* --- Internal Node Structure API --- */

struct node_s {
    MemoryPool_t *pool;
    size_t capacity; /* The current memory that has been added to the heap */
    /* Tells how us much memory we would like to give to
     * our initial object structure everytime we expand the heap out */
    size_t expand_by; 
    size_t length; /* The current length of the structure we are carrying around in memory */
    unsigned char* data; /* the string we are carrying around in memory */
};

/* A node used to carry our objects around in such a way that is 
 * fast and easy to carry around */
typedef struct node_s node_t;

/* Makes a new node structure in memory this is useful for setting nodes easily... */
node_t *node_init(MemoryPool_t* pool, size_t default_cap, size_t expand_by);

/* appends a string to the given node value and is used to control 
expansion of memory and appending of the objects 
returns 1 on fail and 0 on success */
int node_expand(node_t* node, unsigned char* istr, size_t isize);

/* clears out the current node by setting it's length to 0 which is the mos efficient way to 
keep the memory it already has in place */
void node_clear(node_t* node);


/* used to free an internal node structure */
void node_free(node_t* node);



/* Note: It's faster to handle 
the structure directly since there's very 
little functions to go with it... */

struct table_s {
    MemoryPool_t *pool; /* a pool of memory to share with all memory sturctures... */
    size_t table_capacity; /* the current capacity of the tabel object */
    size_t default_expansion_size; /* used to control node-expansions */
    node_t** table; /* The internal node-table structure to handle */
};  

/* Faster than a hash table but its used for handling 
 * number to key pairs which means no need to hash our 
 * variables out the keys can all be obtained directly... */
typedef struct table_s table_t;

/* controls the creation of a new table to use... 
 * if pool is null a default pool will be made for it... 
 * this will also allocate memory to all the strings it needs */
table_t * table_init(MemoryPool_t* pool , size_t table_len ,size_t node_default_cap, size_t exapnsion_size);

/* Appends strings to a target node in the table , it's mainly here to keep 
the library complete and afloat but this can be ignored as needed... 
returns 1 on failure and 0 on success */
int table_node_append(table_t* table, size_t index, unsigned char* istr, size_t isize);


/* Frees the table and all the nodes that it's carrying around with itself... */
void table_free(table_t* table);


#endif /* TABLE_H */
