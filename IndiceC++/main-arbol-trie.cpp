#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;

// Nodo del Trie
struct Node {
    unordered_map<char, Node*> children;
    unordered_set<string> nombresArchivos;
};

// Clase Trie
class Trie {
private:
    Node* root;

public:
    Trie() {
        root = new Node();
    }

    void insertar(const string& palabra, const string& nombreArchivo) {
        Node* node = root;
        for (char letra : palabra) {
            if (!node->children.count(letra)) {
                node->children[letra] = new Node();
            }
            node = node->children[letra];
        }
        node->nombresArchivos.insert(nombreArchivo);
    }

    unordered_set<string> buscar(const string& palabra) const {
        Node* node = root;
        for (char letra : palabra) {
            if (!node->children.count(letra)) {
                return {};
            }
            node = node->children[letra];
        }
        return node->nombresArchivos;
    }

};

// Mutex global para la sincronización
mutex mx;

// Función para recolectar los archivos de texto
unordered_map<string, string> recolectarArchivos(const vector<string>& nombresArchivos) {
    unordered_map<string, string> archivosRecolectados;
    for (const string& nombre : nombresArchivos) {
        ifstream archivoEntrada(nombre);
        if (archivoEntrada) {
            stringstream texto;
            texto << archivoEntrada.rdbuf();
            archivosRecolectados[nombre] = texto.str();
        } else {
            cerr << "Error al abrir el archivo: " << nombre << endl;
        }
    }
    return archivosRecolectados;
}

// Función para eliminar signos de puntuación y saltos de linea
string eliminarSignos(const string& texto) {
    string nuevoTexto;
    for (char caracter : texto) {
        if (isalnum(caracter) || caracter == ' ') {
            nuevoTexto += caracter;
        } else if (caracter == '\n') {
            nuevoTexto += ' ';
        }
    }
    return nuevoTexto;
}

// Función para tokenizar un texto
vector<string> tokenizarTexto(const string& texto) {
    vector<string> listaPalabras;
    istringstream stream(texto);
    string palabra;
    while (stream >> palabra) {
        listaPalabras.push_back(palabra);
    }
    return listaPalabras;
}


// Función para eliminar palabras que no brindan información
vector<string> eliminarStopWords(const vector<string>& listaPalabras, const unordered_set<string>& stopWords) {
    vector<string> palabrasFiltradas;
    for (const string& palabra : listaPalabras) {
        if (stopWords.find(palabra) == stopWords.end()) {
            palabrasFiltradas.push_back(palabra);
        }
    }
    return palabrasFiltradas;
}

// Estructura auxiliar para almacenar la palabra y el nombre del archivo
struct PalabraArchivo {
    string palabra;
    string nombreArchivo;
};

// Función para mapear los archivos procesados
vector<PalabraArchivo> mapearArchivos(const unordered_map<string, vector<string>>& archivosProcesados) {
    vector<PalabraArchivo> datosMappeados;
    for (const auto& [nombre, listaPalabras] : archivosProcesados) {
        for (const string& palabra : listaPalabras) {
            datosMappeados.push_back({palabra, nombre});
        }
    }
    return datosMappeados;
}

// Organización de los datos intermedios
unordered_map<string, vector<string>> shuffle(const vector<PalabraArchivo>& datosMapeados) {
    unordered_map<string, vector<string>> datosAgrupados;
    for (const auto& dato : datosMapeados) {
        datosAgrupados[dato.palabra].push_back(dato.nombreArchivo);
    }
    return datosAgrupados;
}

// Reducir combinando listas de nombre de los archivos para cada palabra usando un Trie
void reducirDatos(const unordered_map<string, vector<string>>& datosAgrupados, Trie& trie) {
    lock_guard<mutex> lock(mx); // Bloqueo del mutex global
    for (const auto& [palabra, nombresArchivos] : datosAgrupados) {
        for (const auto& nombreArchivo : nombresArchivos) {
            trie.insertar(palabra, nombreArchivo);
        }
    }
}

// Procesar entrada
unordered_set<string> procesarEntrada(const Trie& trie, const string& entrada) {
    istringstream stream(entrada);
    string palabra1, operador, palabra2;
    stream >> palabra1 >> operador >> palabra2;

    if (operador == "AND" || operador == "and") {
        unordered_set<string> archivosEncontrados1 = trie.buscar(palabra1);
        unordered_set<string> archivosEncontrados2 = trie.buscar(palabra2);
        unordered_set<string> interseccionArchivos;
        for (const string& nombres : archivosEncontrados1) {
            if (archivosEncontrados2.find(nombres) != archivosEncontrados2.end()) {
                interseccionArchivos.insert(nombres);
            }
        }
        return interseccionArchivos;
    } else if (operador == "OR" || operador == "or") {
        unordered_set<string> archivosEncontrados1 = trie.buscar(palabra1);
        unordered_set<string> archivosEncontrados2 = trie.buscar(palabra2);
        archivosEncontrados1.merge(archivosEncontrados2);
        return archivosEncontrados1;
    } else {
        return trie.buscar(palabra1);
    }
}

// Función para crear índice invertido
void crearIndiceInvertido(const vector<string>& nombresArchivos, Trie& trie) {
    // Cargamos las StopWords del archivo
    ifstream archivoEntrada("stop_words_spanish.txt"); // archivo de palabras vacías (no aportan información)
    unordered_set<string> stopWords; // almacena las palabras vacías
    if (archivoEntrada) { // si el archivo de StopWords se pudo abrir
        string palabra; 
        while (getline(archivoEntrada, palabra)) { // para cada palabra en el archivo
            stopWords.insert(palabra); // la palabra se almacena en 'stopWords'
        }
    } else { // en caso no se pudo abrir
        cerr << "Error al abrir el archivo de palabras vacías." << endl; // mensaje de error
    } 

    // Recolectamos los archivos (nombre - contenido)
    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos); 

    unordered_map<string, vector<string>> archivosProcesados; // contendrá los archivos procesados
    for (auto& [nombre, texto] : archivosRecolectados) { // para cada archivo en archivosRecolectados
        texto = eliminarSignos(texto); // elimina los signos en los textos
        vector<string> listaPalabras = tokenizarTexto(texto); // separa las palabras en el texto
        vector<string> palabrasFiltradas = eliminarStopWords(listaPalabras, stopWords); // elimina las palabras vacías
        archivosProcesados[nombre] = palabrasFiltradas; // la palabra filtrada se almacena en archivosProcesados
    }

    // Los mapeamos (pasan a estar en estructura PalabraArchivo contiene palabra y nombre del archivo) 
    vector<PalabraArchivo> datosMapeados = mapearArchivos(archivosProcesados);

    // Los ordenamos, pasan a estar de nuevo en clave-valor, ahora sin repeticiones
    unordered_map<string, vector<string>> datosAgrupados = shuffle(datosMapeados); 
    
    // Usamos mutex para controlar la inserción al trie con los hilos
    reducirDatos(datosAgrupados, trie); // ingresamos el índice invertido en el trie
}

// Función principal
int main() {
    auto start = std::chrono::high_resolution_clock::now();

    vector<string> nombresArchivos = {
        "17 LEYES DEL TRABAJO EN EQUIPO - JOHN C. MAXWELL.txt",
        "21 LEYES DEL LIDERAZGO - JOHN C. MAXWELL.txt",
        "25 MANERAS DE GANARSE A LA GENTE - JOHN C. MAXWELL.txt",
        "ACTITUD DE VENCEDOR - JOHN C. MAXWELL.txt",
        "El Oro Y La Ceniza - Abecassis Eliette.txt",
        "La ultima sirena - Abe ShanaLa.txt",
        "SEAMOS PERSONAS DE INFLUENCIA - JOHN MAXWELL.txt",
        "VIVE TU SUENO - JOHN MAXWELL.txt"
    };

    Trie trie;
    vector<thread> threads;
    for (const auto& nombreArchivo : nombresArchivos) {
        threads.emplace_back(crearIndiceInvertido, vector<string>{nombreArchivo}, ref(trie));
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    // Detenemos el cronómetro y mostramos el tiempo transcurrido
    auto stop = std::chrono::high_resolution_clock::now();
    // imprime el tiempo transcurrido en leer los archivos
    cout << "tiempo= " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << endl;



    // Solicitar al usuario que ingrese una palabra para buscar en el índice
    string palabraBuscar; // palabra a buscar por el usuario
    bool salir = false; // variable para detener el bucle
    do { // bucle para pedir palabras al usuario
        cout << "Ingrese una palabra para buscar en el indice invertido (o '0' para terminar): ";
        getline(cin, palabraBuscar); // leemos la palabra
        
        if (palabraBuscar == "0") { // si es 0, salimos del bucle
            salir = true;
        }
        else { // si ingreso una palabra, la buscamos
            unordered_set<string> archivosEncontrados = procesarEntrada(trie,palabraBuscar); // buscamos la palabra y se almacena en archivosEncontrados
            if (archivosEncontrados.empty()) {  // si el resultado es vacío
                cout << "La palabra '" << palabraBuscar << "' no esta en el indice invertido." << endl;
            }
            else { // si el resultado no es vacío, imprime los documentos pertenecientes a la palabra
                cout << "La palabra '" << palabraBuscar << "' esta en los documentos:" << endl;
                for (const string& nombres : archivosEncontrados) {
                    cout << "- " << nombres << endl;
                }
            }
        }
        
    } while (!salir);
    

    return 0; // finaliza el programa
}
