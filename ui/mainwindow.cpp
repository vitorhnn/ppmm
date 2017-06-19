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
    ui->registersTable->setColumnCount(3);
    ui->registersTable->setRowCount(32);
    ui->registersTable->setColumnWidth(0, 60);
    ui->registersTable->setColumnWidth(1, 60);
    ui->registersTable->setColumnWidth(2, 80);
    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,2,new QTableWidgetItem("0"));
    }
    for (size_t i = 0; i < 32; ++i) {
        ui->registersTable->setItem(i,1,new QTableWidgetItem(QString::number(i)));
    }
    ui->registersTable->setItem(0,0,new QTableWidgetItem("$zero"));
     ui->registersTable->setItem(1,0,new QTableWidgetItem("$at"));
      ui->registersTable->setItem(2,0,new QTableWidgetItem("$v0"));
       ui->registersTable->setItem(3,0,new QTableWidgetItem("$v1"));
        ui->registersTable->setItem(4,0,new QTableWidgetItem("$a0"));
         ui->registersTable->setItem(5,0,new QTableWidgetItem("$a1"));
          ui->registersTable->setItem(6,0,new QTableWidgetItem("$a2"));
           ui->registersTable->setItem(7,0,new QTableWidgetItem("$a3"));
            ui->registersTable->setItem(8,0,new QTableWidgetItem("$t0"));
             ui->registersTable->setItem(9,0,new QTableWidgetItem("$t1"));
              ui->registersTable->setItem(10,0,new QTableWidgetItem("$t2"));
               ui->registersTable->setItem(11,0,new QTableWidgetItem("$t3"));
                ui->registersTable->setItem(12,0,new QTableWidgetItem("$t4"));
                 ui->registersTable->setItem(13,0,new QTableWidgetItem("$t5"));
                  ui->registersTable->setItem(14,0,new QTableWidgetItem("$t6"));
                   ui->registersTable->setItem(15,0,new QTableWidgetItem("$t7"));
                    ui->registersTable->setItem(16,0,new QTableWidgetItem("$s0"));
                     ui->registersTable->setItem(17,0,new QTableWidgetItem("$s1"));
                      ui->registersTable->setItem(18,0,new QTableWidgetItem("$s2"));
                       ui->registersTable->setItem(19,0,new QTableWidgetItem("$s3"));
                        ui->registersTable->setItem(20,0,new QTableWidgetItem("$s4"));
                         ui->registersTable->setItem(21,0,new QTableWidgetItem("$s5"));
                          ui->registersTable->setItem(22,0,new QTableWidgetItem("$s6"));
                           ui->registersTable->setItem(23,0,new QTableWidgetItem("$s7"));
                            ui->registersTable->setItem(24,0,new QTableWidgetItem("$t8"));
                             ui->registersTable->setItem(25,0,new QTableWidgetItem("$t9"));
                              ui->registersTable->setItem(26,0,new QTableWidgetItem("$k0"));
                               ui->registersTable->setItem(27,0,new QTableWidgetItem("$k1"));
                                ui->registersTable->setItem(28,0,new QTableWidgetItem("$gp"));
                                 ui->registersTable->setItem(29,0,new QTableWidgetItem("$sp"));
                                  ui->registersTable->setItem(30,0,new QTableWidgetItem("$fp"));
                                   ui->registersTable->setItem(31,0,new QTableWidgetItem("$ra"));
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
