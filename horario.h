#ifndef HORARIO_H
#define HORARIO_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTextStream>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QInputDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class Horario;
}
QT_END_NAMESPACE

class Horario : public QMainWindow
{
    Q_OBJECT

public:
    explicit Horario(QWidget *parent = nullptr);
    ~Horario();

private slots:
    void horas();
    void frecuencia();
    void abrir();
    void guardar();
    void nuevo_horario();

private:
    Ui::Horario *ui;

    void inicializarTabla();

    // Opciones de edición al clickear
    void showContextMenu(const QPoint &pos);
    void editar_celda();
    void eliminar_celda();

    // Funciones auxiliares
    int contarAsignaturas(QString asignatura);
    int contarAsignaturasPorDia(QString dia, QString asignatura);
    bool validarEspacio(QString dia, QString turno, int frecuencia);
    void mostrarTurnosDisponibles();
    void mostrarDiasDisponibles();
    void informarEstadoAsignatura(QString asignatura);
    int contarTurnosDisponibles(QString dia, QString turno);
    int getColumnFromDia(QString dia);
    int getRowFromTurno(QString turno);
    void agregarAsignatura(QString dia, QString turno, QString asignatura, int frecuencia);
};

#endif // HORARIO_H
