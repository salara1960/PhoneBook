#include "mainwindow.h"

int main(int argc, char *argv[])
{
int ret = 0;
QString d_name("phonebook.s3db");

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
    catch (bad_alloc) {
        perror("Error alloc memory\n");
        return -1;
    }

    return ret;
}
