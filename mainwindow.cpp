#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_names({"name1", "name2", "name3", "name4", "name5"})
    , m_ids({1, 2, 3, 4, 5})
{
    ui->setupUi(this);
    connect(ui->openAct, &QAction::triggered, this, &MainWindow::openFromFile);
    connect(ui->saveAct, &QAction::triggered, this, &MainWindow::save);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openFromFile()
{
    auto filePath = QFileDialog::getOpenFileName(this, tr("Load bms"), tr("./"), tr("*.bms"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    BmSExpression root = BmSExpression::parse(file.readAll());
    for (const BmSExpression &node : root.getChildren("name")) {
        qDebug() << deserialize<QString>(node.getChild("@0"));
    }
    qDebug() << root.toByteArray();
}

void MainWindow::save()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save to File"), tr("./"), tr("*.bms"));
    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    BmSExpression doc(serializeToDomElement("bmserialize"));
    file.write(doc.toByteArray());
    file.close();
}

void MainWindow::serialize(BmSExpression &root) const
{
    root.ensureLineBreak();
    for (auto &name : m_names) {
        root.appendChild("name", name);
    }
    root.ensureLineBreak();
    for (auto &id : m_ids) {
        root.appendChild("id", id);
    }
    root.ensureLineBreak();
}

