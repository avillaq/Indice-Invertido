#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>
using namespace std;

mutex mx;
vector<unordered_map<string, vector<string>>> datosTotalesAgrupados;
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
// Esta funcion se ejecutara en paralelo
void crearIndiceInvertido(vector<string> nombresArchivos, Trie& trie, unordered_set<string>& stopWords) {
    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos);

    unordered_map<string, vector<string>> archivosProcesados;
    for (auto& [nombre, texto] : archivosRecolectados) {
        texto = eliminarSignos(texto);
        vector<string> listaPalabras = tokenizarTexto(texto);
        vector<string> palabrasFiltradas = eliminarStopWords(listaPalabras, stopWords);
        archivosProcesados[nombre] = palabrasFiltradas;
    }

    // los mapeamos (pasan a estar en estructura PalabraArchivo contiene palabra y nombre del archivo) 
    vector<PalabraArchivo> datosMapeados = mapearArchivos(archivosProcesados);
    unordered_map<string, vector<string>> datosAgrupados = shuffle(datosMapeados);

    // Bloquear el acceso a la variable compartida
    lock_guard<mutex> lock(mx);
    datosTotalesAgrupados.push_back(datosAgrupados);
    
}

int main() {
    // Iniciamos el cronómetro
    auto start = chrono::high_resolution_clock::now();

    // Cargamos las palabras vacias del archivo
    ifstream archivoEntrada("stop_words_spanish.txt");
    unordered_set<string> stopWords;
    if (archivoEntrada) {
        string palabra;
        while (getline(archivoEntrada, palabra)) {
            stopWords.insert(palabra);
        }
    } else {
        cerr << "Error al abrir el archivo de palabras vacias." << endl;
        return 1;
    }

    // Nombre de los documentos a procesar
    vector<vector<string>> nombresArchivos = {
        {"17 LEYES DEL TRABAJO EN EQUIPO - JOHN C. MAXWELL.txt"},
        {"21 LEYES DEL LIDERAZGO - JOHN C. MAXWELL.txt"},
        {"25 MANERAS DE GANARSE A LA GENTE - JOHN C. MAXWELL.txt"},
        {"ACTITUD DE VENCEDOR - JOHN C. MAXWELL.txt"},
        {"El Oro Y La Ceniza - Abecassis Eliette.txt"},
        {"La ultima sirena - Abe ShanaLa.txt"},
        {"SEAMOS PERSONAS DE INFLUENCIA - JOHN MAXWELL.txt"},
        {"VIVE TU SUENO - JOHN MAXWELL.txt"},
        {"Frankenstein-mary-shelley.txt"},
        {"La Divina Comedia - Dante Alighieri.txt"}
    };

    Trie trie;
    int numeroThreads = nombresArchivos.size();
    thread threads[numeroThreads];
    for (int i = 0; i < numeroThreads; ++i) {
        threads[i] = thread(crearIndiceInvertido, nombresArchivos[i] , ref(trie), ref(stopWords));
    }
    // Esperamos a que todos los hilos terminen
    for (int i = 0; i < numeroThreads; ++i) {
        if (threads[i].joinable()){
            threads[i].join();
        } else {
            cerr << "Error al unir el hilo " << i << endl;
        }
    }
    // Insertamos las palabras en el trie
    for (auto& datos : datosTotalesAgrupados) {
        reducirDatos(datos, trie);        
    }

    // Detenemos el cronómetro y mostramos el tiempo transcurrido
    auto stop = chrono::high_resolution_clock::now();
    cout << "tiempo total = " << chrono::duration_cast<chrono::milliseconds>(stop - start).count() << " ms" << endl;



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
