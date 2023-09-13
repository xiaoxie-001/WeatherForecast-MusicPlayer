#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDateTime>
class TimerThread : public QThread
{
    Q_OBJECT
public:
    explicit TimerThread(QObject *parent = nullptr);
    ~TimerThread();
signals:
    void currentUpDated(const QString &currentTime);

protected:
    void run();

private:
    QTimer *timer; //设计定时器
};

#endif // TIMERTHREAD_H
/*
对于protected和private修饰符的选择，是基于继承和类的设计原则。当我们将一个虚函数声明为protected时，它意味着该函数只能被其派生类访问和重写。如果将其声明为private，那么该函数将无法被派生类访问和重写。

在这种情况下，run函数是一个重要的虚函数，它负责在线程中执行任务。通常，我们希望派生类能够自由地重写这个函数来实现自定义的线程行为。因此，将其声明为protected，允许派生类访问和重写该函数，同时限制其他类无法直接调用或访问该函数。

换句话说，protected修饰符使得run函数对类的外部是不可见的，但对于该类的派生类而言，是可见和可访问的。这符合OOP（面向对象编程）的封装原则，隐藏内部实现细节，并将访问权限控制在必要的范围内。

*/
