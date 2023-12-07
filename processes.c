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

    if (pid < 0) { // Fork failed
        return -1;
    }

    if (pid == 0) { // Child process
        func(parameters);
        exit(EXIT_SUCCESS);
    }

    // Parent process
    return pid;
}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    // Do nothing if not parallel
    // Send terminate
    // Wait for responses
    // Free allocated memory
    // Free the MQ
}
