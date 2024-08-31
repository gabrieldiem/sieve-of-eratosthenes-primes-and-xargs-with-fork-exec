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
static const int WRITE_SIDE = 1, READ_SIDE = 0;

void
print_prime(const int *prime)
{
	printf("primo %d\n", *prime);
	fflush(stdout);
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
drop_multiples(int *left_pipe_read_side)
{
	int prime = 0;
	int res = read(*left_pipe_read_side, &prime, sizeof(int));
	printf("res: %d\n", res);
	if (res < 0) {
		printf("Error reading from pipe\n");
		exit(EXIT_FAILURE);
	} else if (res == 0) {
		close(*left_pipe_read_side);
		exit(EXIT_SUCCESS);
	}

	print_prime(&prime);

	int right_pipe[PIPE_SIZE_VECTOR];
	if (pipe(right_pipe) == ERROR_CODE_PIPE) {
		exit(EXIT_FAILURE);
	}

	pid_t child_id = fork();
	if (child_id < 0) {
		printf("Error while forking\n");
		exit(EXIT_FAILURE);

	} else if (child_id == 0) {
		close(right_pipe[WRITE_SIDE]);
		drop_multiples(&right_pipe[READ_SIDE]);
		close(*left_pipe_read_side);

	} else {
		close(right_pipe[READ_SIDE]);
		bool keep_reading = true;
		while (keep_reading) {
			int value = 0;
			res = read(*left_pipe_read_side, &value, sizeof(int));
			printf("res: %d\n", res);
			if (res < 0) {
				printf("Error reading from pipe\n");
				exit(EXIT_FAILURE);
			} else if (res == 0) {
				keep_reading = false;
				break;
			}


			if (value % prime == 0) {
				// do nothing
			} else {
				res = write(right_pipe[WRITE_SIDE],
				            &value,
				            sizeof(int));

				if (res == 0) {
					printf("Error during writing of "
					       "numbers in right pipe\n");
					exit(EXIT_FAILURE);
				}
			}
		}
		close(right_pipe[WRITE_SIDE]);
		close(*left_pipe_read_side);
		wait_for_child();
	}
}


int
main(int argc, char *argv[])
{
	if (argc != MINIMUM_INPUT_PARAMS + 1) {
		printf("The program must be called in this form: ./primes "
		       "<max_num>");
		exit(EXIT_FAILURE);
	}

	int max_number = atoi(argv[1]);

	if (max_number < MINIMUM_NUMBER) {
		printf("Error at calling. The maximum number should be %d or "
		       "greater\n",
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

	} else if (child_id == 0) {
		close(initial_pipe[WRITE_SIDE]);
		drop_multiples(initial_pipe);

	} else {
		close(initial_pipe[READ_SIDE]);
		int res = 0;

		for (int i = MINIMUM_NUMBER; i <= max_number; i++) {
			res = write(initial_pipe[WRITE_SIDE], &i, sizeof(int));
			printf("write: %d\n", i);

			if (res == 0) {
				printf("Error during writing of numbers in "
				       "pipe\n");
				exit(EXIT_FAILURE);
			}
		}
		close(initial_pipe[WRITE_SIDE]);

		wait_for_child();
	}

	/*printf("begin read\n");
	for (int i = MINIMUM_NUMBER; i <= max_number; i++) {
	        int numread = -30;
	        int res = read(initial_pipe[READ_SIDE], &numread, sizeof(int));
	        if (res <= 0) {
	                printf("Error during writing of numbers in pipe\n");
	                exit(EXIT_FAILURE);
	        }

	        printf("read: %d\n", numread);
	}*/
	exit(EXIT_SUCCESS);
}