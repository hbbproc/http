Hbb_PATH=$(shell pwd)
SERVER_BIN=httpd
CLIENT_BIN=demo_client
CGI_BIN=$(HBB_PATH)/cgi_bin

CC=gcc
#FLAGS=#-D_DEBUG_
FLAGS=-D_DEBUG_
LDFLAGS=-lpthread#-static
INCLUDE=
LIB=

SERVER_SRC=httpd.c
CLIENT_SRC=demo_client.c


.PHONY:All
All:$(SERVER_BIN) $(CLIENT_BIN) cgi

$(SERVER_BIN):$(SERVER_SRC)
			    $(CC) -o $@ $^ $(FLAGS) $(LDFLAGS)
$(CLIENT_BIN):$(CLIENT_SRC)
				$(CC) -o $@ $^ $(FLAGS) $(LDFLAGS)

.PHONY:cgi
cgi:
	for name in 'echo $(CGI_PATH)';\
	do\
		cd $$name;\
		make;\
		cd -;\
	done

.PHONY:clean
clean:
	rm -rf $(SERVER_BIN) $(CLIENT_BIN) output
	for name in 'echo $(CGI_PATH)';\
	do\
		cd $$name;\
		make clean;\
		cd -;\
	done

.PHONY:output
output: All
	mkdir -p output/htdocs/cgi-bin
	cp httpd output
	cp demo_client output
	cp -rf conf output
	cp -rf log output
#	cp start.sh output
	cp -rf htdocs/* output/htdocs
	for name in 'echo $(CGI_PATH)';\
	do\
		cd $$name;\
		make output;\
		cd -;\
	done

