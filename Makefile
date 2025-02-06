default: sum par_sum maybe_sum

sum: sum.c
	gcc -g -O2 --std=c99 -Wall -o sum sum.c

par_sum: par_sum.c
	gcc -g -O2 --std=c99 -Wall -o par_sum par_sum.c -lpthread

maybe_sum: maybe_sum.c
	gcc -g -O2 --std=c99 -Wall -pthread -o maybe_sum maybe_sum.c

clean:
	rm -f sum par_sum maybe_sum