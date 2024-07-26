#include "widget.h"
#include "ui_widget.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QDir>
#include <QStringList>
#include <QMetaObject>
#include <QMetaMethod>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , server(new QTcpServer(this))  // Se crea un nuevo servidor TCP
{
    ui->setupUi(this);

    QString ip = obtenerDireccionIP();  // Obtiene la IP local
    ui->ip->setText(ip);  // Muestra la IP en el campo correspondiente
//ui->ip->setReadOnly(true);  // Hace el campo de IP solo lectura
}


Widget::~Widget() {
    if (server && server->isListening()) {
        detenerServidor();  // Detiene el servidor si está en ejecución antes de destruirlo
    }
    delete server;  // Elimina el servidor TCP
    delete ui;  // Elimina la interfaz de usuario
}

void Widget::on_iniciar_clicked() {
    qDebug() << "on_iniciar_clicked called";
    quint16 puerto = static_cast<quint16>(ui->puerto->text().toUInt());  // Obtiene el puerto del campo de texto

    if (server->isListening()) {
        QMessageBox::warning(this, "Error", "El servidor ya está en ejecución.");  // Muestra una advertencia si el servidor ya está en ejecución
        return;
    }

    iniciarServidor(puerto);  // Llama al método para iniciar el servidor
}

void Widget::on_detener_clicked() {
    qDebug() << "on_detener_clicked called";
    detenerServidor();  // Llama al método para detener el servidor
}

void Widget::on_limpiarlog_clicked() {
    ui->log->clear();  // Limpia el área de log
}

void Widget::iniciarServidor(quint16 puerto) {
    if (server->isListening()) {
        ui->log->append("El servidor ya está en ejecución.");  // Mensaje si el servidor ya está en ejecución
        return;
    }

    QString ip = ui->ip->text();  // Obtiene la IP local del campo de texto

    // Intenta iniciar el servidor en la IP y puerto especificados
    if (!server->listen(QHostAddress(ip), puerto)) {
        ui->log->append("No se pudo iniciar el servidor: " + server->errorString());  // Mensaje de error si no se pudo iniciar el servidor
    } else {
        ui->log->append("Servidor iniciado en " + ip + ":" + QString::number(puerto));  // Mensaje indicando que el servidor está en ejecución
        connect(server, &QTcpServer::newConnection, this, &Widget::manejarConexion);  // Conecta la señal de nueva conexión a la función correspondiente

        // Obtener la ruta del directorio del ejecutable
        QDir dir(QCoreApplication::applicationDirPath());
        QString textosPath = dir.absoluteFilePath("textos");

        // Verificar si la carpeta "textos" existe
        if (!QDir(textosPath).exists()) {
            QMessageBox::critical(this, "Error", "La carpeta 'textos' no existe.");
            return;
        }

        // Cargamos la lista de palabras vacías
        string nombreStopWord = "stop_words_spanish.txt";
        string pathStopWords = textosPath.toStdString() + "/"+nombreStopWord;
        std::ifstream archivoEntrada(pathStopWords);
        std::unordered_set<std::string> stopWords;
        if (archivoEntrada.is_open()) {
            std::string palabra;
            while (std::getline(archivoEntrada, palabra)) {
                stopWords.insert(palabra);
            }
        } else {
            QString mensajeError = "El archivo '"+QString::fromStdString(nombreStopWord)+"' no se encuentra en la carpeta 'textos'.";
            QMessageBox::critical(this, "Error",mensajeError);
            return;
        }

        // Lista de nombres de archivos a cargar
        std::vector<std::string> nombresArchivos = {
            "17 LEYES DEL TRABAJO EN EQUIPO - JOHN C. MAXWELL.txt",
            "21 LEYES DEL LIDERAZGO - JOHN C. MAXWELL.txt",
            "25 MANERAS DE GANARSE A LA GENTE - JOHN C. MAXWELL.txt",
            "ACTITUD DE VENCEDOR - JOHN C. MAXWELL.txt",
            "El Oro Y La Ceniza - Abecassis Eliette.txt",
            "La ultima sirena - Abe ShanaLa.txt",
            "SEAMOS PERSONAS DE INFLUENCIA - JOHN MAXWELL.txt",
            "VIVE TU SUENO - JOHN MAXWELL.txt",
            "Frankenstein-mary-shelley.txt",
            "La Divina Comedia - Dante Alighieri.txt",
            "tom_jones-novelacomica-ingles.txt"
        };

        // Añadir la ruta "textos/" a cada archivo
        for (auto& nombreArchivo : nombresArchivos) {
            nombreArchivo = textosPath.toStdString() + "/" + nombreArchivo;
        }

        crearIndiceInvertido(nombresArchivos, trie, stopWords);  // Carga los archivos en el índice invertido
        ui->log->append("Índice invertido cargado correctamente.");  // Mensaje indicando que el índice invertido se ha cargado
    }
}

void Widget::detenerServidor() {
    if (!server->isListening()) {
        ui->log->append("No hay servidor en ejecución.");  // Mensaje si no hay servidor en ejecución
        return;
    }

    // Cerrar todas las conexiones activas de los clientes
    for (QTcpSocket* socket : clientesSockets) {
        socket->disconnectFromHost();  // Desconecta el socket del host
        socket->waitForDisconnected();  // Espera a que la desconexión se complete
    }

    server->close();  // Cierra el servidor
    ui->log->append("Servidor detenido.");  // Mensaje indicando que el servidor se ha detenido
}

void Widget::manejarConexion() {
    QTcpSocket *nuevoClienteSocket = server->nextPendingConnection();  // Obtiene el siguiente cliente que se conecta
    connect(nuevoClienteSocket, &QTcpSocket::readyRead, this, &Widget::manejarDatos);  // Conecta la señal de datos listos a la función correspondiente
    connect(nuevoClienteSocket, &QTcpSocket::disconnected, this, &Widget::manejarDesconexion);  // Conecta la señal de desconexión a la función correspondiente
    clientesSockets.append(nuevoClienteSocket);  // Añade el nuevo socket a la lista de clientes
    ui->log->append("Nuevo cliente conectado.");  // Mensaje indicando que un nuevo cliente se ha conectado
}

void Widget::manejarDatos() {
    QTcpSocket* clienteSocket = qobject_cast<QTcpSocket*>(sender());  // Obtiene el socket del cliente que envía los datos
    if (!clienteSocket) {
        return;  // Si el socket es nulo, no hace nada
    }

    QByteArray datos = clienteSocket->readAll();  // Lee todos los datos del socket
    QString consulta = QString(datos).trimmed();  // Convierte los datos a QString y elimina espacios en blanco

    ui->log->append("Datos recibidos: " + consulta);  // Muestra los datos recibidos en el log

    // Procesar la consulta utilizando el índice invertido
    std::string consultaStr = consulta.toStdString();
    std::unordered_set<std::string> resultado = procesarEntrada(trie, consultaStr);  // Procesa la consulta

    QString respuesta;
    if (resultado.empty()) {
        respuesta = "No se encontraron resultados para: " + consulta;  // Mensaje si no se encontraron resultados
    } else {
        respuesta = "Archivos encontrados:\n";
        for (const std::string& archivo : resultado) {
            QFileInfo fileInfo(QString::fromStdString(archivo));
            respuesta += QString("   - ") +  fileInfo.fileName() + "\n";  // Añade el nombre del archivo a la respuesta
            ui->log->append("Archivo encontrado: " + QString::fromStdString(archivo));  // Muestra el archivo encontrado en el log
        }
    }

    clienteSocket->write(respuesta.toUtf8());  // Envía la respuesta al cliente
    clienteSocket->flush();  // Asegura que todos los datos se envíen
}

void Widget::manejarDesconexion() {
    QTcpSocket* clienteSocket = qobject_cast<QTcpSocket*>(sender());  // Obtiene el socket del cliente que se ha desconectado
    if (clienteSocket) {
        clientesSockets.removeAll(clienteSocket);  // Elimina el socket de la lista de clientes
        clienteSocket->deleteLater();  // Marca el socket para su eliminación
        ui->log->append("Cliente desconectado.");  // Mensaje indicando que un cliente se ha desconectado
    }
}

QString Widget::obtenerDireccionIP() {
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
            return address.toString();  // Devuelve la dirección IP local
        }
    }
    return QHostAddress(QHostAddress::LocalHost).toString();  // Devuelve la dirección IP local si no se encontró una IP externa
}
