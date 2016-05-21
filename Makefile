RM = rm -fv
DEBUG = -Wall -Wextra -g -o
LFLAGS =  -pthread -lrt
CVFLAGS = -ggdb `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv` 

all: qr

qr:
	gcc ./qr.c $(CVFLAGS) $(LFLAGS) $(LIBS) -o qr
clean:
	@echo 'Cleaning object files'
	$(RM) qr
