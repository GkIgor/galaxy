#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

/**
 * @brief Safely opens a file using platform-specific functions.
 *
 * This macro provides a cross-platform way to safely open files. On Windows,
 * it uses the secure fopen_s function, while on other platforms, it uses the
 * standard fopen function.
 *
 * @param file Pointer to a FILE pointer that will receive the opened file handle.
 * @param path The path to the file to be opened.
 * @param mode The mode in which to open the file (e.g., "r" for read, "w" for write).
 *
 * @return true if the file was successfully opened, false otherwise.
 */
#ifdef _WIN32
    #define fopen_safe(file, path, mode) \
        ((fopen_s(&(file), path, mode) == 0) && (file != NULL))
#else
    #define fopen_safe(file, path, mode) \
        (((file) = fopen(path, mode)) != NULL)
#endif

/**
 * @brief Provides a cross-platform string duplication function.
 *
 * On Windows, this macro maps strdup to _strdup, which is the Windows-specific
 * implementation. On other platforms, it leaves strdup unchanged.
 * 
 * OBS: For Windows compiler, the macro GSTRDUP_S may not work, due to conflicts
 * to MSVC compiler. If so, use _strdup.
 */

#ifdef _WIN32
#define strdup _strdup
#endif

/**
 * @brief Implements gstrdup(), a fork of strdup() for Galaxy Project.
 * 
 * This code implements a new more 'safer' version of strdup() for Galaxy Project.
 * It's not a big deal and its quite simple, btw, theres a thousand variations of the same code
 * around the web, so feel free to modify anything if u want.
 * 
 * @param String Returns the byte string pointer duplicated.
 * @return copy The copied string
 */

#define MAX_GSTRDUP_STRING_LENGHT 1024 * 1024 // 1 Megabyte.

#define STRDUP_S(string) ({                         \
    size_t size;                                    \
    char *copy;                                     \
                                                    \
    if (size > MAX_GSTRDUP_STRING_LENGHT) {         \
        errno = EINVAL;                             \
        return NULL;                                \
    }                                               \
                                                    \
    size = strlen(string) + 1;                      \
    if ((copy = malloc(size)) == NULL) {            \
        return NULL;                                \
    }                                               \
                                                    \
    (void)memcpy(copy, string, size);               \
    copy;                                           \
})                                                  

/**
 * @brief Safely allocates memory and handles allocation failures.
 *
 * This macro allocates memory using malloc and checks for allocation failures.
 * If the allocation fails, it prints an error message with the file and line
 * number, then exits the program.
 *
 * @param size The number of bytes to allocate.
 * @return A void pointer to the allocated memory if successful.
 * @note This macro will not return if memory allocation fails. Instead, it will
 *       terminate the program with an error message.
 * @example Usage example:
 *     #include "utils.h"
 *     #include <stdlib.h>
 *     
 *     int main() {
 *         // Allocates memory for an array of 10 integers
 *         int *array = MALLOC_S(10 * sizeof(int));
 *         
 *         // Uses allocated array
 *         for (int i = 0; i < 10; i++) {
 *             array[i] = i * i;
 *             printf("array[%d] = %d\n", i, array[i]);
 *         }
 *         
 *         // Frees allocated memory
 *         free(array);
 *         return 0;
 *     }
 */
#define MALLOC_S(size) ({                     \
    void *ptr = malloc(size);                 \
    if (ptr == NULL) {                        \
        fprintf(stderr, "Insufficient memory in %s:%d\n", __FILE__, __LINE__); \
        exit(EXIT_FAILURE);                   \
    }                                         \
    ptr;                                      \
})

/**
 * @file utils.h
 * @brief Demonstrates safe dynamic memory management using MALLOC_S and REALLOC_S macros.
 * 
 * This example shows how to use MALLOC_S for initial memory allocation and REALLOC_S 
 * for resizing an existing memory block safely. The macros handle memory allocation 
 * failures gracefully by reporting the error and terminating the program to prevent 
 * undefined behavior.
 *
 * Usage:
 * 1. MALLOC_S(size): Allocates memory of the given size and ensures success.
 * 2. REALLOC_S(ptr, size): Reallocates memory for the given pointer, expanding or shrinking 
 *    the memory block as needed, while checking for allocation failures.
 *
 * Example:
 * The code below allocates an integer array of size 5, initializes it, and then 
 * resizes the array to hold 10 integers using REALLOC_S.
 */

#define REALLOC_S(ptr, size) ({                                           \
    void *new_ptr = realloc(ptr, size);                                    \
    if (new_ptr == NULL) {                                                 \
        fprintf(stderr, "Insufficient memory during realloc in %s:%d\n",   \
                __FILE__, __LINE__);                                       \
        free(ptr);                                                         \
        exit(EXIT_FAILURE);                                                \
    }                                                                      \
    new_ptr;                                                               \
})

/**
 * @brief Macro for safely freeing dynamically allocated memory.
 *
 * This macro frees the memory pointed to by a given pointer and sets the pointer to `NULL`
 * to avoid dangling references. If the pointer is already `NULL`, it logs a warning message.
 *
 * @param ptr The pointer to the memory to be freed.
 *
 * @example Usage example:
 *     #include "utils.h"
 *     
 *     int main() {
 *         int *array = MALLOC_S(10 * sizeof(int)); // Allocate memory
 *         
 *         // Free memory safely
 *         FREE_S(array);
 *         
 *         // Attempting to free again shows a warning
 *         FREE_S(array);
 *         return 0;
 *     }
 */
// #define FREE_S(ptr) do {                             \
//     if (ptr != NULL) {                               \
//         free(ptr);                                   \
//         ptr = NULL;                                  \
//     } else {                                         \
//         fprintf(stderr, "Warning: Attempt to free a NULL pointer at %s:%d\n", __FILE__, __LINE__); \
//     }                                                \
// } while (0)

#endif // UTILS_H