#include "sync.h"
#include <dirent.h>
#include <string.h>
#include "processes.h"
#include "utility.h"
#include "messages.h"
#include "file-properties.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>

#include <stdio.h>

/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    if (the_config == NULL || p_context == NULL) {
        return;
    }

    files_list_t src_list = {0}, dst_list = {0};

    if (the_config->is_parallel) {
        make_files_lists_parallel(&src_list, &dst_list, the_config, p_context->message_queue_id);
    } else {
        make_files_list(&src_list, the_config->source);
        make_files_list(&dst_list, the_config->destination);
    }

    // Compare lists and synchronize files
    // ...
    files_list_entry_t* tempsrc = src_list.head;
    files_list_entry_t* tempdst = dst_list.head;

    while(tempsrc != NULL && tempdst != NULL){
        if(mismatch(tempsrc,tempdst,the_config->uses_md5)){
            tempdst = tempsrc;
        }
        tempsrc = tempsrc->next;
        tempdst = tempdst->next;
    }

    clear_files_list(&src_list);
    clear_files_list(&dst_list);
}

/*!
 * @brief mismatch tests if two files with the same name (one in source, one in destination) are equal
 * @param lhd a files list entry from the source
 * @param rhd a files list entry from the destination
 * @has_md5 a value to enable or disable MD5 sum check
 * @return true if both files are not equal, false else
 */
bool mismatch(files_list_entry_t *lhd, files_list_entry_t *rhd, bool has_md5) {
    if (lhd == NULL || rhd == NULL) {
        return true; // Consider mismatch if either entry is NULL
    }

    if (lhd->size != rhd->size || lhd->mtime.tv_sec != rhd->mtime.tv_sec ||
        lhd->mtime.tv_nsec != rhd->mtime.tv_nsec) {
        return true;
    }

    if (has_md5 && memcmp(lhd->md5sum, rhd->md5sum, sizeof(lhd->md5sum)) != 0) {
        return true;
    }

    return false;
}

/*!
 * @brief make_files_list buils a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {
    if (list == NULL || target_path == NULL) {
        return;
    }

    make_list(list, target_path);
}

/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {
    if (src_list == NULL || dst_list == NULL || the_config == NULL) {
        return;
    }
}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    if (source_entry == NULL || the_config == NULL) {
        return;
    }

    // Construct the destination path
    char dest_path[PATH_SIZE];
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
    if (list == NULL || target == NULL) {
        return;
    }

    DIR *dir = open_dir(target);
    if (dir == NULL) {
        return;
    }

    struct dirent *entry;
    while ((entry = get_next_entry(dir)) != NULL) {
        // Construct full path and add to list
        // ...
    }

    closedir(dir);
}

/*!
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
    return (path != NULL) ? opendir(path) : NULL;
}


/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    if (dir == NULL) {
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            return entry; // Return the entry if it's not '.' or '..'
        }
    }

    return NULL;
}
