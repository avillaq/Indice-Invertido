#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> // Para memset

using namespace std;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Socket creation error" << endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "192.168.1.3", &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" << endl;
        return -1;
    }

    string input;
    while (true) {
        cout << "Ingrese las palabras a buscar (formato: palabra1 operador palabra2, o 'exit' para salir): ";
        getline(cin, input);

        if (input == "exit") {
            break;
        }

        send(sock, input.c_str(), input.length(), 0);

        int valread = read(sock, buffer, 1024);
        if (valread > 0) {
            cout << "Archivos encontrados:\n" << buffer << endl;
        } else {
            cout << "No se recibió respuesta del servidor." << endl;
        }

        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}