# makfile configuration
CPU             	= msp430g2553
CFLAGS          	= -mmcu=${CPU} -Os -I../h
LDFLAGS		= -L../lib -L/opt/ti/msp430_gcc/include/ 

#switch the compiler (for the internal make rules)
CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all:memorygame.elf

#additional rules for files
memorygame.elf: ${COMMON_OBJECTS} memorygame.o wdt_handler.o inputChecker.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd 

inputChecker.o: inputChecker.s
	${AS} $< -o $@

load: memorygame.elf inputChecker.o
	msp430loader.sh $^

clean:
	rm -f *.o *.elf
