#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QSqlQuery>
#include <QFileInfo>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSqlDatabase* project_db;

private slots:

    void on_comboBox_currentTextChanged(const QString &arg1);
    void on_pushButton_clicked();
    void on_lineEdit_textEdited(const QString &arg1);
    void on_lineEdit_2_textEdited(const QString &arg1);
};
#endif // MAINWINDOW_H
