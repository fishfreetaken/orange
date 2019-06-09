FINAL_CFLAGS=-W -std=c++11 $(DEBUG) -pthread

MYSQL_CONCPP_DIR =`mysql_config --cflags`
MYSQL_LDLIBS= -lmysqlclient
MYSQL_CPPFLAGS = `mysql_config --libs`


OPTIMIZATION?=-O2
DEBUG=-g

SERVER_CC=$(CXX) $(FINAL_CFLAGS)  $(MYSQL_CONCPP_DIR)

SUM_SERVER_NAME=server
SUM_CLIENT_NAME=client
SUM_MYSQL_NAME=mysqltest
SUM_SERVER_OBJ= main.o util.o log.o genrandom.o
SUM_CLIENT_OBJ= clientmain.o util.o log.o 
SUM_MYSQL_OBJ= mysqltest.o genrandom.o

all: $(SUM_SERVER_NAME) $(SUM_CLIENT_NAME) $(SUM_MYSQL_NAME)
	@echo "chat server!"
	@echo $(SERVER_CC) $(SUM_CLIENT_NAME)

$(SUM_SERVER_NAME): $(SUM_SERVER_OBJ)
	$(SERVER_CC) -o $@ $^

$(SUM_CLIENT_NAME): $(SUM_CLIENT_OBJ)
	$(SERVER_CC) -o $@ $^

$(SUM_MYSQL_NAME) : $(SUM_MYSQL_OBJ)
	$(SERVER_CC)  -o $@ $^ $(MYSQL_CPPFLAGS) 

%.o: %.cpp
	$(SERVER_CC) -c $<

.PHONY : clean
clean:
	rm  -rf $(SUM_SERVER_OBJ) clientmain.o
