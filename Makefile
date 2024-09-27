all: ssi

ssi: ssi.c
	gcc ssi.c -o ssi

clean:
	rm -f ssi
