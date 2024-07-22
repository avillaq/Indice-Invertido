#include "widget.h"
#include "ui_widget.h"
#include <QHostAddress>
#include <QTcpSocket>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    socket(new QTcpSocket(this))  // Inicializa el socket
{
    ui->setupUi(this);

    // Conecta el boton conectar al slot conectarServidor
    connect(ui->conectar, &QPushButton::clicked, this, &Widget::conectarServidor, Qt::UniqueConnection);
    // Conecta el boton enviar consulta al slot enviarConsulta
    connect(ui->enviarconsulta, &QPushButton::clicked, this, &Widget::enviarConsulta, Qt::UniqueConnection);
    // Conecta el boton limpiar al metodo de limpiar el log
    connect(ui->limpiar, &QPushButton::clicked, ui->log, &QTextEdit::clear, Qt::UniqueConnection);
    // Conecta la señal readyRead del socket al slot leerRespuesta
    connect(socket, &QTcpSocket::readyRead, this, &Widget::leerRespuesta, Qt::UniqueConnection);
    // Conecta la señal disconnected del socket al slot handleDisconnection
    connect(socket, &QTcpSocket::disconnected, this, &Widget::handleDisconnection, Qt::UniqueConnection);
}

Widget::~Widget() {
    // Si el socket esta conectado, desconectalo antes de destruir el socket
    if (socket->state() == QTcpSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
    delete socket;  // Libera memoria del socket
    delete ui;      // Libera memoria de la interfaz de usuario
}

void Widget::conectarServidor() {
    QString ip = ui->ipservidor->text();        // Obtiene la direccion IP del servidor
    quint16 puerto = static_cast<quint16>(ui->puerto->text().toUInt());  // Obtiene el puerto

    socket->connectToHost(QHostAddress(ip), puerto);  // Conecta al servidor

    // Verifica si la conexion fue exitosa
    if (socket->waitForConnected(3000)) {
        ui->log->append("conectado al servidor");  // Muestra mensaje de conexion exitosa
    } else {
        ui->log->append("no se pudo conectar al servidor: " + socket->errorString());  // Muestra mensaje de error
    }
}

void Widget::enviarConsulta() {
    // Verifica si el socket esta conectado antes de enviar datos
    if (socket->state() == QTcpSocket::ConnectedState) {
        QString consulta = ui->consulta->text();  // Obtiene la consulta del usuario
        socket->write(consulta.toUtf8());  // Envía la consulta al servidor
    } else {
        ui->log->append("no esta conectado al servidor o el servidor ha sido detenido");  // Muestra mensaje de error
    }
}

void Widget::leerRespuesta() {
    // Verifica si el socket esta conectado antes de leer datos
    if (socket->state() == QTcpSocket::ConnectedState) {
        QByteArray respuesta = socket->readAll();  // Lee la respuesta del servidor
        ui->arearespuesta->setText(QString(respuesta));  // Muestra la respuesta en la interfaz
    } else {
        ui->log->append("no esta conectado al servidor");  // Muestra mensaje de error
    }
}

void Widget::handleDisconnection() {
    ui->log->append("conexion con el servidor perdida");  // Muestra mensaje de desconexion
    socket->abort();  // Cancela cualquier operacion pendiente
}
