all:
	gcc -Wall -O0 -fprofile-arcs -ftest-coverage -o example example.c ../code/gcov_public.c ../code/gcov_gcc.c ../code/gcov_printf.c
	mv *.gcno ../objs
	./example > ./example_log.txt

