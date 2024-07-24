#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include "IndiceInvertido.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);  // Constructor del widget
    ~Widget();  // Destructor del widget

private slots:
    void on_iniciar_clicked();  // Slot para iniciar el servidor
    void on_detener_clicked();  // Slot para detener el servidor
    void on_limpiarlog_clicked();  // Slot para limpiar el log
    void manejarConexion();  // Slot para manejar nuevas conexiones de clientes
    void manejarDatos();  // Slot para manejar los datos recibidos de los clientes
    void manejarDesconexion();  // Slot para manejar la desconexion de clientes

private:
    void iniciarServidor(quint16 puerto);  // Metodo para iniciar el servidor en el puerto especificado
    void detenerServidor();  // Metodo para detener el servidor
    QString obtenerDireccionIP();  // Metodo para obtener la direccion IP local

    Ui::Widget *ui;  // Puntero a la interfaz de usuario
    QTcpServer *server;  // Puntero al servidor TCP
    QList<QTcpSocket*> clientesSockets;  // Lista para gestionar múltiples conexiones de clientes
    Trie trie;  // Estructura de datos para el índice invertido
};

#endif // WIDGET_H
