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

void
move_line_to_input_buffer(char *input_buffer[NARGS],
                          size_t *input_buff_size,
                          char **new_line)
{
	/* Remove the line break added by stdin */
	char *line_break_ptr = strchr(*new_line, '\n');
	if (line_break_ptr != NULL) {
		*line_break_ptr = '\0';
	}

	input_buffer[*input_buff_size] = *new_line;
	*new_line = NULL;
	*input_buff_size += 1;
}

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
		printf("Error while forking\n");
		exit(EXIT_FAILURE);

	} else if (child_id == 0) /* process is child */ {
		execvp(cmd, exec_argv);

		/* If this line is reached execvp failed and returned */
		printf("Error from execvp\n");
		exit(EXIT_FAILURE);

	} else /* if process is parent */ {
		if (wait(NULL) < 0) {
			printf("Error on wait\n");
			exit(EXIT_FAILURE);
		}
	}
}

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
		printf("Error while calling program. Expected %s [command "
		       "[initial-arguments]]",
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
