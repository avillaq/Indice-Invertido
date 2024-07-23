#include "IndiceInvertido.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>

using namespace std;

mutex mx;

Trie::Trie() {
    root = new Node();
}

void Trie::insertar(const string& palabra, const string& nombreArchivo) {
    Node* node = root;
    for (char letra : palabra) {
        if (!node->children.count(letra)) {
            node->children[letra] = new Node();
        }
        node = node->children[letra];
    }
    node->nombresArchivos.insert(nombreArchivo);
}

unordered_set<string> Trie::buscar(const string& palabra) const {
    Node* node = root;
    for (char letra : palabra) {
        if (!node->children.count(letra)) {
            return {};
        }
        node = node->children[letra];
    }
    return node->nombresArchivos;
}

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

vector<string> tokenizarTexto(const string& texto) {
    vector<string> listaPalabras;
    istringstream stream(texto);
    string palabra;
    while (stream >> palabra) {
        listaPalabras.push_back(palabra);
    }
    return listaPalabras;
}

vector<string> eliminarStopWords(const vector<string>& listaPalabras, const unordered_set<string>& stopWords) {
    vector<string> palabrasFiltradas;
    for (const string& palabra : listaPalabras) {
        if (stopWords.find(palabra) == stopWords.end()) {
            palabrasFiltradas.push_back(palabra);
        }
    }
    return palabrasFiltradas;
}

vector<PalabraArchivo> mapearArchivos(const unordered_map<string, vector<string>>& archivosProcesados) {
    vector<PalabraArchivo> datosMapeados;
    for (const auto& [nombre, listaPalabras] : archivosProcesados) {
        for (const string& palabra : listaPalabras) {
            datosMapeados.push_back({palabra, nombre});
        }
    }
    return datosMapeados;
}

unordered_map<string, vector<string>> shuffle(const vector<PalabraArchivo>& datosMapeados) {
    unordered_map<string, vector<string>> datosAgrupados;
    for (const auto& dato : datosMapeados) {
        datosAgrupados[dato.palabra].push_back(dato.nombreArchivo);
    }
    return datosAgrupados;
}

void reducirDatos(const unordered_map<string, vector<string>>& datosAgrupados, Trie& trie) {
    lock_guard<mutex> lock(mx);
    for (const auto& [palabra, nombresArchivos] : datosAgrupados) {
        for (const auto& nombreArchivo : nombresArchivos) {
            trie.insertar(palabra, nombreArchivo);
        }
    }
}

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

void crearIndiceInvertido(const vector<string>& nombresArchivos, Trie& trie) {
    ifstream archivoEntrada("stop_words_spanish.txt");
    if (!archivoEntrada) {
        archivoEntrada.open("textos/stop_words_spanish.txt");
    }

    unordered_set<string> stopWords;
    if (archivoEntrada) {
        string palabra;
        while (getline(archivoEntrada, palabra)) {
            stopWords.insert(palabra);
        }
    } else {
        cerr << "Error al abrir el archivo de palabras vacías." << endl;
    }

    unordered_map<string, string> archivosRecolectados = recolectarArchivos(nombresArchivos);

    unordered_map<string, vector<string>> archivosProcesados;
    for (auto& [nombre, texto] : archivosRecolectados) {
        texto = eliminarSignos(texto);
        vector<string> listaPalabras = tokenizarTexto(texto);
        vector<string> palabrasFiltradas = eliminarStopWords(listaPalabras, stopWords);
        archivosProcesados[nombre] = palabrasFiltradas;
    }

    vector<PalabraArchivo> datosMapeados = mapearArchivos(archivosProcesados);

    unordered_map<string, vector<string>> datosAgrupados = shuffle(datosMapeados);

    reducirDatos(datosAgrupados, trie);
}
