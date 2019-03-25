#开始
SYDemo:modulapi.o SYProtocol.o SYDevice.o serial.o pusb-linux.o
	g++ -g modulapi.o SYProtocol.o SYDevice.o serial.o pusb-linux.o -o SYDemo

modulapi.o:modulapi.cpp SYProtocol.h SYDevice.h
	g++ -g -c modulapi.cpp -o modulapi.o
SYProtocol.o:SYProtocol.cpp SYProtocol.h SYDevice.h
	g++ -g -c SYProtocol.cpp -o SYProtocol.o
SYDevice.o:SYDevice.cpp SYDevice.h pusb.h serial.h pusb-linux.h
	g++ -g -c SYDevice.cpp -o SYDevice.o
serial.o:serial.cpp serial.h vardef.h
	g++ -g -c serial.cpp -o serial.o
pusb-linux.o:pusb-linux.c pusb-linux.h pusb.h
	g++ -g -c pusb-linux.c -o pusb-linux.o

#生成静态库
	ar cqs SYProtocol.a SYProtocol.o SYDevice.o serial.o pusb-linux.o

clean:
	rm -f *.o *.a SYDemo
