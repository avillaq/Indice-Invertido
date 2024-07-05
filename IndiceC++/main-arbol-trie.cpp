#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <dirent.h>
using namespace std;

// Nodo del Trie
struct Node {
    unordered_map<char, Node*> children;
    unordered_set<string> doc_ids;  // Identificadores de documentos donde aparece la palabra
};

// Clase Trie
class Trie {

private:
    Node* root;

public:
    Trie() {
        root = new Node();
    }

    // Insertar una palabra y su doc_id en el Trie
    void insert(const string& palabra, const string& doc_id) {
        Node* node = root;
        for (char letra : palabra) {
            if (!node->children.count(letra)) {
                node->children[letra] = new Node();
            }
            node = node->children[letra];
        }
        node->doc_ids.insert(doc_id);
    }

    // Buscar documentos que contienen la palabra
    unordered_set<string> search(const string& palabra) {
        Node* node = root;
        for (char letra : palabra) {
            if (!node->children.count(letra)) {
                return {};
            }
            node = node->children[letra];
        }
        return node->doc_ids;
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
vector<string> remove_stop_words(const vector<string>& words, const unordered_set<string>& stop_words) {
    vector<string> filtered_words;
    for (const string& word : words) {
        if (stop_words.find(word) == stop_words.end()) {
            filtered_words.push_back(word);
        }
    }
    return filtered_words;
}

// Fase Map: Asignación de pares clave-valor
vector<pair<string, string>> map_phase(const unordered_map<string, vector<string>>& processed_documents) {
    vector<pair<string, string>> map_output;
    for (const auto& [doc_id, words] : processed_documents) {
        for (const string& word : words) {
            map_output.emplace_back(word, doc_id);
        }
    }
    return map_output;
}

// Organización de los datos intermedios: Agrupación por clave (palabra)
unordered_map<string, vector<string>> shuffle_and_sort(const vector<pair<string, string>>& map_output) {
    unordered_map<string, vector<string>> grouped_data;
    for (const auto& [word, doc_id] : map_output) {
        grouped_data[word].push_back(doc_id);
    }
    return grouped_data;
}

// Fase Reduce: Combinar listas de identificadores de documentos para cada palabra usando Trie
Trie reduce_phase_trie(const unordered_map<string, vector<string>>& grouped_data) {
    Trie trie;
    for (const auto& [word, doc_ids] : grouped_data) {
        for (const auto& doc_id : doc_ids) {
            trie.insert(word, doc_id);
        }
    }
    return trie;
}


// Función para buscar palabras en el índice invertido
unordered_set<string> search_inverted_index(const string& word, const unordered_map<string, unordered_set<string>>& inverted_index) {
    auto it = inverted_index.find(word);
    if (it != inverted_index.end()) {
        return it->second;
    }
    return {};
}

int main() {
    unordered_set<string> stop_words = {"el", "la", "y", "de", "a", "en", "un", "por", "con", "no", "una", "su", "para", "es", "al", "lo", "como", "más", "pero", "sus", "le", "ya", "o", "sí", "porque", "esta", "entre", "cuando", "muy", "sin", "sobre", "me", "hasta", "hay", "donde", "quien", "desde", "todo", "nos", "uno", "les", "ni", "contra", "otros", "ese", "eso", "ante", "ellos", "e", "esto", "mí", "antes",  "qué", "unos", "yo", "otro", "otras", "otra", "él", "tanto", "esa", "estos", "mucho", "cual", "poco", "ella", "estar", "estas", "mi", "mis", "tú", "te", "ti", "tu", "tus", "ellas"};  

    // Nombre de los documentos a procesar
    vector<string> nombresArchivos = {"El senor doctor.txt", "La caja misteriosa.txt", "La mansion del misterio.txt", "La noche eterna.txt"};

    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos);

    // Mostrar los documentos recolectados (Prueba)
    cout << "Documentos recolectados:" << endl;
    for (const auto& [doc_id, text] : archivosRecolectados) {
        cout << "- " << doc_id << endl;
        cout << text << endl;
    }

    return 0;

    unordered_map<string, vector<string>> archivosProcesados;
    
    for (auto& [nombre, texto] : archivosRecolectados) {
        vector<string> listaPalabras = tokenizarTexto(texto);
        auto filtered_words = remove_stop_words(listaPalabras, stop_words);
        archivosProcesados[nombre] = filtered_words;
    }

    auto map_output = map_phase(archivosProcesados);
    auto grouped_data = shuffle_and_sort(map_output);
    auto trie = reduce_phase_trie(grouped_data);

    // Solicitar al usuario que ingrese una palabra para buscar en el índice
    string word_to_search;
    bool salir = false;

    do
    {
        cout << "Ingrese una palabra para buscar en el índice invertido (o '0' para terminar): ";
        cin >> word_to_search;
        if (word_to_search == "0")
        {
            salir = true;
        }
        else
        {
            auto doc_ids = trie.search(word_to_search);
            if (doc_ids.empty())
            {
                cout << "La palabra '" << word_to_search << "' no se encontró en el índice invertido." << endl;
            }
            else
            {
                cout << "La palabra '" << word_to_search << "' se encontró en los siguientes documentos:" << endl;
                for (const string& doc_id : doc_ids)
                {
                    cout << "- " << doc_id << endl;
                }
            }
        }
    } while (!salir);
    

    return 0;
}
