#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#define NARGS_WITH_AUX_PARAMS (NARGS + 2)
#endif

static const int MINIMUM_INPUT_PARAMS = 1;
static const char LINE_BREAK = '\n', STRING_NULL_TERMINATOR = '\0';

/*
 * Given a non-full input_buffer vector, the function moves the "ownership" and
 * the content of the new_line allocated memory to the next available position of input_buffer.
 * new_line has the line break removed and when the function returns it is nulled
 */
void
move_line_to_input_buffer(char *input_buffer[NARGS],
                          size_t *input_buff_size,
                          char **new_line)
{
	/* Remove the line break added by stdin */
	char *line_break_ptr = strchr(*new_line, LINE_BREAK);
	if (line_break_ptr != NULL) {
		*line_break_ptr = STRING_NULL_TERMINATOR;
	}

	input_buffer[*input_buff_size] = *new_line;
	*new_line = NULL;
	*input_buff_size += 1;
}

/*
 * Forks the current process and lets the child process run with exec the
 * desired command with the given non-null arguments from input_buffer. The
 * parent process will wait for the child to finish execution
 */
void
run_command(char *cmd, char *input_buffer[NARGS], size_t input_buff_size)
{
	char *exec_argv[NARGS_WITH_AUX_PARAMS] = { NULL };
	exec_argv[0] = cmd;

	for (size_t i = 0; i < input_buff_size; i++) {
		exec_argv[i + 1] = input_buffer[i];
	}

	pid_t child_id = fork();

	if (child_id < 0) {
		perror("Error while forking\n");
		exit(EXIT_FAILURE);

	} else if (child_id == 0) /* process is child */ {
		execvp(cmd, exec_argv);

		/* If this line is reached execvp returned, meaning that it failed */
		perror("Error from execvp\n");
		exit(EXIT_FAILURE);

	} else /* if process is parent */ {
		if (wait(NULL) < 0) {
			perror("Error on wait\n");
			exit(EXIT_FAILURE);
		}
	}
}

/*
 * Frees and then nulls each position of the input_buffer vector and resets to 0 input_buff_size
 */
void
clear_input_buffer(char *input_buffer[NARGS], size_t *input_buff_size)
{
	for (size_t i = 0; i < *input_buff_size; i++) {
		free(input_buffer[i]);
		input_buffer[i] = NULL;
	}
	*input_buff_size = 0;
}

int
main(int argc, char *argv[])
{
	if (argc != MINIMUM_INPUT_PARAMS + 1) {
		printf("Error while calling program. Expected %s <command>",
		       argv[0]);
		exit(EXIT_FAILURE);
	}
	char *cmd = argv[1];

	char *input_buffer[NARGS] = { NULL };
	size_t input_buff_size = 0;

	char *new_line = NULL;
	size_t line_size = 0;

	while (getline(&new_line, &line_size, stdin) != EOF) {
		move_line_to_input_buffer(input_buffer, &input_buff_size, &new_line);

		if (input_buff_size == NARGS) {
			run_command(cmd, input_buffer, input_buff_size);
			clear_input_buffer(input_buffer, &input_buff_size);
		}
	}

	if (input_buff_size != 0) {
		run_command(cmd, input_buffer, input_buff_size);
		clear_input_buffer(input_buffer, &input_buff_size);
	}

	free(new_line);
	exit(EXIT_SUCCESS);
}
