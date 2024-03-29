#include <openssl/evp.h>
#include "file-properties.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "defines.h"
#include <fcntl.h>

#include <stdlib.h>

/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {
    if (entry == NULL) {
        fprintf(stderr, "Error: Entry is NULL\n");
        return -1;
    }

    struct stat file_stat;
    if (stat(entry->path_and_name, &file_stat) < 0) {
        perror("stat failed");
        return -1;
    }

    entry->mode = file_stat.st_mode;
    entry->mtime.tv_sec = file_stat.st_mtime; // seconds
    entry->mtime.tv_nsec = 0;

    if (S_ISREG(file_stat.st_mode)) {
        entry->size = file_stat.st_size;
        entry->entry_type = FICHIER;
        if (compute_file_md5(entry) < 0) {
            fprintf(stderr, "Error computing MD5 for file: %s\n", entry->path_and_name);
            return -1;
        }
    } else if (S_ISDIR(file_stat.st_mode)) {
        entry->entry_type = DOSSIER;
    } else {
        fprintf(stderr, "Error: Not a file or directory: %s\n", entry->path_and_name);
        return -1;
    }

    return 0;
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */

int compute_file_md5(files_list_entry_t *entry) {
    if (entry == NULL || entry->entry_type != FICHIER) { // Use entry_type
        return -1;
    }
    OpenSSL_add_all_algorithms();
    FILE *file = fopen(entry->path_and_name, "rb"); // Use path_and_name
    if (!file) {
        return -1;
    }

    unsigned char md5_sum[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(file);
        return -1;
    }

    // Obtain file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    int errorCode = EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
    if ( errorCode != 1 ||
        EVP_DigestUpdate(mdctx, file, file_size) != 1 ||
        EVP_DigestFinal_ex(mdctx, md5_sum, NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return -1;
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);
    memcpy(entry->md5sum, md5_sum, EVP_MAX_MD_SIZE); // Use md5sum

    return 0;
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    if (path_to_dir == NULL) {
        return false;
    }

    struct stat statbuf;
    if (stat(path_to_dir, &statbuf) != 0) {
        return false;
    }

    return S_ISDIR(statbuf.st_mode);
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    if (path_to_dir == NULL) {
        return false;
    }

    char temp_path[PATH_SIZE];
    snprintf(temp_path, PATH_SIZE, "%s/.tmpXXXXXX", path_to_dir);

    int fd = mkstemp(temp_path);
    if (fd == -1) {
        return false;
    }

    close(fd); // fermer le fichier temporaire
    unlink(temp_path); // nettoyer le fichier temporaire

    return true;
}
