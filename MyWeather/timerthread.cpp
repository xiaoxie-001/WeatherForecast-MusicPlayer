#include "timerthread.h"
#include <QtDebug>
TimerThread::TimerThread(QObject *parent) : QThread(parent)
{
    timer = new QTimer(this);
    qDebug()<<"TimerThread构造函数"<<endl;
}

TimerThread::~TimerThread(){
    if(timer->isActive()){
        timer->stop();
    }
}

void TimerThread::run(){
    qDebug()<<"定时器run:"<<endl;
    connect(timer,&QTimer::timeout,this,[this](){
        QDateTime currentTime = QDateTime::currentDateTime();
        QString nowTime = currentTime.toString("yyyy-MM-dd hh:mm:ss ddd");
        emit currentUpDated(nowTime);
    });
    // 使用 QTimer::singleShot() 触发定时器信号
    QTimer::singleShot(1000, this, [=](){
        timer->start();
    });
//    timer->start(1000);//设置定时器的触发时间间隔1s 每秒计数1000次  定时器计数周期为1ms/次
}


