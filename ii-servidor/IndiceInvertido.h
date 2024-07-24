#ifndef INDICEINVERTIDO_H
#define INDICEINVERTIDO_H

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
    Trie();
    void insertar(const string& palabra, const string& nombreArchivo);
    unordered_set<string> buscar(const string& palabra) const;
};


// Función para recolectar los archivos de texto
unordered_map<string, string> recolectarArchivos(const vector<string>& nombresArchivos);

// Función para eliminar signos de puntuación y saltos de linea
string eliminarSignos(const string& texto);

// Función para tokenizar un texto
vector<string> tokenizarTexto(const string& texto);

// Función para eliminar palabras que no brindan información
vector<string> eliminarStopWords(const vector<string>& listaPalabras, const unordered_set<string>& stopWords);

// Estructura auxiliar para almacenar la palabra y el nombre del archivo
struct PalabraArchivo {
    string palabra;
    string nombreArchivo;
};

// Función para mapear los archivos procesados
vector<PalabraArchivo> mapearArchivos(const unordered_map<string, vector<string>>& archivosProcesados);

// Organización de los datos intermedios
unordered_map<string, vector<string>> shuffle(const vector<PalabraArchivo>& datosMapeados);

// Reducir combinando listas de nombre de los archivos para cada palabra usando un Trie
void reducirDatos(const unordered_map<string, vector<string>>& datosAgrupados, Trie& trie);

// Procesar entrada
unordered_set<string> procesarEntrada(const Trie& trie, const string& entrada);

// Función para crear índice invertido
void crearIndiceInvertido(const vector<string>& nombresArchivos, Trie& trie);

#endif // INDICEINVERTIDO_H
