#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../core/MIPSCore.hpp"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionQuit_triggered();

    void on_actionAbout_triggered();

    void on_saveButton_clicked();

    void on_loadButton_clicked();

    void on_runButton_clicked();

    void on_stepButton_clicked();

    void on_assembleButton_clicked();

    void on_pushButton_clicked();

    void refreshMemoryTable();
private:
    Ui::MainWindow *ui;

    MIPSCore core;
};

#endif // MAINWINDON_H
