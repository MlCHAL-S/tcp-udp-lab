import java.io.*;
import java.net.*;

public class SqTCPServ {

    private static final int PORT = 5000;

    public static void main(String[] args) {
        new SqTCPServ().startServer();
    }

    public void startServer() {
        System.out.println("Server listening on port " + PORT);
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("New client connected");
                new Thread(new ClientHandler(clientSocket)).start();
            }
        } catch (IOException e) {
            System.err.println("I/O error: " + e.getMessage());
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket clientSocket;

        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }

        @Override
        public void run() {
            try (
                BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()))
            ) {
                char[] cbuf = new char[100];
                String v = new String(cbuf, 0, reader.read(cbuf, 0, cbuf.length)).trim();
                System.out.println("Received: " + v);

                int i;
                try {
                    i = Integer.parseInt(v);
                } catch (NumberFormatException e) {
                    System.err.println("Received non-integer input from client.");
                    writer.write("Error: Input is not an integer.\n");
                    writer.flush();
                    return;
                }

                int squared = i * i;
                writer.write(String.valueOf(squared) + "\n");
                writer.flush();
                System.out.println("Sent: " + squared);
            } catch (IOException e) {
                System.err.println("I/O error with client: " + e.getMessage());
            } finally {
                try {
                    clientSocket.close();
                } catch (IOException e) {
                    System.err.println("Error closing client socket: " + e.getMessage());
                }
                System.out.println("Client disconnected");
            }
        }
    }
}
