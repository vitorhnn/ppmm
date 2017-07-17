#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "about.h"
#include <cstdio>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    core()
{
    ui->setupUi(this);

    ui->registersTable->setColumnWidth(0, 50);
    ui->registersTable->setColumnWidth(1, 50);
    ui->registersTable->setColumnWidth(2, 80);

    QStringList tableTitles;
    tableTitles << "Name" << "Number" << "Value";
    ui->registersTable->setHorizontalHeaderLabels(tableTitles);
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

//    std::vector<uint32_t> assembly = Assemble(ui->textEdit->toPlainText().toStdString());

    FILE *file = fopen("dump", "rb");
    if (file == nullptr){
        ui->OutputText->insertPlainText("Nenhum arquivo carregado.\n");
        return;
    }

    u32 i = core.pc = 0x400000/4,
            byte = 0;
    while(fread(&byte, sizeof(u32), 1, file) != 0){
        core.memory[i] = byte;
        i++;
    }
}

void MainWindow::on_runButton_clicked()
{
    while(core.Cycle());

    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem(QString::number(core.gpr[i])));
    }
}

void MainWindow::on_stepButton_clicked()
{
    core.Cycle();

    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem(QString::number(core.gpr[i])));
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->OutputText->setText("");
}
