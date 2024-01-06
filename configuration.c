#include "configuration.h"
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum {DATE_SIZE_ONLY, NO_PARALLEL, DRY_RUN} long_opt_values;

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code, you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date-size-only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
    printf("         \t--dry-run for test execution (just list the operations to do, do not actually make the copies)\n");
    printf("         \t-v for verbose (display of the list and operations in details)\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    if (the_config != NULL) {
        the_config->source[0] = '\0'; // Chemin source vide par défaut
        the_config->destination[0] = '\0'; // Chemin destination vide par défaut
        the_config->processes_count = 1; // Un seul processus par défaut
        the_config->is_parallel = true; // Par défaut, exécuter en parallèle
        the_config->uses_md5 = true; // Par défaut, utiliser le calcul MD5
        the_config->uses_verbose = false; // Par défaut, ne pas utiliser verbose
        the_config->uses_dry_run = false; // Par défaut, ne pas utilsier dry-run
    }
}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) {
    if (the_config == NULL) {
        return -1;
    }

    // Options longues
    static struct option long_options[] = {
            {"date-size-only", no_argument, 0, DATE_SIZE_ONLY},
            {"no-parallel", no_argument, 0, NO_PARALLEL},
            {"dry-run", no_argument, 0, DRY_RUN},
            {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "hvn:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                display_help(argv[0]);
                exit(EXIT_SUCCESS);
            case 'n':
                the_config->processes_count = (uint8_t) atoi(optarg);
                break;
            case 'v':
                the_config->uses_verbose = true;
                break;
            case DATE_SIZE_ONLY:
                the_config->uses_md5 = false;
                break;
            case NO_PARALLEL:
                the_config->is_parallel = false;
                break;
            case DRY_RUN:
                the_config->uses_dry_run = true;
            default:
                display_help(argv[0]);
                return -1;
        }
    }

    // Vérifier si les dossiers source et destination sont spécifiés
    if (argc - optind < 2) {
        fprintf(stderr, "Source and destination directories are required.\n");
        display_help(argv[0]);
        return -1;
    }

    // Affecter les chemins source et destination
    strncpy(the_config->source, argv[optind], sizeof(the_config->source) - 1);
    the_config->source[sizeof(the_config->source) - 1] = '\0';
    strncpy(the_config->destination, argv[optind + 1], sizeof(the_config->destination) - 1);
    the_config->destination[sizeof(the_config->destination) - 1] = '\0';

    return 0;
}
