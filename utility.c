#include "defines.h"
#include <string.h>

/*!
 * @brief concat_path concatenates suffix to prefix into result
 * It checks if prefix ends by / and adds this token if necessary
 * It also checks that result will fit into PATH_SIZE length
 * @param result the result of the concatenation
 * @param prefix the first part of the resulting path
 * @param suffix the second part of the resulting path
 * @return a pointer to the resulting path, NULL when concatenation failed
 */
char *concat_path(char *result, char *prefix, char *suffix) {
    // Check if the inputs are valid
    if (result == NULL || prefix == NULL) {
        return NULL;
    }else if (suffix == NULL){
        return prefix;
    }

    // Check total length
    size_t total_length = strlen(prefix) + strlen(suffix) + 2; // +2 for potential '/' and '\0'
    if (total_length > PATH_SIZE) {
        return NULL;
    }

    // Copy the prefix to the result
    strcpy(result, prefix);

    // Check if the prefix already ends with '/'
    if (prefix[strlen(prefix) - 1] != '/') {
        // Add '/' if it's not already there
        strcat(result, "/");
    }

    // Concatenate the suffix
    strcat(result, suffix);

    return result;
}
