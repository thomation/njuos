NAME := $(shell basename $(PWD))
all: $(NAME)-64 $(NAME)-32
export MODULE := Lab1
include ../Makefile

test: all
	python3 gen_num.py | sh test_mul.sh ./$(NAME)-64 
	python3 gen_num.py | sh test_mul.sh ./$(NAME)-32