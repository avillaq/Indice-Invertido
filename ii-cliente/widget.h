#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void conectarServidor();  // Slot para conectar al servidor
    void enviarConsulta();    // Slot para enviar una consulta al servidor
    void leerRespuesta();    // Slot para leer la respuesta del servidor

private:
    void handleDisconnection();  // Maneja la desconexion del servidor
    Ui::Widget *ui;              // Puntero a la interfaz de usuario
    QTcpSocket *socket;         // Puntero al socket de red
};

#endif // WIDGET_H
