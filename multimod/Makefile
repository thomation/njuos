NAME := $(shell basename $(PWD))
all: $(NAME)-64 $(NAME)-32
export MODULE := Lab1
include ../Makefile

define test_with
	@while read line; do $(1) $$line; done < ./test_case 
endef

test: all
	$(call test_with, ./$(NAME)-64)
	$(call test_with, ./$(NAME)-32)