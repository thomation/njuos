NAME := $(shell basename $(PWD))
export MODULE := M1
all: $(NAME)-64 $(NAME)-32

test:
	for v in 32 64; do \
        echo Test $$v; \
		./$(NAME)-$$v -p; \
		./$(NAME)-$$v -n; \
		./$(NAME)-$$v -n -p; \
    done

include ../Makefile
