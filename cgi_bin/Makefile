hbb_PATH=-I/home/hbb/http/mysql_connect -I/home/hbb/http/mysql_connect/mysql_lib/include -L//home/hbb/http/mysql_connect/mysql_lib/

.PHONY:all
all:cgi_math cgi_register cgi_showStudent
cgi_math:cgi_math.cpp 
	g++ -o $@ $^
cgi_register:cgi_register.cpp /home/hbb/http/mysql_connect/mysql_connect.cpp
	g++ -o $@ $^ $(hbb_PATH) -g
cgi_showStudent:cgi_showStudent.cpp /home/hbb/http/mysql_connect/mysql_connect.cpp
	g++ -o $@ $^ $(hbb_PATH) -g

.PHONY:clean
clean:
	rm -rf cgi_math cgi_register cgi_showStudent

.PHONY:output
output:
	cp cgi_math ../output/htdocs/cgi_bin
	cp cgi_register ../output/htdocs/cgi_bin
	cp cgi_showStudent ../output/htdocs/cgi_bin
