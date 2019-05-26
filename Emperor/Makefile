
DEBUG=-g -ggdb
SERVER_CC=g++ $(DEBUG) -Wall -std=c++11


SUM_SERVER_NAME=aserver
SUM_SERVER_OBJ= main.o util.o
all: $(SUM_SERVER_NAME)
	@echo "hello!"

$(SUM_SERVER_NAME): $(SUM_SERVER_OBJ)
	$(SERVER_CC) -o $@ $^

%.o: %.c .make-prerequisites
	$(SERVER_CC) -c $<