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

// Función para recolectar documentos de un directorio
unordered_map<string, string> collect_documents(const string& directory) {
    unordered_map<string, string> documents;
    struct dirent *entry;
    DIR *dp = opendir(directory.c_str());

    if (dp == nullptr) {
        perror("opendir");
        return documents;
    }

    while ((entry = readdir(dp))) {
        string filename = entry->d_name;
        if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".txt") {
            ifstream file(directory + "/" + filename);
            if (file) {
                stringstream buffer;
                buffer << file.rdbuf();
                documents[filename] = buffer.str();
            }
        }
    }

    closedir(dp);
    return documents;
}

// Función para convertir texto a minúsculas
string to_lower(const string& text) {
    string lower_text = text;
    transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    return lower_text;
}

// Función para eliminar signos de puntuación y caracteres no alfanuméricos
string preprocess_text(const string& text) {
    string processed_text;
    for (char ch : text) {
        if (isalnum(ch) || isspace(ch)) {
            processed_text += ch;
        }
    }
    return processed_text;
}

// Función para tokenizar texto
vector<string> tokenize(const string& text) {
    vector<string> words;
    istringstream stream(text);
    string word;
    while (stream >> word) {
        words.push_back(word);
    }
    return words;
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
    string directory = "D:/Proyecto vscode/OTROS/MapReduce-Python/IndiceInvertido"; 
    unordered_set<string> stop_words = {"el", "la", "y", "de", "a", "en", "un", "por", "con", "no", "una", "su", "para", "es", "al", "lo", "como", "más", "pero", "sus", "le", "ya", "o", "sí", "porque", "esta", "entre", "cuando", "muy", "sin", "sobre", "también", "me", "hasta", "hay", "donde", "quien", "desde", "todo", "nos", "durante", "todos", "uno", "les", "ni", "contra", "otros", "ese", "eso", "ante", "ellos", "e", "esto", "mí", "antes", "algunos", "qué", "unos", "yo", "otro", "otras", "otra", "él", "tanto", "esa", "estos", "mucho", "quienes", "nada", "muchos", "cual", "poco", "ella", "estar", "estas", "algunas", "algo", "nosotros", "mi", "mis", "tú", "te", "ti", "tu", "tus", "ellas", "nosotras", "vosotros", "vosotras", "os", "mío", "mía", "míos", "mías", "tuyo", "tuya", "tuyos", "tuyas", "suyo", "suya", "suyos", "suyas", "nuestro", "nuestra", "nuestros", "nuestras", "vuestro", "vuestra", "vuestros", "vuestras", "esos", "esas", "estoy", "estás", "está", "estamos", "estáis", "están", "esté", "estés", "estemos", "estéis", "estén", "estaré", "estarás", "estará", "estaremos", "estaréis", "estarán", "estaría", "estarías", "estaríamos", "estaríais", "estarían", "estaba", "estabas", "estábamos", "estabais", "estaban", "estuve", "estuviste", "estuvo", "estuvimos", "estuvisteis", "estuvieron", "estuviera", "estuvieras", "estuviéramos", "estuvierais", "estuvieran", "estuviese", "estuvieses", "estuviésemos", "estuvieseis", "estuviesen", "estando", "estado", "estada", "estados", "estadas", "estad"};  
   

    auto documents = collect_documents(directory);

    unordered_map<string, vector<string>> processed_documents;
    for (const auto& [doc_id, text] : documents) {
        string processed_text = preprocess_text(to_lower(text));
        auto words = tokenize(processed_text);
        auto filtered_words = remove_stop_words(words, stop_words);
        processed_documents[doc_id] = filtered_words;
    }

    auto map_output = map_phase(processed_documents);
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
