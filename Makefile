all:
	gcc jni/uartRS.c -o uartRS -lpthread

clean:
	rm uartRS
