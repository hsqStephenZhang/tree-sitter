#ifndef RUNTIME_ARRAY_H_
#define RUNTIME_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "runtime/alloc.h"

#define Array(T)     \
  struct {           \
    T *contents;     \
    uint32_t size;     \
    uint32_t capacity; \
  }

#define array_init(self) \
  ((self)->size = 0, (self)->capacity = 0, (self)->contents = NULL)

#define array_new() \
  { NULL, 0, 0 }

#define array_get(self, index) \
  (assert((uint32_t)index < (self)->size), &(self)->contents[index])

#define array_front(self) array_get(self, 0)

#define array_back(self) array_get(self, (self)->size - 1)

#define array_clear(self) ((self)->size = 0)

#define array_grow(self, new_capacity) \
  array__grow((VoidArray *)(self), array__elem_size(self), new_capacity)

#define array_erase(self, index) \
  array__erase((VoidArray *)(self), array__elem_size(self), index)

#define array_delete(self) array__delete((VoidArray *)self)

#define array_push(self, element)                                    \
  (array_grow((self), (self)->size + 1), \
   (self)->contents[(self)->size++] = (element))

#define array_push_all(self, other)                                       \
  array_splice((self), (self)->size, 0, (other)->size, (other)->contents)

#define array_splice(self, index, old_count, new_count, new_elements)          \
  array__splice((VoidArray *)(self), array__elem_size(self), index, old_count, \
                new_count, (new_elements))

#define array_insert(self, index, element) \
  array_splice(self, index, 0, 1, &(element))

#define array_pop(self) ((self)->contents[--(self)->size])

// Private

typedef Array(void) VoidArray;

#define array__elem_size(self) sizeof(*(self)->contents)

static inline void array__delete(VoidArray *self) {
  ts_free(self->contents);
  self->contents = NULL;
  self->size = 0;
  self->capacity = 0;
}

static inline void array__erase(VoidArray *self, size_t element_size,
                                uint32_t index) {
  assert(index < self->size);
  char *contents = (char *)self->contents;
  memmove(contents + index * element_size, contents + (index + 1) * element_size,
          (self->size - index - 1) * element_size);
  self->size--;
}

static inline void array__grow(VoidArray *self, size_t element_size,
                               uint32_t new_capacity) {
  if (new_capacity > self->capacity) {
    if (new_capacity < 2 * self->capacity)
      new_capacity = 2 * self->capacity;
    if (new_capacity < 8)
      new_capacity = 8;
    if (self->contents)
      self->contents = ts_realloc(self->contents, new_capacity * element_size);
    else
      self->contents = ts_calloc(new_capacity, element_size);
    self->capacity = new_capacity;
  }
}

static inline void array__splice(VoidArray *self, size_t element_size,
                                 uint32_t index, uint32_t old_count,
                                 uint32_t new_count, void *elements) {
  uint32_t new_size = self->size + new_count - old_count;
  uint32_t old_end = index + old_count;
  uint32_t new_end = index + new_count;
  assert(old_end <= self->size);

  array__grow(self, element_size, new_size);

  char *contents = (char *)self->contents;
  if (self->size > old_end)
    memmove(contents + new_end * element_size, contents + old_end * element_size,
            (self->size - old_end) * element_size);
  if (new_count > 0)
    memcpy((contents + index * element_size), elements,
           new_count * element_size);
  self->size += new_count - old_count;
}

#ifdef __cplusplus
}
#endif

#endif  // RUNTIME_ARRAY_H_
