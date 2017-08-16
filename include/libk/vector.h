#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <libk/heap.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEFAULT_CAPACITY 1

// ~Generic~ vector data structure. It can store both values or pointers.
// The type-specific vector are generated by calling the GENERATE_VECTOR
// and the GENERATE_VECTOR_PTR macro.
//
// The macro will generate the new, push, pop and delete function, as follows
//
// e.g. GENERATE_VECTOR(int) will generate the functions:
//
//      static int_vector* new_int_vector();
//      static void push_int_vector(int_vector* vector, int value);
//      static void pop_int_vector(int_vector* vector, int value);
//      static void delete_int_vector(int_vector* vector);
//
//      GENERATE_VECTOR_PTR(char) will generate the functions:
//
//      static char_ptr_vector* new_char_ptr_vector(bool own_ptrs);
//      static void push_char_ptr_vector(char_ptr_vector* vector, int value);
//      static void pop_char_ptr_vector(char_ptr_vector* vector, int value);
//      static void delete_char_ptr_vector(char_ptr_vector* vector);
//
// Given the verbosity of these functions, there are macros to make life easier
//
// e.g. int_vector* my_vector = new_int_vector();
//      push(my_vector, 1);  - calls push_int_vector
//      pop(my_vector)       - calls pop_int_vector
//      delete(my_vector)    - calls delete_int_vector
//
// Those macros work both for value vector and pointer vectors, and internal
// magic makes sure the correct function is called for each type of vector.
//
// Notice that for Vectors of pointers, the new function takes a second
// parameter own_ptrs. If true, the vector will own the given pointers,
// being responsible for deleting them when the vector is deleted.

typedef struct vector {
  uint32_t size;
  uint32_t capacity;
  size_t data_size;
  void* data;
} vector __attribute__((packed));

vector* new_vector(size_t vector_size, size_t size);
void delete_vector(vector* vector);
void vector_resize(vector* vector);

#define delete (__vector) __vector->delete (__vector)
#define push(__vector, __value) __vector->push(__vector, __value)
#define pop(__vector) __vector->pop(__vector)

#define GENERATE_VECTOR(type)                                                                 \
  typedef struct type##_vector {                                                              \
    uint32_t size;                                                                            \
    uint32_t capacity;                                                                        \
    size_t data_size;                                                                         \
    type* data;                                                                               \
    void (*delete)(struct type##_vector * vector);                                            \
    void (*push)(struct type##_vector * vector, type value);                                  \
    type (*pop)(struct type##_vector * vector);                                               \
  } type##_vector __attribute__((packed));                                                    \
                                                                                              \
  static void push_##type##_vector(type##_vector* vect, type value) {                         \
    if (vect->size == vect->capacity) {                                                       \
      vector_resize((vector*)vect);                                                           \
    }                                                                                         \
    vect->data[vect->size] = value;                                                           \
    vect->size++;                                                                             \
  }                                                                                           \
                                                                                              \
  static type pop_##type##_vector(type##_vector* vect) {                                      \
    vect->size--;                                                                             \
    return vect->data[vect->size];                                                            \
  }                                                                                           \
                                                                                              \
  static void delete_##type##_vector(type##_vector* vect) {                                   \
    delete_vector((vector*)vect);                                                             \
  }                                                                                           \
  \ 
                                                                        static type##_vector* \
      new_##type##_vector() {                                                                 \
    type##_vector* vector = new_vector(sizeof(type##_vector), sizeof(type));                  \
    vector->delete = delete_##type##_vector;                                                  \
    vector->push = push_##type##_vector;                                                      \
    vector->pop = pop_##type##_vector;                                                        \
    return vector;                                                                            \
  }

#define GENERATE_PTR_VECTOR(type)                                                                 \
  typedef struct type##_ptr_vector {                                                              \
    uint32_t size;                                                                                \
    uint32_t capacity;                                                                            \
    size_t data_size;                                                                             \
    type** data;                                                                                  \
    bool own_ptrs;                                                                                \
    void (*delete)(struct type##_ptr_vector * vector);                                            \
    void (*push)(struct type##_ptr_vector * vector, type* value);                                 \
    type* (*pop)(struct type##_ptr_vector * vector);                                              \
  } type##_ptr_vector __attribute__((packed));                                                    \
                                                                                                  \
  static void push_##type##_ptr_vector(type##_ptr_vector* vect, type* value) {                    \
    if (vect->size == vect->capacity) {                                                           \
      vector_resize(vect);                                                                        \
    }                                                                                             \
    vect->data[vect->size] = value;                                                               \
    vect->size++;                                                                                 \
  }                                                                                               \
                                                                                                  \
  static type* pop_##type##_ptr_vector(type##_ptr_vector* vect) {                                 \
    vect->size--;                                                                                 \
    return vect->data[vect->size];                                                                \
  }                                                                                               \
                                                                                                  \
  static void delete_##type##_ptr_vector(type##_ptr_vector* vect) {                               \
    if (vect->own_ptrs) {                                                                         \
      for (size_t i = 0; i < vect->size; i++) {                                                   \
        kfree(vect->data[i]);                                                                     \
      }                                                                                           \
    }                                                                                             \
    delete_vector((vector*)vect);                                                                 \
  }                                                                                               \
  \ 
                                                                        static type##_ptr_vector* \
      new_##type##_ptr_vector(bool own_ptrs) {                                                    \
    type##_ptr_vector* vector =                                                                   \
        new_vector(sizeof(type##_ptr_vector), sizeof(type*));                                     \
    vector->own_ptrs = own_ptrs;                                                                  \
    vector->delete = delete_##type##_ptr_vector;                                                  \
    vector->push = push_##type##_ptr_vector;                                                      \
    vector->pop = pop_##type##_ptr_vector;                                                        \
    return vector;                                                                                \
  }

GENERATE_VECTOR(int);
GENERATE_VECTOR(char);
GENERATE_VECTOR(double);
GENERATE_PTR_VECTOR(char);

#endif  // _VECTOR_H_