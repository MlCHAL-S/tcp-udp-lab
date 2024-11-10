### 1. Choose one of the git commits:
- 03ca446 Inicial program
- 9b23e5d Working UDP
- 353bbab Working TCP
- a5e2dc1 Working TCP - multiple clients
- 8bf8a31 Working TCP Java

### 2. Test Program

# Compile C
```bash
gcc squdpserv.c -o squdpserv
```

```bash
gcc squdpclient.c -o squdpclient
```

# Run C
```bash
./squdpserv
```

```bash
./squdpclient localhost 5
```

# Compile Java
```bash
javac SqTCPServ.java
```

# Run Java
```bash
java SqTCPServ
```

```bash
telnet localhost 5000
```
