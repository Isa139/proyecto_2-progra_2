#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
}

void MainWindow::setupUi() {
    // Configurar el fondo
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(20); // Añadir espacio entre los elementos
    mainLayout->setAlignment(Qt::AlignCenter); // Centrar los elementos en el layout
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor("#F5DEB3")); // Color arena para el suelo del restaurante
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Configurar la cocina
    QHBoxLayout *kitchenLayout = new QHBoxLayout();
    QLabel *kitchenLabel = new QLabel("COCINA", this);
    kitchenLabel->setStyleSheet("QLabel { background-color: #98FB98; border: 1px solid black; }"); // Verde para la cocina
    kitchenLabel->setAlignment(Qt::AlignCenter);
    kitchenLabel->setFixedWidth(150); // Tamaño fijo para la cocina
    kitchenLayout->addWidget(kitchenLabel);
    mainLayout->addLayout(kitchenLayout);

    // Configurar las mesas
    const int NUM_CUSTOMERS_IN = 5; // Número de clientes dentro del restaurante
    const int NUM_CUSTOMERS_OUT = 5; // Número de clientes fuera del restaurante
    const int NUM_TABLES_ROW = 2;
    const int NUM_TABLES_COL = 3;
    const int TOTAL_TABLES = NUM_TABLES_ROW * NUM_TABLES_COL;

    QGridLayout *tablesLayout = new QGridLayout();
    tablesLayout->setSpacing(50); // Añadir espacio entre las mesas

    for (int i = 0; i < TOTAL_TABLES; ++i) {
        QVBoxLayout *tableLayout = new QVBoxLayout();
        tableLayout->setSpacing(10); // Añadir espacio entre la mesa y las sillas
        QLabel *tableLabel = new QLabel("Mesa " + QString::number(i+1), this);
        tableLabel->setFixedSize(100, 100); // Tamaño fijo para la mesa
        tableLabel->setStyleSheet("QLabel { background-color: #CD853F; border-radius: 50px; border: 1px solid black; }"); // Marrón para la mesa (circular)
        tableLabel->setAlignment(Qt::AlignCenter);
        tableLayout->addWidget(tableLabel);
        tablesLayout->addLayout(tableLayout, i / NUM_TABLES_COL, i % NUM_TABLES_COL);
    }

    mainLayout->addLayout(tablesLayout);

    // Configurar los clientes en el restaurante
    QHBoxLayout *customersLayout = new QHBoxLayout();
    for (int i = 0; i < NUM_CUSTOMERS_IN; ++i) {
        QLabel *customerLabel = new QLabel("●", this);
        customerLabel->setStyleSheet("QLabel { color: #FF69B4; font-size: 20px; }"); // Rosa para los clientes dentro del restaurante
        customerLabel->setAlignment(Qt::AlignCenter);
        customersLayout->addWidget(customerLabel);
    }
    mainLayout->addLayout(customersLayout);

    // Configurar los clientes fuera del restaurante
    QHBoxLayout *waitingLayout = new QHBoxLayout();
    for (int i = 0; i < NUM_CUSTOMERS_OUT; ++i) {
        QLabel *waitingCustomerLabel = new QLabel("●", this);
        waitingCustomerLabel->setStyleSheet("QLabel { color: #0000FF; font-size: 20px; }"); // Azul para los clientes fuera del restaurante
        waitingCustomerLabel->setAlignment(Qt::AlignCenter);
        waitingLayout->addWidget(waitingCustomerLabel);
    }
    mainLayout->addLayout(waitingLayout);
}













