NAME := $(shell basename $(PWD))
export MODULE := M5
all: $(NAME)-64 $(NAME)-32
LDFLAGS += -lm

include ../Makefile
run:
	./frecov-64 ./M5-frecov.img
