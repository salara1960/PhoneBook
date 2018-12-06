#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#undef SET_DEBUG

#include <QApplication>
#include <QMainWindow>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#ifdef SET_DEBUG
    #include <QDebug>
#endif
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <iostream>
#include <time.h>

typedef struct
{
    int     number;
    QString phone;
    QString first_name;
    QString last_name;
    QString country;
    QString city;
    QString age;
    QString email;
    QString remark;
    time_t  epoch;
} db_rec;
//-------------------------------------------------------------------
using namespace std;
//-------------------------------------------------------------------
namespace Ui {
class PWindow;
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class PWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PWindow(QWidget *parent = 0, QString *nm = 0);
    ~PWindow();

    bool open_db();
    int ListRecord(int);
    bool add_rec(db_rec *);
    int Total_rec();
    void DelItems();
    void MsgEmpty();
    void UpdateStatus();
    int ShowAnswer(int);
    bool cmd_request(QStringList *, QString *);
    void error_msg(QSqlError &);

signals:
    void show_list(int);
    void get_item(int);

public slots:
    void about();
    bool find_rec();
    bool update_rec();
    void refresh();
    bool del_rec();
    void get_rec(int);
    int ListShow(int);
    void AddRecord();
    void RowNum(int);
    void clear_forms();

private:
    Ui::PWindow *ui;
    QSqlDatabase *db;
    QSqlQuery *query;
    QString *name;
    bool openok, good;
    int cur_record, total_rec, last_rec;
    QSqlError error;
    db_rec srec;
    QStringList list;
    QList<QTableWidgetItem *> ptr_item;
};
//-------------------------------------------------------------------
#endif // MAINWINDOW_H
