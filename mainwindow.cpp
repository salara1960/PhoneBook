#include "mainwindow.h"
#include "ui_mainwindow.h"

//const QString app_ver = "1.2";//04.12.2018
//const QString app_ver = "1.3";//05.12.2018
const QString app_ver = "1.4";//10.12.2018


const QString app_name = "Телефонная книжка";
const int max_column = 10;
const int max_cnt = 20;
const QString sep="$^$";
const char * mk_table_db3 = "CREATE TABLE IF NOT EXISTS phone (\
        number INTEGER primary key autoincrement, \
        u_phone TEXT, \
        u_first_name TEXT, \
        u_last_name TEXT, \
        u_contry TEXT, \
        u_city TEXT, \
        u_age TEXT, \
        u_email TEXT, \
        u_remark TEXT, \
        u_epoch TIMESTAMP);";

//**************************************************************************

PWindow::TheError::TheError(int err) { code = err; }

//**************************************************************************
PWindow::PWindow(QWidget *parent, QString *nm) : QMainWindow(parent), ui(new Ui::PWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->size());

    this->setWindowTitle(app_name);

    MyError = 0;

    list.clear();
    ptr_item.clear();
    name = nm;
    query = NULL;

    error.setType(QSqlError::NoError);
    openok = good = false;
    cur_record = last_rec = 0;
    total_rec = -1;
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    if (db == NULL) {
        MyError |= 1;//create database object error
        throw TheError(MyError);
    }
    db->setDatabaseName(*name);
    query = new QSqlQuery(*db);
    if (!open_db()) {
        MyError |= 2;//open database error
        throw TheError(MyError);
    }

    connect(ui->actionAdd, SIGNAL(triggered(bool)), this, SLOT(AddRecord()));
    connect(ui->actionClear, SIGNAL(triggered(bool)), this, SLOT(clear_forms()));
    connect(ui->actionDel, SIGNAL(triggered(bool)), this, SLOT(del_rec()));
    connect(ui->actionRefresh, SIGNAL(triggered(bool)), this, SLOT(refresh()));
    connect(ui->actionUpdate, SIGNAL(triggered(bool)), this, SLOT(update_rec()));
    connect(ui->actionFind, SIGNAL(triggered(bool)), this, SLOT(find_rec()));
    connect(ui->actionHelp, SIGNAL(triggered(bool)), this, SLOT(about()));

    connect(this, SIGNAL(show_list(int)), this, SLOT(ListShow(int)));

    connect(ui->tbl , SIGNAL(cellClicked(int,int)) , this, SLOT(RowNum(int)));

    total_rec = cur_record = Total_rec();

    emit show_list(0);

    UpdateStatus();

}
//--------------------------------------------------------------------------
PWindow::~PWindow()
{
    if (query) {
        query->clear();
        delete query;
    }
    if (db) {
        if (db->isOpen()) db->close();
        delete db;
    }
    DelItems();
    delete ui;
}
//--------------------------------------------------------------------------
void PWindow::about()
{
    QMessageBox::information(this,
                             "О программе",
                             app_name + "\n Версия " + app_ver + "\nБаза данных : " + *name + "\n(Qt + Sqlite3)");
}
//--------------------------------------------------------------------------
void PWindow::error_msg(QSqlError &er)
{
    if (er.type() == QSqlError::NoError) return;
    QMessageBox::critical(this,"Ошибка", "База данных " + *name + ": " + er.text());
}
//--------------------------------------------------------------------------
bool PWindow::find_rec()
{
    QString tp, tpm = "|";
    tp.clear();
    if (ui->checkBoxNumber->checkState() == Qt::Checked) {
        tp += " u_phone='" + ui->ValueNumber->text() + "'";
        tpm += ui->ValueNumber->text() + "|";
    }
    if (ui->checkBoxFirstName->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp +=" u_first_name='" + ui->ValueFirst->text() + "'";
        tpm += ui->ValueFirst->text() + "|";
    }
    if (ui->checkBoxLastName->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_last_name='" + ui->ValueLast->text() + "'";
        tpm += ui->ValueLast->text() + "|";
    }
    if (ui->checkBoxCountry->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_contry='" + ui->ValueCountry->text() + "'";
        tpm += ui->ValueCountry->text() + "|";
    }
    if (ui->checkBoxCity->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_city='" + ui->ValueCity->text() + "'";
        tpm += ui->ValueCity->text() + "|";
    }
    if (ui->checkBoxAge->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_age='" + ui->ValueAge->text() + "'";
        tpm += ui->ValueAge->text() + "|";
    }
    if (ui->checkBoxEmail->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_email='" + ui->ValueEmail->text() + "'";
        tpm += ui->ValueEmail->text() + "|";
    }
    if (ui->checkBoxRemark->checkState() == Qt::Checked) {
        if (tp.length()>0) tp += " and";
        tp += " u_remark='" + ui->ValueRemark->text() + "'";
        tpm += ui->ValueRemark->text() + "|";
    }
    if (!tp.length()) {
        QMessageBox::critical(this,"Информационное сообщение", "Не опеределены условия поиска");
        return false;
    }

    QString tmp = "SELECT * FROM phone WHERE" + tp + " order by number desc;";
    good = false;
    bool gd = false;

    QStringList lt; lt.clear();
    gd = cmd_request(&lt, &tmp);
    int cnt = lt.length();
    if ((gd) && (cnt>0)) {
        list = lt;
        ShowAnswer(cnt);
        good = true;
    } else QMessageBox::warning(this,"Запись не найдена","По запросу\n" + tpm + "\nничего не найдено");

    return good;
}
//--------------------------------------------------------------------------
bool PWindow::cmd_request(QStringList *lst, QString *st)
{
    QString tmp = *st;
    QString tp;
    bool gd;
    time_t ttm;
    struct tm *timi;
    error.setType(QSqlError::NoError);
    query->clear();
    if (query->exec(tmp)) {
        while (query->next()) {
            tmp.clear();
            for (int i = 0; i < max_column - 1; i++) tmp += query->value(i).toString() + sep;
            tp.clear(); gd=false;
            ttm = (time_t)query->value(max_column - 1).toUInt(&gd); //u_epoch
            if (gd) {
            timi = localtime(&ttm);
            tp.sprintf("%02d.%02d.%04d %02d:%02d:%02d",
                    timi->tm_mday, timi->tm_mon + 1, timi->tm_year + 1900,
                    timi->tm_hour, timi->tm_min,     timi->tm_sec);
            }
            tmp += tp + sep;//sepl
            *lst << tmp;
        }
    } else error = query->lastError();

    if (error.type() == QSqlError::NoError) gd = true;
                                       else gd = false;
    return gd;
}
//--------------------------------------------------------------------------
bool PWindow::update_rec()
{
bool ret = false;

    if (cur_record <= 0) return ret;

    srec.phone = ui->ValueNumber->text();
    if (!srec.phone.length()) return ret;
    srec.first_name = ui->ValueFirst->text();
    srec.last_name = ui->ValueLast->text();
    srec.country = ui->ValueCountry->text();
    srec.city = ui->ValueCity->text();
    srec.age = ui->ValueAge->text();
    srec.email = ui->ValueEmail->text();
    srec.remark = ui->ValueRemark->text();

    QString tmp = QString("UPDATE phone set u_phone='%1',u_first_name='%2',u_last_name='%3',\
        u_contry='%4',u_city='%5',u_age='%6',u_email='%7',u_remark='%8',u_epoch=%9 WHERE number=%10;")
            .arg(srec.phone)
            .arg(srec.first_name)
            .arg(srec.last_name)
            .arg(srec.country)
            .arg(srec.city)
            .arg(srec.age)
            .arg(srec.email)
            .arg(srec.remark)
            .arg((uint)QDateTime::currentDateTime().toTime_t())
            .arg(cur_record);

    error.setType(QSqlError::NoError);
    query->clear();
    if (query->exec(tmp)) good = true;
    else {
        error = query->lastError();
        good = false;
    }

    emit show_list(0);
    ret = good;

    return ret;
}
//--------------------------------------------------------------------------
void PWindow::refresh()
{
    emit show_list(0);
}
//--------------------------------------------------------------------------
void PWindow::MsgEmpty()
{
    QString tp = "База данных " + *name + " пустая";
    QMessageBox::information(this,"Информационное сообщение", tp);
}
//--------------------------------------------------------------------------
bool PWindow::open_db()
{
    openok = db->open();

    query->clear();
    query->exec(mk_table_db3);

    if (openok) {// DB открыта, с ней можно работать
        error.setType(QSqlError::NoError);
        if (query->exec("SELECT * FROM phone order by number desc limit 1;")) {
            while (query->next()) {
                QString ttt = query->value(0).toString();
                bool gd;
                total_rec = ttt.toInt(&gd, 10);
                if (!gd) total_rec = 0;
                break;
            }
        } else error = query->lastError();
    }

    if (openok)
        if (error.type() != (QSqlError::NoError)) openok = false;

    return openok;
}
//--------------------------------------------------------------------------
bool PWindow::add_rec(db_rec * r)
{
    if (!r) return false;

    bool gd = true;
    QString tmp;
    good = false;
    error.setType(QSqlError::NoError);
    query->clear();
    QString tp = QString("select * from phone where u_phone='%1';").arg(r->phone);
    if (query->exec(tp)) {
        while (query->next()) {
            tmp.clear();
            for (int i = 0; i < max_column; i++) tmp += query->value(i).toString();
            if (tmp.length() > 0) { gd = false; break; }
        }
    } else { error = query->lastError(); return good; }

    if (!gd) {
        QString tp = "Номер '" + r->phone + "' уже есть в базе данных";
        QMessageBox::critical(this,"Информационное сообщение", tp);
        return gd;
    }

    tmp.clear();
    tmp = QString("INSERT INTO phone (u_phone,u_first_name,u_last_name,u_contry,u_city,u_age,u_email,u_remark,u_epoch)\
                   VALUES ('%1','%2','%3','%4','%5','%6','%7','%8',")
                    .arg(r->phone)
                    .arg(r->first_name)
                    .arg(r->last_name)
                    .arg(r->country)
                    .arg(r->city)
                    .arg(r->age)
                    .arg(r->email)
                    .arg(r->remark);
    tp.clear(); tp = QString("%1").arg( (uint)QDateTime::currentDateTime().toTime_t() );
    tmp += tp +");";
    if (query->exec(tmp)) good = true;
    else {
        error = query->lastError();
        good = false;
    }

    return good;
}
//--------------------------------------------------------------------------
bool PWindow::del_rec()
{
    if ((openok) && (cur_record > 0)) {
#ifdef SET_DEBUG
    qDebug() << "openok=" << openok << "cur_record=" << cur_record << "total_rec=" << total_rec;
#endif
    error.setType(QSqlError::NoError);
        query->clear();
        QString tp;
        tp.sprintf("DELETE FROM `phone` where number=%d;", cur_record);
#ifdef SET_DEBUG
    qDebug() << tp;
#endif
    if (query->exec(tp)) good = true;
        else {
            error = query->lastError();
            good = false;
        }
        total_rec = Total_rec();
#ifdef SET_DEBUG
    qDebug() << "after delete: total_rec=" << total_rec << "good=" << good;
#endif
        emit show_list(0);
    }

    return good;
}
//--------------------------------------------------------------------------
void PWindow::get_rec(int ind)
{
    ui->ValueNumber->setText(ui->tbl->item(ind,1)->text());
    ui->ValueFirst->setText(ui->tbl->item(ind,2)->text());
    ui->ValueLast->setText(ui->tbl->item(ind,3)->text());
    ui->ValueCountry->setText(ui->tbl->item(ind,4)->text());
    ui->ValueCity->setText(ui->tbl->item(ind,5)->text());
    ui->ValueAge->setText(ui->tbl->item(ind,6)->text());
    ui->ValueEmail->setText(ui->tbl->item(ind,7)->text());
    ui->ValueRemark->setText(ui->tbl->item(ind,8)->text());
#ifdef SET_DEBUG
    qDebug() << "get_rec(" << ind << ")";
#endif
}
//--------------------------------------------------------------------------
void PWindow::clear_forms()
{
    ui->ValueNumber->clear();
    ui->ValueFirst->clear();
    ui->ValueLast->clear();
    ui->ValueCountry->clear();
    ui->ValueCity->clear();
    ui->ValueAge->clear();
    ui->ValueEmail->clear();
    ui->ValueRemark->clear();
}
//--------------------------------------------------------------------------
int PWindow::Total_rec()
{
int ret = 0;

    error.setType(QSqlError::NoError);
    query->clear();
    if (query->exec("select count(*) from `phone`;")) {
        bool gd = false;
        query->first();
        ret = query->value(0).toInt(&gd);
        if (!gd) ret = 0;
    }
#ifdef SET_DEBUG
    qDebug() << "Total_rec()=" << ret;
#endif
    return ret;
}
//--------------------------------------------------------------------------
void PWindow::AddRecord()
{
    srec.phone = ui->ValueNumber->text();
    if (!srec.phone.length()) return;
    srec.first_name = ui->ValueFirst->text();
    srec.last_name = ui->ValueLast->text();
    srec.country = ui->ValueCountry->text();
    srec.city = ui->ValueCity->text();
    srec.age = ui->ValueAge->text();
    srec.email = ui->ValueEmail->text();
    srec.remark = ui->ValueRemark->text();

    if (add_rec(&srec)) total_rec = Total_rec();

    emit show_list(0);

    UpdateStatus();
}
//--------------------------------------------------------------------------
void PWindow::UpdateStatus()
{
QString tp = QString("База [%1] записей : %2/%3/%4").arg(*name).arg(total_rec).arg(cur_record).arg(last_rec);
    if (error.type() != QSqlError::NoError) tp.append(" ("+ error.text() + ")");
    ui->statusBar->showMessage(tp);
    error_msg(error);
}
//--------------------------------------------------------------------------
int PWindow::ListRecord(int kol)
{
int cnt=0;

    if (total_rec > 0) {
        bool gd;
        QString tp;
        if (!kol)
            tp = QString("SELECT * FROM phone order by number desc limit %1;").arg(max_cnt);
        else
            tp = QString("SELECT * FROM phone where number<=%1 order by number desc limit %2;").arg(kol).arg(max_cnt);

        QStringList lt; lt.clear();

        gd = cmd_request(&lt, &tp);
        cnt = lt.length();
        if (gd) {
            if (cnt > 0) list = lt;
        }
    } else MsgEmpty();

    return cnt;
}
//---------------------------------------------------------------------------
int PWindow::ListShow(int kol)
{
int all = ListRecord(kol);

    ShowAnswer(all);

    return all;
}
//---------------------------------------------------------------------------
int PWindow::ShowAnswer(int all)
{
    DelItems();
    if (all > 0) {
        //-------------------
        ui->tbl->setRowCount(all);//количество строк
        ui->tbl->setColumnCount(max_column); //количество столбцов
        ui->tbl->verticalHeader()->hide();
        QStringList *lt = new QStringList(QStringList() << "#"
                                        << "Номер"
                                        << "Фамилия"
                                        << "Имя"
                                        << "Страна"
                                        << "Город"
                                        << "Возраст"
                                        << "Почта"
                                        << "Примечание"
                                        << "Дата");
        int row, column;
        for (column = 0; column < ui->tbl->columnCount(); column++) {
            ui->tbl->setHorizontalHeaderLabels(*lt);
            if (!column) ui->tbl->hideColumn(0);//!!!!!!!!!!!!!!!!!!
        }

        lt->clear();
        ui->tbl->resizeRowsToContents();
        for (row = 0; row < ui->tbl->rowCount(); row++) {
            *lt = list.at(row).split(sep);
            for (column = 0; column < ui->tbl->columnCount(); column++) {
                QTableWidgetItem *item = new QTableWidgetItem;
                ptr_item << item;
                item->setText(lt->at(column));
                ui->tbl->setItem(row, column, item);
                ui->tbl->resizeColumnToContents(column);
                if (!column) ui->tbl->hideColumn(0);//!!!!!!!!!!!!!!!!!!
            }
        }
        delete lt;

        QString tmp; tmp.append(ui->tbl->item(0,0)->text());
        bool gd;
        if (gd) last_rec = tmp.toInt(&gd, 10);

        ui->tbl->show();

        emit ui->tbl->cellClicked(0, 0);
        //-------------------
    } else ui->tbl->hide();

    UpdateStatus();

#ifdef SET_DEBUG
    qDebug() << "ShowAnswer(" << all << ") : last_rec=" << last_rec << "cur_record=" << cur_record;
#endif

    return all;
}
//---------------------------------------------------------------------------
void PWindow::RowNum(int r)
{
    ui->tbl->selectRow(r);
    QString tmp; tmp.append(ui->tbl->item(r, 0)->text());
    bool gd;
    int ok = tmp.toInt(&gd, 10);
    if (gd) cur_record = ok;
    UpdateStatus();
    get_rec(r);
#ifdef SET_DEBUG
    qDebug() << "RowNum(" << r << ") : cur_record=" << cur_record << "tmp=" << tmp << "last_rec=" << last_rec;
#endif
}
//---------------------------------------------------------------------------
void PWindow::DelItems()
{
    for (int i = 0; i < ptr_item.size(); i++) {
        if (ptr_item.at(i)) delete ptr_item.at(i);
    }
    ptr_item.clear();
}
//---------------------------------------------------------------------------
