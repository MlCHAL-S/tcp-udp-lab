### 1. Choose one of the git commits:
8bf8a31 Working TCP Java
a5e2dc1 Working TCP - multiple clients
353bbab Working TCP
9b23e5d Working UDP
03ca446 Inicial program

### 2. Test Program

# Compile C
gcc squdpserv.c -o squdpserv
gcc squdpclient.c -o squdpclient

# Run C
./squdpserv
./squdpclient localhost 5


# Compile Java
javac SqTCPServ.java

# Run Java
java SqTCPServ
telnet localhost 5000
