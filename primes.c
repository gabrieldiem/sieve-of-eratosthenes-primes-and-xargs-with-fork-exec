#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PIPE_SIZE_VECTOR 2

static const int MINIMUM_INPUT_PARAMS = 1;
static const int MINIMUM_NUMBER = 2;
static const int ERROR_CODE_PIPE = -1;
static const int READ_SIDE = 0, WRITE_SIDE = 1;

void
print_prime(const int prime)
{
	printf("primo %d\n", prime);
	fflush(stdout);
	return;
}

void
wait_for_child()
{
	if (wait(NULL) < 0) {
		printf("Error on wait\n");
		exit(EXIT_FAILURE);
	}
}

void
write_to_pipe_non_multiples_of_prime(const int left_pipe_read_side,
                                     const int right_pipe_write_side,
                                     const int prime)
{
	int res = 0;
	bool keep_reading = true;

	while (keep_reading) {
		int value = 0;
		res = read(left_pipe_read_side, &value, sizeof(int));

		if (res < 0) {
			printf("Error while reading from pipe\n");
			exit(EXIT_FAILURE);
		} else if (res == 0) /* final read */ {
			keep_reading = false;
			break;
		}

		if (value % prime != 0) {
			res = write(right_pipe_write_side, &value, sizeof(int));

			if (res == 0) {
				printf("Error during writing of "
				       "numbers in right pipe\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

void
drop_multiples(const int left_pipe_read_side)
{
	/* Read the prime let out by the previous process */
	int prime = 0;
	int res = read(left_pipe_read_side, &prime, sizeof(int));

	if (res < 0) {
		printf("Error while reading from pipe\n");
		exit(EXIT_FAILURE);

	} else if (res == 0) /* final read */ {
		close(left_pipe_read_side);
		return;
	}

	print_prime(prime);

	int right_pipe[PIPE_SIZE_VECTOR];
	if (pipe(right_pipe) == ERROR_CODE_PIPE) {
		exit(EXIT_FAILURE);
	}

	pid_t child_id = fork();

	if (child_id < 0) {
		printf("Error while forking\n");
		exit(EXIT_FAILURE);

	} else if (child_id == 0) /* process is child */ {
		close(right_pipe[WRITE_SIDE]);
		close(left_pipe_read_side);
		drop_multiples(right_pipe[READ_SIDE]);

	} else /* if process is parent */ {
		close(right_pipe[READ_SIDE]);
		write_to_pipe_non_multiples_of_prime(left_pipe_read_side,
		                                     right_pipe[WRITE_SIDE],
		                                     prime);
		close(right_pipe[WRITE_SIDE]);
		close(left_pipe_read_side);
		wait_for_child();
	}
}

void
write_numbers_in_initial_pipe(const int pipe_read_side, const int max_number)
{
	int res = 0;
	for (int i = MINIMUM_NUMBER; i <= max_number; i++) {
		res = write(pipe_read_side, &i, sizeof(int));

		if (res == 0) {
			printf("Error during writing of numbers in initial "
			       "pipe\n");
			exit(EXIT_FAILURE);
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc != MINIMUM_INPUT_PARAMS + 1) {
		printf("Error while calling program. Expected %s <max_number> "
		       "call with no extra arguments",
		       argv[0]);
		exit(EXIT_FAILURE);
	}

	int max_number = atoi(argv[1]);

	if (max_number < MINIMUM_NUMBER) {
		printf("Error while calling program. The maximum number should "
		       "be %d or greater\n",
		       MINIMUM_NUMBER);
		exit(EXIT_FAILURE);
	}

	int initial_pipe[PIPE_SIZE_VECTOR];
	if (pipe(initial_pipe) == ERROR_CODE_PIPE) {
		exit(EXIT_FAILURE);
	}

	pid_t child_id = fork();

	if (child_id < 0) {
		printf("Error while forking\n");
		exit(EXIT_FAILURE);

	} else if (child_id == 0) /* process is child */ {
		close(initial_pipe[WRITE_SIDE]);
		drop_multiples(initial_pipe[READ_SIDE]);

	} else /* if process is parent */ {
		close(initial_pipe[READ_SIDE]);
		write_numbers_in_initial_pipe(initial_pipe[WRITE_SIDE],
		                              max_number);
		close(initial_pipe[WRITE_SIDE]);

		wait_for_child();
	}

	exit(EXIT_SUCCESS);
}