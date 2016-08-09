hbb_PATH=$(shell pwd)
INCLUDE=-I$(hbb_PATH)/mysql_lib/include/
LIB=-L$(hbb_PATH)/mysql_lib/lib
LDFLAG=-lmysqlclient

mysql_connect:mysql_connect.cpp
	g++ -o $@ $^ $(INCLUDE) $(LIB) $(LDFLAG) -g -D_DEBUG_

.PHONY:clean
clean:
	rm -rf mysql_connect
	
