all: benchmark validate tests concurrency

benchmark: benchmark.c
	$(CC) -g -O0 benchmark.c -o benchmark -lckv
validate: validate.c
	$(CC) -g -O0 validate.c -o validate -lckv
tests: tests.c
	$(CC) -g -O0 tests.c -o tests -lckv 
concurrency: concurrency.c
	$(CC) -g -O0 concurrency.c -o concurrency -lckv -pthread
clean:
	rm -f benchmark validate tests concurrency
