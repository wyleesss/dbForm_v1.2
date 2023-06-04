#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(585, 375);

    setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    QString database_path = QString::fromStdString(DATABASE_PATH);

    project_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    project_db->setDatabaseName(database_path);

    if (project_db->open())
    {
        QSqlQuery* query = new QSqlQuery(*(project_db));

        ui->db_log_text->setText("підключено");
        ui->db_log_text->setStyleSheet("QLabel { color : blue; }");

        if (ui->comboBox->currentText() == "Надходження")
            query->exec("SELECT UaName FROM INCOME_TYPE_DIRECTORY_UA_LOC");

        else if (ui->comboBox->currentText() == "Видаток")
            query->exec("SELECT UaName FROM EXPENSE_TYPE_DIRECTORY_UA_LOC");

        else
        {
            ui->db_log_text->setText("помилка");
            ui->db_log_text->setStyleSheet("QLabel { color : red; }");
        }

        while (query->next())
        {
            ui->comboBox_2->addItem(query->value(0).toString());
        }

        if (ui->comboBox_2->count() == 0)
        {
            ui->db_log_text->setText("помилка");
            ui->db_log_text->setStyleSheet("QLabel { color : red; }");
        }

        int priority_index = ui->comboBox_2->findText("Переказ");

        if (priority_index != -1)
            ui->comboBox_2->setCurrentIndex(priority_index);

        delete query;
    }
    else
    {
        ui->db_log_text->setText("не підключено");
        ui->db_log_text->setStyleSheet("QLabel { color : red; }");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete project_db;
}


void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    QSqlQuery* query = new QSqlQuery(*(project_db));

    ui->comboBox_2->clear();

    if (arg1 == "Надходження")
        query->exec("SELECT UaName FROM INCOME_TYPE_DIRECTORY_UA_LOC;");

    else if (arg1 == "Видаток")
        query->exec("SELECT UaName FROM EXPENSE_TYPE_DIRECTORY_UA_LOC;");

    else
    {
        ui->db_log_text->setText("помилка");
        ui->db_log_text->setStyleSheet("QLabel { color : red; }");
    }

    while (query->next())
    {
        ui->comboBox_2->addItem(query->value(0).toString());
    }

    if (ui->comboBox_2->count() == 0)
    {
        ui->db_log_text->setText("помилка");
        ui->db_log_text->setStyleSheet("QLabel { color : red; }");
    }

    int priority_index = ui->comboBox_2->findText("Переказ");

    if (priority_index != -1)
        ui->comboBox_2->setCurrentIndex(priority_index);

    delete query;
}


void MainWindow::on_pushButton_clicked()
{
    QSqlQuery* query = new QSqlQuery(*(project_db));

    ui->result_text->setText("зачекайте..");

    if (ui->db_log_text->text() == "помилка" || ui->db_log_text->text() == "не підключено")
    {
        ui->result_text->setText("неможливо здійснити операцію: помилка під'єднання до БД");
        ui->result_text->setStyleSheet("QLabel { color : red; }");
        return;
    }

    if (ui->lineEdit->text().isEmpty() || ui->lineEdit_2->text().isEmpty())
    {
        ui->result_text->setText("неможливо здійснити операцію: обов'язкові дані не введено");
        ui->result_text->setStyleSheet("QLabel { color : red; }");
        return;
    }

    if ((ui->lineEdit->text().toInt() == 0 && ui->lineEdit->text() != "0")
    ||(ui->lineEdit_2->text().toDouble() == 0.0 && ui->lineEdit_2->text() != "0" && ui->lineEdit_2->text() != "0.0"))
    {
        ui->result_text->setText("неможливо здійснити операцію: введені дані некоректні");
        ui->result_text->setStyleSheet("QLabel { color : red; }");
        return;
    }

    QRegularExpression* expression = new QRegularExpression("^(?!.*[.]{2})\\d{1,8}(?:\\.\\d{1,2})?$");

    if (!(expression->match(ui->lineEdit_2->text()).hasMatch()))
    {
        ui->result_text->setText("неможливо здійснити операцію: недопустиме грошове значення");
        ui->result_text->setStyleSheet("QLabel { color : red; }");
        return;
    }

    QString* check_id_query_text = new QString("SELECT * FROM ACCOUNTS WHERE ID == ");
    *check_id_query_text += ui->lineEdit->text() += ";";
    query->exec(*check_id_query_text);
    query->first();

    if (query->value(0).isNull())
    {
        ui->result_text->setText("неможливо здійснити операцію: ID акаунту не існує");
        ui->result_text->setStyleSheet("QLabel { color : red; }");
        return;
    }

    QString* query_text = new QString("INSERT INTO ");
    QString* operation_type = new QString();
    QString* operation_type_select_query_text = new QString("SELECT Name FROM ");

    if (ui->comboBox->currentText() == "Надходження")
    {
        *operation_type_select_query_text += "INCOME_TYPE_DIRECTORY WHERE ID == ";
        *operation_type_select_query_text += QString::number(ui->comboBox_2->currentIndex() + 1);
        *operation_type_select_query_text += ";";
        query->exec(*operation_type_select_query_text);
        query->first();
        *operation_type = query->value(0).toString();
        *query_text += "INCOMES (AccountID, Amount, Type, AdditionalInfo) VALUES(";
    }
    else
    {
        *operation_type_select_query_text += "EXPENSE_TYPE_DIRECTORY WHERE ID == ";
        *operation_type_select_query_text += QString::number(ui->comboBox_2->currentIndex() + 1);
        *operation_type_select_query_text += ";";
        query->exec(*operation_type_select_query_text);
        query->first();
        *operation_type = query->value(0).toString();
        *query_text += "EXPENSES (AccountID, Amount, Type, AdditionalInfo) VALUES(";
    }

    *query_text += ui->lineEdit->text() += ", ";
    *query_text += ui->lineEdit_2->text() += ", '";
    *query_text += *operation_type += "', ";

    if (ui->textEdit->toPlainText().isEmpty())
        *query_text += "NULL);";
    else
    {
        *query_text += "'";
        *query_text += ui->textEdit->toPlainText() += "');";
    }

    if (query->exec(*query_text))
    {
        ui->result_text->setText("операцію здійснено успішно!");
        ui->result_text->setStyleSheet("QLabel { color : blue; }");
    }
    else
    {
        ui->result_text->setText("не вдалося виконати операцію: " + query->lastError().text());
        ui->result_text->setStyleSheet("QLabel { color : red; }");
    }

    delete query;
}


void MainWindow::on_lineEdit_textEdited(const QString &arg1)
{
    if (arg1 != "")
        ui->label_2->setText("");

    else
        ui->label_2->setText("*");
}


void MainWindow::on_lineEdit_2_textEdited(const QString &arg1)
{
    if (arg1 != "")
        ui->label_3->setText("");

    else
        ui->label_3->setText("*");
}

