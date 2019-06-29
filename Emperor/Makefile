FINAL_CFLAGS=-W -std=c++11 $(DEBUG) -pthread

MYSQL_CONCPP_DIR =`mysql_config --cflags`
MYSQL_CPPFLAGS = `mysql_config --libs`

OPTIMIZATION?=-O2
DEBUG=-g

SERVER_CC=$(CXX) $(FINAL_CFLAGS)  

SUM_SERVER_NAME=server
SUM_CLIENT_NAME=client
SUM_MYSQL_NAME=mysqltest
SUM_SERVER_OBJ= main.o util.o log.o cryptmsg.o genrandom.o epollserverhandle.o epollevent.o  channel.o filehandle.o
SUM_CLIENT_OBJ= clientmain.o util.o cryptmsg.o log.o clientuser.o epollevent.o
SUM_MYSQL_OBJ= mysqltest.o genrandom.o

all: $(SUM_SERVER_NAME) $(SUM_CLIENT_NAME)
	@echo "sb"
	@echo $(SERVER_CC) $(SUM_CLIENT_NAME)

Makefile.dep:
	#-$(SERVER_CC) -MM *.cpp > Makefile.dep  || true
	-$(SERVER_CC) -MM *.cpp > Makefile.dep 2> /dev/null || true

-include Makefile.dep

$(SUM_SERVER_NAME): $(SUM_SERVER_OBJ)
	$(SERVER_CC) -o $@ $^

$(SUM_CLIENT_NAME): $(SUM_CLIENT_OBJ)
	$(SERVER_CC) -o $@ $^

$(SUM_MYSQL_NAME) : $(SUM_MYSQL_OBJ)
	$(SERVER_CC)  -o $@ $^ 

%.o: %.cpp
	$(SERVER_CC) -c $<

.PHONY : clean
clean:
	rm  -rf $(SUM_SERVER_OBJ) $(SUM_CLIENT_OBJ) $(SUM_MYSQL_OBJ) $(SUM_SERVER_NAME) $(SUM_CLIENT_NAME) $(SUM_MYSQL_NAME) Makefile.dep
