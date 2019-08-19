CFLAGS = -Wall -O0 -g3
INC = 

# All Target
all: q_qsort

C_SRCS += main.c

q_qsort: main.c
	@echo 'Building file: $<'
	@echo 'Invoking: PowerPC gcc compiler'
	powerpc-linux-gnu-gcc $(CFLAGS) main.c $(INC) -o q_qsort
	@echo 'Finished building: $<'
	@echo ' '

# Other Targets
clean:
	rm -f q_qsort
