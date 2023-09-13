# cython:language_level = 3 
# distutils: sources = table.c

# "few fixes were made since the video came out but there will be more fixes to this code in episode 6" - Vizonex

cimport cython 
from cpython.mem cimport PyMem_Malloc , PyMem_Calloc, PyMem_Realloc , PyMem_Free
from cpython.bytes cimport PyBytes_FromStringAndSize, PyBytes_AsStringAndSize


cdef extern from "table.h":
    struct MemoryPool_s:
        void* (*malloc)(size_t size)
        void* (*calloc)(size_t size, size_t element_size)
        void* (*realloc)(void* ptr, size_t new_size)
        void (*free)(void* ptr)

    ctypedef MemoryPool_s MemoryPool_t 

    MemoryPool_t* memory_pool_init(MemoryPool_t* pool)

    void memory_pool_set_default_settings(MemoryPool_t* pool)

    void memory_pool_free(MemoryPool_t* pool)


    struct node_s:
        MemoryPool_t *pool
        size_t capacity # The current memory that has been added to the heap */
        # Tells how us much memory we would like to give to
        # our initial object structure everytime we expand the heap out 
        size_t expand_by
        size_t length # The current length of the structure we are carrying around in memory */
        unsigned char* data # the string we are carrying around in memory 

    ctypedef node_s node_t

    node_t* node_init(MemoryPool_t* pool, size_t default_cap, size_t expand_by)

    int node_expand(node_t* node, unsigned char* istr, size_t isize);

    void node_clear(node_t* node)

    void node_free(node_t* node)

    struct table_s:
        MemoryPool_t *pool # a pool of memory to share with all memory sturctures... 
        size_t table_capacity # the current capacity of the tabel object 
        size_t default_expansion_size # used to control node-expansions */
        node_t** table # The internal node-table structure to handle */
    
    ctypedef table_s table_t

    int table_node_append(table_t* table, size_t index, unsigned char* istr, size_t isize)


    table_t * table_init(MemoryPool_t* pool , size_t table_len ,size_t node_default_cap, size_t exapnsion_size)

    void table_free(table_t* table)


cdef void* mempool_malloc(size_t size):
    return PyMem_Malloc(size)

cdef void* mempool_calloc(size_t size, size_t element_size):
    return PyMem_Calloc(size, element_size)

cdef void* mempool_realloc(void*ptr, size_t size):
    return PyMem_Realloc(ptr, size)

cdef void mempool_free(void* ptr):
    return PyMem_Free(ptr)

cdef MemoryPool_t* initalize_py_mempool():
    cdef MemoryPool_t* pool = memory_pool_init(NULL)
    pool.malloc = mempool_malloc
    pool.calloc = mempool_calloc
    pool.realloc = mempool_realloc
    pool.free = mempool_free
    return pool 


@cython.no_gc_clear
cdef class Table:
    cdef:
        table_t* _table
        MemoryPool_t* _pool


    def __cinit__(self, size_t table_len, size_t node_default_cap, size_t exapnsion_size):
        self._pool = initalize_py_mempool()
        if self._pool == NULL:
            raise MemoryError

        self._table = table_init(self._pool, table_len, node_default_cap, exapnsion_size)
        if self._table == NULL:
            raise MemoryError

    def instert_object(self, object obj, size_t index):
        cdef Py_ssize_t obj_len
        cdef char* buff

        if not isinstance(obj, bytes):
            raise ValueError("object must be bytes")

        PyBytes_FromStringAndSize(&buff, &obj_len)

        if table_node_append(self._table, index, buff, obj_len):
            raise MemoryError

    def extract_object(self, size_t index):
        cdef node_t * node = self._table.table[index]
        return PyBytes_FromStringAndSize(<char*>node.data, index)
        

    def __dealloc__(self):
        table_free(self._table)
        memory_pool_free(self._pool)



