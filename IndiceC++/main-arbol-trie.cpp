#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
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

    // Insertar una palabra y el nombre de su archivo en el Trie
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

    // Buscar los archivos que contienen la palabra
    unordered_set<string> buscar(string& palabra) {
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

// Función para recolectar los archivos de texto
unordered_map<string, string> recolectarArchivos(vector<string> &nombresArchivos) {
    unordered_map<string, string> archivosRecolectados;
    for (string& nombre : nombresArchivos) {
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

// Función para tokenizar un texto
vector<string> tokenizarTexto(string& texto) {
    vector<string> listaPalabras;
    
    // istringstream sirve para tratar un string como si fuera un stream de entrada
    istringstream stream(texto);
    string palabra;

    // Se extraen palabra por palabra del stream y se almacenan en la lista
    while (stream >> palabra) {
        listaPalabras.push_back(palabra);
    }
    return listaPalabras;
}

// Función para eliminar palabras vacías
vector<string> eliminarStopWords(vector<string>& listaPalabras, unordered_set<string>& stopWords) {
    vector<string> palabrasFiltradas;
    for (string& palabra : listaPalabras) {
        // Se verifica si la palabra no está en el conjunto de palabras vacías
        if (stopWords.find(palabra) == stopWords.end()) {
            palabrasFiltradas.push_back(palabra);
        }
    }
    return palabrasFiltradas;
}

// Estructura para almacenar la palabra y el nombre del archivo
struct PalabraArchivo {
    string palabra;
    string nombreArchivo;
};

// Función para mapear los archivos procesados
vector<PalabraArchivo> mapearArchivos(unordered_map<string, vector<string>>& archivosProcesados) {
    PalabraArchivo pa;
    vector<PalabraArchivo> datosMappeados;
    for (auto& [nombre, listaPalabras] : archivosProcesados) {
        for (string& palabra : listaPalabras) {
            pa.palabra = palabra;
            pa.nombreArchivo = nombre;
            datosMappeados.push_back(pa);
        }
    }
    return datosMappeados;
}

// Organización de los datos intermedios: Agrupación por clave (palabra, palabra, ...)
unordered_map<string, vector<string>> shuffle(vector<PalabraArchivo>& datosMapeados) {
    unordered_map<string, vector<string>> datosAgrupados;
    for (auto& dato : datosMapeados) {
        datosAgrupados[dato.palabra].push_back(dato.nombreArchivo);
    }
    return datosAgrupados;
}

// Fase Reduce: Combinar listas de nombre de los archivos para cada palabra usando un Trie
Trie reducirDatos(unordered_map<string, vector<string>>& datosAgrupados) {
    Trie trie;
    for (auto& [palabra, nombreArchivo] : datosAgrupados) {
        for (auto& nom : nombreArchivo) {
            trie.insertar(palabra, nom);
        }
    }
    return trie;
}

int main() {
    unordered_set<string> stopWords = {"el", "la", "y", "de", "a", "en", "un", "por", "con", "no", "una", "su", "para", "es", "al", "lo", "como", "más", "pero", "sus", "le", "ya", "o", "sí", "porque", "esta", "entre", "cuando", "muy", "sin", "sobre", "me", "hasta", "hay", "donde", "quien", "desde", "todo", "nos", "uno", "les", "ni", "contra", "otros", "ese", "eso", "ante", "ellos", "e", "esto", "mí", "antes",  "qué", "unos", "yo", "otro", "otras", "otra", "él", "tanto", "esa", "estos", "mucho", "cual", "poco", "ella", "estar", "estas", "mi", "mis", "tú", "te", "ti", "tu", "tus", "ellas"};  

    // Nombre de los documentos a procesar
    vector<string> nombresArchivos = {"El senor doctor.txt", "La caja misteriosa.txt", "La mansion del misterio.txt", "La noche eterna.txt"};

    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos);

    unordered_map<string, vector<string>> archivosProcesados;
    for (auto& [nombre, texto] : archivosRecolectados) {
        vector<string> listaPalabras = tokenizarTexto(texto);
        vector<string> palabrasFiltradas = eliminarStopWords(listaPalabras, stopWords);
        archivosProcesados[nombre] = palabrasFiltradas;
    }

    vector<PalabraArchivo> datosMapeados = mapearArchivos(archivosProcesados);
    unordered_map<string, vector<string>> datosAgrupados = shuffle(datosMapeados);
    Trie trie = reducirDatos(datosAgrupados);


    // Solicitar al usuario que ingrese una palabra para buscar en el índice
    string nombreArchivo;
    bool salir = false;
    do {
        cout << "Ingrese una palabra para buscar en el índice invertido (o '0' para terminar): ";
        cin >> nombreArchivo;
        if (nombreArchivo == "0") {
            salir = true;
        }
        else {
            unordered_set<string> archivosEncontrados = trie.buscar(nombreArchivo);
            if (archivosEncontrados.empty()) {
                cout << "La palabra '" << nombreArchivo << "' no esta en el índice invertido." << endl;
            }
            else {
                cout << "La palabra '" << nombreArchivo << "' esta en los documentos:" << endl;
                for (const string& nombres : archivosEncontrados) {
                    cout << "- " << nombres << endl;
                }
            }
        }
    } while (!salir);
    

    return 0;
}
