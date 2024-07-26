# INTERFAZ DE INDICE INVERTIDO CON QTCREATOR

## Descripción

Este repositorio contiene dos aplicaciones desarrolladas en C++ utilizando Qt: `ii-cliente` y `ii-servidor`. Estas aplicaciones están diseñadas para comunicarse a través de sockets TCP, proporcionando una interfaz gráfica para el cliente y el servidor.

## Tecnologías Usadas

- **C++**: Lenguaje de programación principal.
- **QtCreator**: Entorno de desarrollo integrado (IDE) utilizado para desarrollar la interfaz gráfica.
- **QTcpSocket**: Clase de Qt utilizada para la comunicación a través de sockets TCP.
- **QWidget**: Clase de Qt utilizada para crear las interfaces gráficas.

## Estructura del Proyecto

- `IndiceC++`: Primera implementación en consola del índice invertido.
- `ii-cliente`: Código fuente del cliente desarrollado con QtCreator.
- `ii-servidor`: Código fuente del servidor desarrollado con QtCreator.
- `ejecutables`: Contiene los ejecutables del cliente y servidor para Linux y Windows.

## Instrucciones de Uso

### Opción 1: Usar los Ejecutables

1. **Ubicar los ejecutables** en la carpeta `ejecutables`.
2. **Descomprimir** los archivos `.zip` según tu sistema operativo (Linux o Windows):
    - `ii-cliente-elf.zip` y `ii-servidor-elf.zip` para Linux.
    - `ii-cliente-exe.zip` y `ii-servidor-exe.zip` para Windows.
3. **Ejecutar** los archivos descomprimidos:
    - En Linux: `./ii-cliente` y `./ii-servidor`.
    - En Windows: `ii-cliente.exe` y `ii-servidor.exe`.

### Opción 2: Compilar el Código Fuente

#### Pasos en Windows

1. **Instalar Qt y QtCreator**:
    - Descarga e instala Qt desde [Qt Downloads](https://www.qt.io/download).
    - Asegúrate de incluir el componente "MSVC" correspondiente a tu versión de Visual Studio.

2. **Abrir el proyecto en QtCreator**:
    - Navega a la carpeta `ii-cliente` o `ii-servidor`.

3. **Compilar y ejecutar**:
    - Haz clic en `Construir` > `Construir Proyecto` para compilar.
    - Haz clic en `Ejecutar` para ejecutar el cliente o servidor.

#### Pasos en Linux

1. **Instalar Qt y QtCreator**:
    - Abre una terminal y ejecuta los siguientes comandos:
    ```bash
    sudo apt-get update
    sudo apt-get install qtcreator qt5-default build-essential
    ```

3. **Compilar y ejecutar desde la consola**:
    - Abre una terminal y navega a la carpeta `ii-cliente` o `ii-servidor`.
    - Usa `qmake` para generar los archivos de construcción:
      ```bash
      qmake
      ```
    - Usa `make` para compilar el proyecto:
      ```bash
      make
      ```
    - Ejecuta el cliente o servidor:
      ```bash
      ./ii-cliente
      ```
      o
      ```bash
      ./ii-servidor
      ```

## Nota Importante

### Uso de la Carpeta `textos`

Esta nota importante se aplica únicamente si se decide compilar el código.

Para el correcto funcionamiento del servidor, es necesario que la carpeta `textos`, que contiene los archivos de texto con los que trabaja el servidor, esté ubicada en el mismo directorio que el ejecutable generado por la compilación, tanto en Windows como en Linux. 

#### Instrucciones

1. **Compilación en Windows**:
    - Después de compilar el servidor, copia la carpeta `textos` al mismo directorio donde se encuentra el ejecutable `ii-servidor.exe`. 
    - Estructura esperada:
    ```
    ├── ii-servidor.exe
    └── textos
        ├── archivo.txt
        └── ...
    ```

2. **Compilación en Linux**:
    - Después de compilar el servidor, copia la carpeta `textos` al mismo directorio donde se encuentra el ejecutable `ii-servidor`. 
    - Estructura esperada:
    ```
    ├── ii-servidor
    └── textos
        ├── archivo.txt
        └── ...
    ```

### Índice Invertido en Consola

La carpeta `IndiceC++` contiene la primera implementación del índice invertido en consola. Se puede encontrar el código fuente para probar esta funcionalidad básica.

## Conexion entre multiple usuarios

### Instrucciones

    - Verficar si estan conectados en la misma red
    - La direccion IP y puerto deben ser el mismo
    
1. **En Windows**
    - Verificar IP Servidor dentro de Simbolo de Sistema en Windows con el comando
        ```bash
      ipconfig
        ```
    - Verificar que la IP sea la misma que IPv4


2. **En Linux**
    - Verificar IP Servidor dentro de Bash en Linux
        ```bash
      ifconfig
        ```
    - Verificar que la IP sea la misma que IPv4
