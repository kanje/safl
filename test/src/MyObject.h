/*
 * This file is a part of Stand-alone Future Library (safl).
 */

#pragma once

#include <QObject>

class MyObject
        : public QObject
{
    Q_OBJECT

signals:
    void sig0();
    void sig1(int);
    void sig2(int, int);
};
