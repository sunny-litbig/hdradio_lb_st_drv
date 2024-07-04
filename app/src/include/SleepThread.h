#ifndef SLEEPTHREAD_H
#define SLEEPTHREAD_H

#include <QThread>

class SleepThread : public QThread
{
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

#endif // SLEEPTHREAD_H
