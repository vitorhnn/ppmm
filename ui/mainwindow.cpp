#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include "../assembler/MIPSAssembler.hpp"
#include <cstdio>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    core(this)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    About about;
    about.exec();
}


void MainWindow::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "save file", "",
                                                    "Assembly (*.asm)");
    QFile file(fileName + ".asm");
    file.open(QIODevice::WriteOnly);
    QString code = ui->textEdit->toPlainText();
    QTextStream output(&file);
    output << code;
    file.close();
}

void MainWindow::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("open file"),"",
                                                    tr("Assembly (*.asm)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream input(&file);
    QString code;
    code = input.readAll();
    ui->textEdit->setPlainText(code);
}

void MainWindow::on_assembleButton_clicked()
{
    try {
        core.memory = Assemble(ui->textEdit->toPlainText().toStdString().append("\n"));
    } catch (const std::invalid_argument& ex) {
        QMessageBox::critical(this, "Invalid argument", QString(ex.what()));
    }


    ui->codeBrowser->setPlainText(ui->textEdit->toPlainText());

    core.Reset();

    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem(QString::number(core.gpr[i])));
    }
}

void MainWindow::on_runButton_clicked()
{
    while(core.Cycle()){
        refreshMemoryTable();
    }

    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem(QString::number(core.gpr[i])));
    }
}

void MainWindow::on_stepButton_clicked()
{
    core.Cycle();
    refreshMemoryTable();

    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem(QString::number(core.gpr[i])));
    }
}

void MainWindow::refreshMemoryTable()
{
    std::vector<std::pair<uint32_t, uint32_t>> memVec(core.memory.begin(), core.memory.end());

    std::sort(memVec.begin(), memVec.end(), [] (const auto& a, const auto& b) { return a.first < b.first; });

    ui->memTable->setRowCount(memVec.size());

    size_t i = 0;

    for (const auto& pair : memVec) {
        ui->memTable->setItem(i,0,new QTableWidgetItem(QString::number(pair.first)));
        ui->memTable->setItem(i,1,new QTableWidgetItem(QString::number(pair.second)));
        i++;
    }
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::Print(std::string line)
{
    ui->OutputText->append(QString::fromStdString(line));
}
