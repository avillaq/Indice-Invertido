#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Nodo del Trie
struct Node {
    unordered_map<char, Node*> children; // hijo, contiene un caracter y una referencia a un hijo
    unordered_set<string> nombresArchivos;  // archivos que contienen la palabra hasta ese punto
};

// Clase Trie
class Trie {

private:
    Node* root; // nodo inicial

public:
    Trie() {
        root = new Node(); // constructor, crea nodo inicial
    }

    // Insertar una palabra y el nombre de su archivo en el Trie
    void insertar(const string& palabra, const string& nombreArchivo) { 
        Node* node = root; // nodo inicial
        for (char letra : palabra) { // para cada letra en nuestra palabra
            if (!node->children.count(letra)) { // si existe aun un hijo con esa letra
                node->children[letra] = new Node(); // creamos un nuevo nodo hijo con dicha letra
            }
            node = node->children[letra]; // nos movemos al nodo hijo que contiene la letra
        }
        node->nombresArchivos.insert(nombreArchivo); // insertamos el nombre del archivo (final de palabra)
    }

    // Buscar los archivos que contienen la palabra
    unordered_set<string> buscar(string& palabra) {
        Node* node = root; // nodo inicial
        for (char letra : palabra) { // para cada letra en nuestra palabra
            if (!node->children.count(letra)) { // si no existe un hijo con esa letra
                return {}; // no existe, devolvemos un conjunto vacío
            }
            node = node->children[letra]; // nos movemos al nodo hijo que contiene la letra
        }
        return node->nombresArchivos; // retorna los archivos a los que pertenece el ultimo nodo (final de palabra)
    }


};

// Función para recolectar los archivos de texto
unordered_map<string, string> recolectarArchivos(vector<string> &nombresArchivos) {
    unordered_map<string, string> archivosRecolectados; // mapa de archivos recolectados (nombre archivo - contenido)
    for (string& nombre : nombresArchivos) {  // para cada archivo
        ifstream archivoEntrada(nombre); // lo lee (modo lectura ifstream)
        if (archivoEntrada) { // si se pudo abrir
            stringstream texto; // se crea un stream para el texto
            texto << archivoEntrada.rdbuf(); // se almacena el contenido en 'texto'
            archivosRecolectados[nombre] = texto.str(); // el texto se almacena en el nombre del archivo en 'archivosRecolectados'
        } else { // no se pudo abrir
            cerr << "Error al abrir el archivo: " << nombre << endl; // imprime mensaje error
        }
    }
    return archivosRecolectados; // retorna los archivos recolectados
}

// Función para eliminar signos de puntuación y saltos de linea
string eliminarSignos(string& texto) {
    string nuevoTexto; // almacena el nuevo texto
    for (char caracter : texto) { // para cada caracter en el texto
        if (isalnum(caracter) || caracter == ' ') { // si es alfanumerico o espacio en blanco
            nuevoTexto += caracter; // lo añade a nuevoTexto
        } else if (caracter == '\n') { // si es salto de linea
            nuevoTexto += " "; // añade un espacio
        }
    }
    return nuevoTexto; // retorna el nuevo texto
}

// Función para tokenizar un texto (separa palabra por palabra)
vector<string> tokenizarTexto(string& texto) {
    vector<string> listaPalabras; // lista de palabras
    
    // istringstream sirve para tratar un string como si fuera un stream de entrada
    istringstream stream(texto); // se crea un stream para el texto
    string palabra; // almacena una palabra

    // Se extraen palabra por palabra del stream y se almacenan en la lista
    while (stream >> palabra) { // mientras se pueda extraer una palabra
        listaPalabras.push_back(palabra);  // esa palabra se almacena en la lista
    }
    return listaPalabras; // retorna la lista de palabras (vector)
}

// Función para eliminar palabras que no brindan informacion
vector<string> eliminarStopWords(vector<string>& listaPalabras, unordered_set<string>& stopWords) {
    vector<string> palabrasFiltradas; // almacena las palabras filtradas
    for (string& palabra : listaPalabras) { // para cada palabra en nuestra lista de palabras
        // Se verifica si la palabra no está en el conjunto de palabras vacías
        if (stopWords.find(palabra) == stopWords.end()) { // si esa palabra no esta en los StopWords
            palabrasFiltradas.push_back(palabra); // la palabra se almacena en palabrasFiltradass
        }
    }
    return palabrasFiltradas; // retorna las palabras filtradas
}

// Estructura auxiliar para almacenar la palabra y el nombre del archivo
struct PalabraArchivo { 
    string palabra;
    string nombreArchivo;
};

// Función para mapear los archivos procesados
vector<PalabraArchivo> mapearArchivos(unordered_map<string, vector<string>>& archivosProcesados) {
    PalabraArchivo pa; // usamos la estructura 'PalabraArchivo' para almacenar el nombre de archivo y la palabra
    vector<PalabraArchivo> datosMappeados; // vector de palabraArchivo
    // para cada nombre de archivo y lista de palabras en los archivos procesados
    for (auto& [nombre, listaPalabras] : archivosProcesados) { 
        for (string& palabra : listaPalabras) { // para cada palabra en la lista de palabras
            pa.palabra = palabra; // la palabra se almacena en la palabra de la palabraArchivo
            pa.nombreArchivo = nombre; // el nombre de archivo se almacena en el nombre de archivo de la palabraArchivo
            datosMappeados.push_back(pa); // la palabraArchivo se almacena en datosMappeados
        }
    }
    return datosMappeados; // retorna los datosMappeados (vector de PalabraArchivo)
}

// Organización de los datos intermedios: Agrupación por clave (palabra, palabra, ...)
unordered_map<string, vector<string>> shuffle(vector<PalabraArchivo>& datosMapeados) {
    unordered_map<string, vector<string>> datosAgrupados; // almacena los datos agrupados (palabra - nombre archivo)
    for (auto& dato : datosMapeados) { // para cada palabraArchivo de nuestro vector de palabraArchivo
        datosAgrupados[dato.palabra].push_back(dato.nombreArchivo); // la palabra se almacena como clave y el nombre de archivo como valor
        // esta asignacion evita repeticiones
    }
    return datosAgrupados; // retorna los datos agrupados
}

// Reducir combinando listas de nombre de los archivos para cada palabra usando un Trie
void reducirDatos(unordered_map<string, vector<string>>& datosAgrupados, Trie& trie) {
    for (auto& [palabra, nombreArchivo] : datosAgrupados) { // para cada palabra y lista de archivos en los datos agrupados
        for (auto& nom : nombreArchivo) { // para cada nombre de archivo por palabra
            // insertamos la palabra y los nombres de los archivos a los que pertenece en el Trie
            trie.insertar(palabra, nom); 
        }
    }
}

unordered_set<string> procesarEntrada(Trie& trie ,string& entrada){ // procesa la entrada
    istringstream stream(entrada); // se crea un stream para la entrada
    string palabra1; 
    stream >> palabra1; // se extrae la primer palabra
    string operador;
    stream >> operador; // se extrae el operador
    string palabra2;
    stream >> palabra2; // se extrae la segunda palabra

    if (operador == "AND" || operador == "and") { // si es AND (deben estar si o si las 2 palabras)
        // hacemos doble busqueda en el trie
        unordered_set<string> archivosEncontrados1 = trie.buscar(palabra1); 
        unordered_set<string> archivosEncontrados2 = trie.buscar(palabra2);
        unordered_set<string> interseccionArchivos;
        for (const string& nombres : archivosEncontrados1) { // para los nombres encontrados en la primera busqueda
            // el metodo find devuelve un iterador al elemento si lo encuentra, si no, devuelve un iterador al final ( end() )
            if (archivosEncontrados2.find(nombres) != archivosEncontrados2.end()) { // si el nombre del archivo esta en la segunda busqueda
                interseccionArchivos.insert(nombres); // lo inserta en interseccionArchivos
            }
        }
        return interseccionArchivos; // retorna la interseccionArchivos
    } else if (operador == "OR" || operador == "or") { // si el operador es OR (debe contener almenos una de las palabras)
        // hacemos doble busqueda en el trie
        unordered_set<string> archivosEncontrados1 = trie.buscar(palabra1); 
        unordered_set<string> archivosEncontrados2 = trie.buscar(palabra2);
        
        archivosEncontrados1.merge(archivosEncontrados2); // combina los resultados de ambas busquedas
        return archivosEncontrados1; // retorna los archivos encontrados
    } else { // si no hay operador
        unordered_set<string> archivosEncontrados1 = trie.buscar(palabra1); // busca la primer palabra
        return archivosEncontrados1; // retorna los archivos encontrados
    }
    
}
void crearIndiceInvertido(vector<string> nombresArchivos, Trie trie) {
    mutex mx ;
    // Cargamos las StopWords del archivo
    ifstream archivoEntrada("stop_words_spanish.txt"); // archivo de palabras vacias (no aportan informacion)
    unordered_set<string> stopWords; // almacena las palabras vacías
    if (archivoEntrada) { // si el archivo de StopWords se pudo abrir
        string palabra; 
        while (getline(archivoEntrada, palabra)) { // para cada palabra en el archivo
            stopWords.insert(palabra); // la palabra se almacena en 'stopWords'
        }
    } else { // en caos no se pudo abrir
        cerr << "Error al abrir el archivo de palabras vacias." << endl;// mensaje de error
    } 
    // recolectamos los archivos (nombre - contenido)
    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos); 

    unordered_map<string, vector<string>> archivosProcesados; // contendra los archivos procesados
    for (auto& [nombre, texto] : archivosRecolectados) { // para cada archivo en archivosRecolectados
        texto = eliminarSignos(texto); // elimina los signos en los textos
        vector<string> listaPalabras = tokenizarTexto(texto); // separa las palabras en el texto
        vector<string> palabrasFiltradas = eliminarStopWords(listaPalabras, stopWords); // elimina las palabras vacías
        archivosProcesados[nombre] = palabrasFiltradas; // la palabra filtrada se almacena en archivosProcesados
    }

    // los mapeamos (pasan a estar en estructura PalabraArchivo contiene palabra y nombre del archivo) 
    vector<PalabraArchivo> datosMapeados = mapearArchivos(archivosProcesados);

    // los ordenamos, pasan a estar de nuevo en clave-valor, ahora sin repeticiones
    unordered_map<string, vector<string>> datosAgrupados = shuffle(datosMapeados);// 
    
    // Usamos mutex para controlar la insercion al trie con los hilos
    lock_guard<mutex> lock(mx);
    reducirDatos(datosAgrupados, trie); // ingresamos el indice invertido en el trie

}

void manejarCliente(SOCKET clienteSocket, Trie& trie) {
    char buffer[1024] = {0};

    while (true) {
        int valread = recv(clienteSocket, buffer, 1024, 0);
        if (valread <= 0) {
            cerr << "Error al leer del socket o cliente desconectado" << endl;
            break;
        }

        string entrada(buffer);
        unordered_set<string> resultados = procesarEntrada(trie, entrada);
        string respuesta;
        if (resultados.empty()) {
            respuesta = "No se encontraron resultados.";
        } else {
            for (const string& archivo : resultados) {
                respuesta += archivo + "\n";
            }
        }

        if (send(clienteSocket, respuesta.c_str(), respuesta.length(), 0) == SOCKET_ERROR) {
            cerr << "Error al enviar la respuesta: " << WSAGetLastError() << endl;
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    closesocket(clienteSocket);
}

int main() {
    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "WSAStartup failed: " << iResult << endl;
        return 1;
    }


    // Iniciamos el cronómetro para medir tiempo de ejecucion
    auto start = std::chrono::high_resolution_clock::now();

    

    // Nombre de los documentos a procesar
    vector<vector<string>> nombresArchivos = { // nombres de archivos a procesar
        {"17 LEYES DEL TRABAJO EN EQUIPO - JOHN C. MAXWELL.txt"},
        {"21 LEYES DEL LIDERAZGO - JOHN C. MAXWELL.txt"},
        {"25 MANERAS DE GANARSE A LA GENTE - JOHN C. MAXWELL.txt"},
        {"ACTITUD DE VENCEDOR - JOHN C. MAXWELL.txt"},
        {"El Oro Y La Ceniza - Abecassis Eliette.txt"},
        {"La ultima sirena - Abe ShanaLa.txt"},
        {"SEAMOS PERSONAS DE INFLUENCIA - JOHN MAXWELL.txt"},
        {"VIVE TU SUENO - JOHN MAXWELL.txt"}
    };

    Trie trie; // creamos el trie
    thread threads[8]; // usaremos 8 hilos
    for (int i = 0; i < 8; ++i) { // a cada hilo, le asignamos un archivo a leer
        threads[i] = thread(crearIndiceInvertido, nombresArchivos[i] , trie);
    }

    // Esperamos a que todos los hilos terminen
    for (int i = 0; i < 8; ++i) {
        if (threads[i].joinable()){
            threads[i].join(); // unimos los hilos al principal cuando estos terminen
        } else {
            cerr << "Error al unir el hilo " << i << endl;
        }
    }

    

    // Detenemos el cronómetro y mostramos el tiempo transcurrido
    auto stop = std::chrono::high_resolution_clock::now();
    // imprime el tiempo transcurrido en leer los archivos
    cout << "tiempo= " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << endl;



    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cerr << "Socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        cerr << "Setsockopt failed: " << WSAGetLastError() << endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, 3) == SOCKET_ERROR) {
        cerr << "Listen failed: " << WSAGetLastError() << endl;
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) == INVALID_SOCKET) {
            cerr << "Accept failed: " << WSAGetLastError() << endl;
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }

        thread(manejarCliente, new_socket, ref(trie)).detach();
    }

    closesocket(server_fd);
    WSACleanup();

    return 0;
}