#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    void setupUi();

    static const int NUM_TABLES = 9;
    static const int NUM_CUSTOMERS = 5;
};

#endif // MAINWINDOW_H
