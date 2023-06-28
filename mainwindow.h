#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStringList>
#include <QMainWindow>

#include "bmserializableobj.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public BmSerializableObj
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFromFile();
    void save();

    //! 序列化
    void serialize(BmSExpression &root) const override;

private:
    Ui::MainWindow *ui;

    QStringList m_names;
    QList<int> m_ids;
};

#endif // MAINWINDOW_H
