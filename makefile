all : 2015082_Shell_2.c 2015082_Shell_1.c
		gcc -o b.out 2015082_Shell_2.c
		gcc 2015082_Shell_1.c
clean:
	rm a.out
	rm b.out