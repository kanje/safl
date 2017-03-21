/*
 * This file is a part of Standalone Future Library (safl).
 */

#include <iostream>

#include <safl/Future.h>
//#include "ToFuture.h"

#include <QCoreApplication>
#include <QTimer>

using namespace safl;

int multiplyBy2(int i)
{
    return i*2;
}

void printInt(int i)
{
    std::cout << i << std::endl;
}

int return5()
{
    return 5;
}

void print9()
{
    std::cout << 9 << std::endl;
}

void term()
{
    qApp->quit();
}

Future<int> get9()
{
    SharedPromise<int> p;

    QTimer::singleShot(5000, [p]() mutable
    {
        p->setValue(9);
    });

    return p->future();
}

Future<int> foo(int i)
{
    auto *p = new Promise<int>;

    QTimer::singleShot(5000, [p, i]()
    {
        p->setValue(i - 4);
    });

    return p->future();
}

void loop()
{
    get9().then(foo).then(printInt)
           .then(term);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTimer::singleShot(0, []() { loop(); });
    return app.exec();
}
