CC = gcc
LIBS = -lportaudio -lm

all: aleatoric

aleatoric: aleatoric.c
	$(CC) -o aleatoric aleatoric.c $(LIBS)

clean:
	rm -f aleatoric *.o *.aleatoric *.wav
