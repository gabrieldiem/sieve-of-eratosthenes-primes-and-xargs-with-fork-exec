#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

static const int MINIMUM_INPUT_PARAMS = 1;

void
move_line_to_input_buffer(char *input_buffer[NARGS],
                          size_t *input_buff_size,
                          char **new_line)
{
	input_buffer[*input_buff_size] = *new_line;
	*new_line = NULL;
	*input_buff_size += 1;
}

void
run_command(char *cmds[],
            int cmds_size,
            char *input_buffer[NARGS],
            size_t input_buff_size)
{
	for (int i = 1; i < cmds_size; i++) {
		printf("cmd: %s\n", cmds[i]);
	}

	for (size_t i = 0; i < input_buff_size; i++) {
		printf("input: %s\n", input_buffer[i]);
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

	char *input_buffer[NARGS] = { NULL };
	size_t input_buff_size = 0;

	char *new_line = NULL;
	size_t line_size = 0;

	while (getline(&new_line, &line_size, stdin) != EOF) {
		move_line_to_input_buffer(input_buffer, &input_buff_size, &new_line);

		if (input_buff_size == NARGS) {
			run_command(argv, argc, input_buffer, input_buff_size);
			clear_input_buffer(input_buffer, &input_buff_size);
		}
	}

	free(new_line);

	if (input_buff_size != 0) {
		run_command(argv, argc, input_buffer, input_buff_size);
		clear_input_buffer(input_buffer, &input_buff_size);
	}

	exit(EXIT_SUCCESS);
}
