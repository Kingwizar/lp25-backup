#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include "messages.h"
#include "file-properties.h"
#include "sync.h"
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
/*!
 * @brief prepare prepares (only when parallel is enabled) the processes used for the synchronization.
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the program processes context
 * @return 0 if all went good, -1 else
 */
int prepare(configuration_t *the_config, process_context_t *p_context) {
    if (the_config == NULL || p_context == NULL) {
        return -1;
    }

    // Initialize process context
    p_context->processes_count = the_config->processes_count;
    p_context->main_process_pid = getpid();
    p_context->source_lister_pid = -1;
    p_context->destination_lister_pid = -1;
    p_context->source_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
    p_context->destination_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);

    if (p_context->source_analyzers_pids == NULL || p_context->destination_analyzers_pids == NULL) {
        free(p_context->source_analyzers_pids);
        free(p_context->destination_analyzers_pids);
        return -1;
    }

    // Setup message queue
    p_context->shared_key = ftok("/tmp", 'a'); // Or any other key generation method
    p_context->message_queue_id = msgget(p_context->shared_key, 0644 | IPC_CREAT);

    if (p_context->message_queue_id == -1) {
        free(p_context->source_analyzers_pids);
        free(p_context->destination_analyzers_pids);
        return -1;
    }

    // Create source lister process :
    p_context->source_lister_pid = make_process(p_context, source_lister_process_loop, NULL);
    if (p_context->source_lister_pid == -1) {
        perror("Failed to create source lister process");
        return -1;
    }

    // Create destination lister process :
    p_context->destination_lister_pid = make_process(p_context, destination_lister_process_loop, NULL);
    if (p_context->destination_lister_pid == -1) {
        perror("Failed to create destination lister process");
        return -1;
    }

    // Create source analyzers processes
    for (int i = 0; i < the_config->processes_count; ++i) {
        p_context->source_analyzers_pids[i] = make_process(p_context, source_analyzer_process_loop, NULL);
        if (p_context->source_analyzers_pids[i] == -1) {
            perror("Failed to create source analyzer process");
            return -1;
        }
    }

    // Create destination analyzers processes
    for (int i = 0; i < the_config->processes_count; ++i) {
        p_context->destination_analyzers_pids[i] = make_process(p_context, destination_analyzer_process_loop, NULL);
        if (p_context->destination_analyzers_pids[i] == -1) {
            perror("Failed to create destination analyzer process");
            return -1;
        }
    }

    return 0;
}

/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */

int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
    pid_t pid = fork();


    if (pid == 0) { // Child process
        func(parameters);
    }else{
        p_context->processes_count++;
        return pid;
    }
}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    lister_configuration_t *config = (lister_configuration_t *)parameters;
    // Logique de listage (par exemple, lister des fichiers dans un répertoire)
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
    analyzer_configuration_t *config = (analyzer_configuration_t *)parameters;
    // Logique d'analyse (par exemple, comparer des propriétés de fichiers)
}

// Fonction make_process inchangée...

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting for confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    if (the_config == NULL || p_context == NULL) {
        return;
    }

    // Envoyer un signal de terminaison aux processus enfants
    for (int i = 0; i < the_config->processes_count; i++) {
        if (p_context->source_analyzers_pids[i] > 0) {
            kill(p_context->source_analyzers_pids[i], SIGTERM);
        }
        if (p_context->destination_analyzers_pids[i] > 0) {
            kill(p_context->destination_analyzers_pids[i], SIGTERM);
        }
    }

    // Attendre la confirmation de terminaison
    for (int i = 0; i < the_config->processes_count; i++) {
        if (p_context->source_analyzers_pids[i] > 0) {
            waitpid(p_context->source_analyzers_pids[i], NULL, 0);
        }
        if (p_context->destination_analyzers_pids[i] > 0) {
            waitpid(p_context->destination_analyzers_pids[i], NULL, 0);
        }
    }

    // Libérer la mémoire allouée
    free(p_context->source_analyzers_pids);
    free(p_context->destination_analyzers_pids);

    // Supprimer la file de messages
    msgctl(p_context->message_queue_id, IPC_RMID, NULL);
}