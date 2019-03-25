#¿ªÊ¼
SYDemo:modulapi.o
	g++ -g modulapi.o SYProtocol.a -o SYDemo

modulapi.o:modulapi.cpp SYProtocol.h
	g++ -g -c modulapi.cpp -o modulapi.o

clean:
	rm -f *.o SYDemo
