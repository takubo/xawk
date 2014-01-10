all: awk
	./awk -f sample.awk -f sample2.awk

awk: lexer.c
	${CC} -Wall -o awk lexer.c  -DSUPPORT_OCATAL_CONSTANT

.PHONY:clean

clean:
	rm awk
