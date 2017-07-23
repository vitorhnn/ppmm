#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../core/MIPSCore.hpp"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, OutputSink
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

    void on_actionAbout_Qt_triggered();

private:
    Ui::MainWindow *ui;

    void refreshMemoryTable();

    void Print(std::string line) override;

    MIPSCore core;
};

#endif // MAINWINDON_H
