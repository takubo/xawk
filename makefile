all: awk
	#./awk
	./awk -f sample.awk -f sample2.awk

awk: lexer.c
	${CC} -Wall -o awk lexer.c

.PHONY:clean

clean:
	rm awk
