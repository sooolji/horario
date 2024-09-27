#include "horario.h"
#include "ui_horario.h"

Horario::Horario(QWidget *parent) : QMainWindow(parent), ui(new Ui::Horario) {
    ui->setupUi(this);
    inicializarTabla();

    // Conectar acciones del menú a sus respectivos slots
    connect(ui->horas, &QAction::triggered, this, &Horario::horas);
    connect(ui->frecuencia, &QAction::triggered, this, &Horario::frecuencia);
    connect(ui->abrir, &QAction::triggered, this, &Horario::abrir);
    connect(ui->guardar, &QAction::triggered, this, &Horario::guardar);
    connect(ui->nuevo_horario, &QAction::triggered, this, &Horario::nuevo_horario);
}

Horario::~Horario() {
    delete ui;
}

void Horario::inicializarTabla() {
    ui->tableWidget->setRowCount(6); // Configuramos 6 filas
    ui->tableWidget->setColumnCount(7); // Configuramos 7 columnas
    QStringList headers = {"Turnos", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado"};
    ui->tableWidget->setHorizontalHeaderLabels(headers); // Establecemos las etiquetas de las columnas

    // Ocultar/Mostrar el encabezado vertical (números de fila)
    ui->tableWidget->verticalHeader()->setVisible(true);

    QStringList timeSlots = {"8:00-9:20", "9:25-10:45", "10:55-12:15", "13:00-14:20", "14:25-15:45", "15:55-16:15"};

    // Llenamos la primera columna con los horarios y establecemos celdas como solo lectura
    for (int row = 0; row < timeSlots.size(); ++row) {
        QTableWidgetItem *item = new QTableWidgetItem(timeSlots[row]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Establecer como no editable
        ui->tableWidget->setItem(row, 0, item); // Establecer el item en la tabla
    }

    // Configurar celdas de otras columnas como no editables
    for (int row = 0; row < timeSlots.size(); ++row) {
        for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = new QTableWidgetItem(); // Crear un nuevo item vacío
            item->setFlags(item->flags() & ~Qt::ItemIsEditable); // Establecer como no editable
            ui->tableWidget->setItem(row, col, item);
        }
    }

    // Conectar el evento de clic derecho
    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &Horario::showContextMenu);
}

void Horario::showContextMenu(const QPoint &pos) {
    QMenu contextMenu(tr("Menu"), this);

    QAction actionEdit("Editar", this);
    QAction actionDelete("Eliminar", this);

    connect(&actionEdit, &QAction::triggered, this, &Horario::editar_celda);
    connect(&actionDelete, &QAction::triggered, this, &Horario::eliminar_celda);

    contextMenu.addAction(&actionEdit);
    contextMenu.addAction(&actionDelete);

    contextMenu.exec(ui->tableWidget->mapToGlobal(pos));
}

void Horario::editar_celda() {
    QTableWidgetItem *item = ui->tableWidget->currentItem(); // Verificamos si hay un ítem seleccionado
    if (item) {
        if (!item->text().isEmpty()) { // Comprobamos si el ítem tiene texto
            bool ok;
            QString text = QInputDialog::getText(this, tr("Editar Celda"), tr("Nuevo texto:"), QLineEdit::Normal, item->text(), &ok);
            if (ok && !text.isEmpty()) {
                item->setText(text);
            }
        } else {
            QMessageBox::information(this, tr("Error"), tr("No se puede editar una celda vacía."));
        }
    }
}

void Horario::eliminar_celda() {
    QTableWidgetItem *item = ui->tableWidget->currentItem();

    if (item) {
        if (item->text().isEmpty()) {
            QMessageBox::warning(this, "Error", "La celda ya está vacía.");
        } else {
            item->setText(""); // Eliminar el texto de la celda
        }
    }
}

void Horario::abrir() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Abrir archivo .csv"), "", tr("(*.csv)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            ui->tableWidget->setRowCount(0); // Limpiamos la tabla

            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList fields = line.split(",");
                int row = ui->tableWidget->rowCount();
                ui->tableWidget->insertRow(row);

                for (int column = 0; column < fields.size(); ++column) {
                    ui->tableWidget->setItem(row, column, new QTableWidgetItem(fields[column]));
                }
            }
            file.close(); // Cerramos el archivo
        } else {
            QMessageBox::warning(this, tr("Error"), tr("No se pudo abrir el archivo"));
        }
    }
}

void Horario::guardar() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Abrir archivo .csv"), "", tr("(*.csv)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            int rowCount = ui->tableWidget->rowCount();
            int columnCount = ui->tableWidget->columnCount();

            for (int row = 0; row < rowCount; ++row) {
                QStringList fields;
                for (int column = 0; column < columnCount; ++column) {
                    QTableWidgetItem *item = ui->tableWidget->item(row, column);
                    if (item) {
                        fields << item->text();
                    } else {
                        fields << "";
                    }
                }
                out << fields.join(",") << "\n"; // Unimos los campos y escribimos en el archivo
            }
            file.close(); // Cerramos el archivo
        } else {
            QMessageBox::warning(this, tr("Error"), tr("No se pudo guardar el archivo"));
        }
    }
}

void Horario::horas() {
    // Crear un diálogo para seleccionar asignaturas
    QDialog dialog(this);
    dialog.setWindowTitle("Seleccionar Asignaturas");

    QVBoxLayout layout(&dialog);
    QListWidget asignaturasListWidget; // Lista para mostrar asignaturas
    asignaturasListWidget.setSelectionMode(QAbstractItemView::MultiSelection);

    // Llenar la lista con las asignaturas de la tabla, evitando duplicados
    QSet<QString> asignaturasSet;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 1; col < ui->tableWidget->columnCount(); ++col) { // Omitir la columna de tiempo
            QString itemText = ui->tableWidget->item(row, col) ? ui->tableWidget->item(row, col)->text() : "";
            if (!itemText.isEmpty() && !asignaturasSet.contains(itemText)) {
                asignaturasSet.insert(itemText);
                QListWidgetItem* item = new QListWidgetItem(itemText);
                asignaturasListWidget.addItem(item);
            }
        }
    }

    layout.addWidget(&asignaturasListWidget);

    // Botón para calcular total de horas
    QPushButton calcularButton("Calcular Total de Horas", &dialog);
    layout.addWidget(&calcularButton);

    // Conectar la señal del botón
    connect(&calcularButton, &QPushButton::clicked, [&]() {
        double totalHoras = 0.0; // Cambiado a double para horas decimales

        // Calcular el total de horas para las asignaturas seleccionadas
        for (const auto& item : asignaturasListWidget.selectedItems()) {
            QString asignatura = item->text();
            int frecuencia = 0;

            // Contar cuántas veces aparece la asignatura en la tabla para calcular la frecuencia
            for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
                for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
                    QString itemText = ui->tableWidget->item(row, col) ? ui->tableWidget->item(row, col)->text() : "";
                    if (itemText == asignatura) {
                        frecuencia++;
                    }
                }
            }

            // Sumar las horas basadas en la frecuencia
            totalHoras += frecuencia * (90.0 / 60.0); // Cada asignatura tiene 90 minutos por clase
        }

        // Mostrar el total de horas en formato decimal
        QMessageBox::information(this, "Total de Horas",
                                 "Total de horas de las asignaturas seleccionadas: " +
                                     QString::number(totalHoras, 'f', 2) + " horas"); // Mostrar con 2 decimales
        dialog.accept(); // Cerrar el diálogo
    });

    dialog.exec(); // Mostrar el diálogo
}

void Horario::frecuencia() {
    // Crear un diálogo para seleccionar asignaturas
    QDialog dialog(this);
    dialog.setWindowTitle("Consultar Frecuencia de Asignaturas");

    QVBoxLayout layout(&dialog);
    QListWidget* asignaturasListWidget = new QListWidget(&dialog); // Lista para mostrar asignaturas
    asignaturasListWidget->setSelectionMode(QAbstractItemView::SingleSelection); // Permitir seleccionar solo una asignatura

    // Llenar la lista con las asignaturas de la tabla, evitando duplicados
    QSet<QString> asignaturasSet;
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        for (int col = 1; col < ui->tableWidget->columnCount(); ++col) { // Omitir la columna de tiempo
            QString itemText = ui->tableWidget->item(row, col) ? ui->tableWidget->item(row, col)->text() : "";
            if (!itemText.isEmpty()) {
                asignaturasSet.insert(itemText); // Agregar al conjunto (evita duplicados)
            }
        }
    }

    // Agregar las asignaturas únicas al QListWidget
    for (const QString& asignatura : asignaturasSet) {
        QListWidgetItem* item = new QListWidgetItem(asignatura);
        asignaturasListWidget->addItem(item);
    }

    layout.addWidget(asignaturasListWidget);

    // Campo para ingresar la frecuencia
    QLineEdit* frecuenciaLineEdit = new QLineEdit(&dialog);
    frecuenciaLineEdit->setPlaceholderText("Ingrese la frecuencia deseada"); // Placeholder para el campo
    layout.addWidget(frecuenciaLineEdit);

    // Botón para mostrar las asignaturas con la frecuencia ingresada
    QPushButton mostrarButton("Mostrar Asignaturas", &dialog);
    layout.addWidget(&mostrarButton);

    // Conectar la señal del botón
    connect(&mostrarButton, &QPushButton::clicked, [&]() {
        bool ok;
        int frecuenciaIngresada = frecuenciaLineEdit->text().toInt(&ok); // Convertir texto a entero

        if (!ok) {
            QMessageBox::warning(&dialog, "Advertencia", "Por favor, ingresa un número válido.");
            return;
        }

        // Verificar si la frecuencia ingresada es mayor a 3
        if (frecuenciaIngresada > 3) {
            QMessageBox::warning(&dialog, "Advertencia", "La frecuencia no puede ser mayor a 3.");
            return;
        }

        asignaturasListWidget->clear(); // Limpiar la lista antes de mostrar resultados

        // Usar un conjunto para evitar duplicados al mostrar resultados
        QSet<QString> resultadosSet;

        // Llenar la lista con las asignaturas que tienen la frecuencia ingresada
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
                QString itemText = ui->tableWidget->item(row, col) ? ui->tableWidget->item(row, col)->text() : "";
                if (!itemText.isEmpty()) {
                    int frecuencia = 0;

                    // Contar cuántas veces aparece la asignatura en la tabla para calcular la frecuencia
                    for (int innerRow = 0; innerRow < ui->tableWidget->rowCount(); ++innerRow) {
                        for (int innerCol = 1; innerCol < ui->tableWidget->columnCount(); ++innerCol) {
                            QString innerItemText = ui->tableWidget->item(innerRow, innerCol) ? ui->tableWidget->item(innerRow, innerCol)->text() : "";
                            if (innerItemText == itemText) {
                                frecuencia++;
                            }
                        }
                    }

                    // Si la frecuencia coincide con la ingresada y no está ya en los resultados, agregar a la lista
                    if (frecuencia == frecuenciaIngresada && !resultadosSet.contains(itemText)) {
                        QListWidgetItem* item = new QListWidgetItem(itemText);
                        asignaturasListWidget->addItem(item);
                        resultadosSet.insert(itemText); // Agregar al conjunto de resultados para evitar duplicados
                    }
                }
            }
        }
    });

    dialog.exec(); // Mostrar el diálogo
}

void Horario::nuevo_horario() {
    // Solicitar al usuario la asignatura
    QString asignatura = QInputDialog::getText(this, "Agregar Asignatura", "Ingrese el nombre de la asignatura:");

    // Solicitar la frecuencia
    bool ok;
    int frecuencia = QInputDialog::getInt(this, "Frecuencia", "Ingrese la frecuencia (1, 2 o 3):", 1, 1, 3, 1, &ok);

    // Validar la frecuencia ingresada
    if (!ok || frecuencia < 1 || frecuencia > 3) {
        QMessageBox::warning(this, "Error", "La frecuencia debe ser 1, 2 o 3.");
        return;
    }

    // Lógica para manejar frecuencias
    for (int i = 0; i < frecuencia; ++i) {
        // Solicitar el día con opción de sugerencia
        QString dia = QInputDialog::getItem(this, "Día", "Seleccione el día o 'Sugerencia' para ver días disponibles:", {"Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Sugerencia"}, 0, false);

        // Solicitar el turno con opción de sugerencia
        QString turno = QInputDialog::getItem(this, "Turno", "Seleccione el turno o 'Sugerencia' para ver turnos disponibles:", {"8:00-9:20", "9:25-10:45", "10:55-12:15", "13:00-14:20", "14:25-15:45", "15:55-16:15", "Sugerencia"}, 0, false);

        // Verificar si la asignatura ya está en el horario
        int totalAsignaturas = contarAsignaturas(asignatura);

        // Validar si hay espacio en el horario
        if (totalAsignaturas >= 3) {
            QMessageBox::warning(this, "Error", "No se puede agregar más instancias de esta asignatura en el horario.");
            return;
        }

        // Si se proporciona un día y un turno
        if (!dia.isEmpty() && !turno.isEmpty() && dia != "Sugerencia" && turno != "Sugerencia") {
            // Nueva validación para permitir solo una vez por día
            if (contarAsignaturasPorDia(dia, asignatura) > 0) {
                QMessageBox::warning(this, "Error", QString("La asignatura '%1' ya ha sido agregada en el día '%2'.").arg(asignatura).arg(dia));
                return;
            }

            if (!validarEspacio(dia, turno, frecuencia)) {
                QMessageBox::information(this, "Sin Espacios", "No hay turnos disponibles en el día seleccionado.");
                return;
            }
            agregarAsignatura(dia, turno, asignatura, frecuencia);
            QMessageBox::information(this, "Éxito", QString("La asignatura '%1' ha sido agregada correctamente en %2 - %3.").arg(asignatura).arg(dia).arg(turno));
        } else if (dia == "Sugerencia") {
            mostrarDiasDisponibles(); // Muestra todos los días disponibles
            return; // Salir para evitar continuar con la lógica normal
        } else if (turno == "Sugerencia") {
            mostrarTurnosDisponibles(); // Muestra todos los turnos disponibles
            return; // Salir para evitar continuar con la lógica normal
        } else {
            informarEstadoAsignatura(asignatura);
        }
    }
}

void Horario::mostrarDiasDisponibles() {
    QStringList diasDisponibles;

    // Lista de turnos con sus horarios correspondientes
    QStringList turnos = {"8:00-9:20", "9:25-10:45", "10:55-12:15", "13:00-14:20", "14:25-15:45", "15:55-16:15"};

    for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
        QString dia = ui->tableWidget->horizontalHeaderItem(col)->text();
        QStringList turnosVacios;

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (!item || item->text().isEmpty()) {
                // Agregar el horario correspondiente al turno vacío
                turnosVacios << turnos[row]; // Agregar el horario del turno disponible
            }
        }

        if (!turnosVacios.isEmpty()) {
            diasDisponibles << QString("%1: vacío %2").arg(dia).arg(turnosVacios.join(", "));
        }
    }

    if (diasDisponibles.isEmpty()) {
        QMessageBox::information(this, "Sin Días Disponibles", "No hay días vacíos.");
    } else {
        QMessageBox::information(this, "Días Disponibles",
                                 QString("Días disponibles:\n%1").arg(diasDisponibles.join("\n")) +
                                     "\n\nPresione OK para continuar.");
    }
}

void Horario::mostrarTurnosDisponibles() {
    QStringList turnosDisponibles;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QString turno = ui->tableWidget->item(row, 0)->text();
        QStringList diasVacios;

        for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (!item || item->text().isEmpty()) {
                diasVacios << ui->tableWidget->horizontalHeaderItem(col)->text(); // Agregar el día disponible
            }
        }

        if (!diasVacios.isEmpty()) {
            turnosDisponibles << QString("%1: vacío en %2").arg(turno).arg(diasVacios.join(", "));
        }
    }

    if (turnosDisponibles.isEmpty()) {
        QMessageBox::information(this, "Sin Turnos Disponibles", "No hay turnos vacíos.");
    } else {
        QMessageBox::information(this, "Turnos Disponibles",
                                 QString("Turnos vacíos:\n%1").arg(turnosDisponibles.join("\n")) +
                                     "\n\nPresione OK para continuar.");
    }
}

int Horario::contarAsignaturasPorDia(QString dia, QString asignatura) {
    int count = 0;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->tableWidget->item(row, getColumnFromDia(dia));
        if (item && item->text() == asignatura) {
            count++;
        }
    }

    return count;
}

int Horario::contarAsignaturas(QString asignatura) {
    int count = 0;

    for (int col = 1; col < ui->tableWidget->columnCount(); ++col) {
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item && item->text() == asignatura) {
                count++;
            }
        }
    }

    return count;
}

bool Horario::validarEspacio(QString dia, QString turno, int frecuencia) {
    int availableSlots = contarTurnosDisponibles(dia, turno);

    if (availableSlots < frecuencia) {
        return false; // No hay suficiente espacio
    }

    return true; // Hay espacio suficiente
}

void Horario::informarEstadoAsignatura(QString asignatura) {
    int totalAsignaturas = contarAsignaturas(asignatura);

    if (totalAsignaturas > 0) {
        QMessageBox::information(this, "Estado de Asignatura",
                                 QString("La asignatura '%1' ya está programada %2 veces.").arg(asignatura).arg(totalAsignaturas));

        // Sugerir espacios disponibles si hay menos de 3 instancias
        if (totalAsignaturas < 3) {
            QMessageBox::information(this, "Espacios Disponibles",
                                     "Puede agregar más instancias de esta asignatura en los días y turnos disponibles.");
        }

    } else {
        QMessageBox::information(this, "Nueva Asignatura",
                                 QString("La asignatura '%1' no está programada.").arg(asignatura));
    }
}

void Horario::agregarAsignatura(QString dia, QString turno, QString asignatura, int frecuencia) {

    int row = getRowFromTurno(turno); // Obtener fila correspondiente al turno
    int col = getColumnFromDia(dia); // Obtener columna correspondiente al día

    if (row != -1 && col != -1) {
        QTableWidgetItem *item = ui->tableWidget->item(row, col);
        if (item && item->text().isEmpty()) {
            item->setText(asignatura);
        } else {
            QMessageBox::warning(this, "Error", QString("El turno '%1' del día '%2' ya está ocupado.").arg(turno).arg(dia));
        }
    }
}

int Horario::contarTurnosDisponibles(QString dia, QString turno) {
    int count = 0;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->tableWidget->item(row, getColumnFromDia(dia));
        if (item && item->text().isEmpty()) {
            count++;
        }
    }

    return count;
}

int Horario::getColumnFromDia(QString dia) {
    QStringList dias = {"Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado"};
    return dias.indexOf(dia) + 1;
}

int Horario::getRowFromTurno(QString turno) {
    QStringList turnos = {"8:00-9:20", "9:25-10:45", "10:55-12:15", "13:00-14:20", "14:25-15:45", "15:55-16:15"};
    return turnos.indexOf(turno);
}
