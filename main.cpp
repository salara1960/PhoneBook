#include "mainwindow.h"

int main(int argc, char *argv[])
{
int ret = 0, cerr = 0;
QString d_name("phonebook.s3db");
QString cerrStr = "", errStr = "";

    if (argc > 1) {
        d_name.clear();
        d_name.append(argv[1]);
    }

    try {
        setlocale(LC_ALL,"UTF8");

        QApplication p(argc, argv);

        PWindow w(NULL, &d_name);
        w.show();

        ret = p.exec();
    }
    catch (PWindow::TheError(er)) {
        cerr = er.code;
        cerrStr.sprintf("%d", cerr);
        if (cerr > 0) {
            if (cerr & 1) errStr.append("Error create database object (" + cerrStr + ")\n");
            if (cerr & 2) errStr.append("Error open database (" + cerrStr + ")\n");
        } else errStr.append("Unknown Error (" + cerrStr + ")\n");
        if (errStr.length() > 0) {
            QByteArray bf; bf.append(errStr);
            perror(bf.data());
        }
        return cerr;
    }
    catch (bad_alloc) {
        perror("Error alloc memory\n");
        return -1;
    }

    return ret;
}
