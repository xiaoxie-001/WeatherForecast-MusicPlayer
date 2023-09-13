#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H

#include <QWidget>
#include "weather.h"
#include "mythread.h"
#include "mymusicplayer.h"
#include <QThread>

#include <QString>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QSvgWidget>   //QT +=svg 在pro文件中添加模块
#include <QSvgRenderer> //svg渲染器
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QListWidgetItem>
namespace Ui {
class WeatherWidget;
}

class WeatherWidget : public QWidget {
    Q_OBJECT

public:
    explicit WeatherWidget(QWidget *parent = 0);
    ~WeatherWidget();
    void UI_Design();
    void setIcon(int);      //设置多日天气图标显示
    void setReminder();  //设置温馨提示语
    void setPreOrNextPlay();//当上一曲或者下一曲播放时调用

private slots:
    void on_pushButtonMini_clicked();   //最小化按钮
    void on_pushButtonDestroy_clicked();//关闭按钮
    void on_pushButtonQuery_clicked();  //查询按钮
    void on_pushButtonLocation_clicked(); //定位按钮
    void on_selectDays_3_clicked();
    void on_selectDays_7_clicked();

    void ShowThisWeather();
    void GetWeatherFailed();

    void updateCurrent(); //更新时间

    void on_btnPause_clicked();
    void on_btnNext_clicked();
    void on_btnLast_clicked();
    void sethSliderValue(const int &number);

    void on_hSliderPlayProgress_valueChanged(int value);
    void on_hSliderPlayProgress_sliderReleased();
    void on_vSliderVolume_valueChanged(int value);
    void on_vSliderVolume_sliderReleased();
    void on_btnVolume_clicked();
    void on_btnAddMusic_clicked();
    void on_lwMusicList_itemDoubleClicked(QListWidgetItem *item); //双击播放
    void showMusicPosition(QString time);
    void showMusicDuration(QString time);
    void on_lwMusicList_itemClicked(QListWidgetItem *item);

signals:

private:
    QPoint startPos;//鼠标点击时的全局位置
    QPoint startPos1;//鼠标点击时的相对位置
    void mousePressEvent(QMouseEvent *event);//鼠标点击
    void mouseMoveEvent(QMouseEvent *event);//鼠标追踪

    void movieToLoad(); //加载gif

    QString iconPic;        //存储图片路径
    QSvgWidget *svgWidget;  // 创建一个 QSvgWidget 来显示 SVG 图像 加载天气图标
    QSvgRenderer *svgRender;


    Ui::WeatherWidget *ui;
    Weather *weather;
    MyMusicPlayer *myMediaPlayer;
    MyThread *myT;
    QThread *thread;

    QTimer *timer;

    QString daysNumber ;//设置选择的天数
    int temp           ;
    int feelTemp       ;
    int uvIndex        ;
    int vis            ;
    int humidity       ;
    double precip      ;
    QString weatherType;

    QMovie *movie;          //背景动画gif
    QMovie *movie2;         //添加一个启动界面gif动画，当天气获取数据加载完后关闭
    QMovie *movie3;
    QMovie *movie4;

    QTimer *timer2; //读取播放器进度条的值
    int sliderPlayValue; //记录读取进度条的值
    QString mediaPath;//存储当前音乐文件的目录加文件名
    QHash<QString,int> musicFileIndex; //添加一个成员变量来保存音乐文件路径和索引的映射
    int valueindex; //用于判断当前上一曲或下一曲的索引value值
    int countFiles=0;//hash key的键值value 计算添加的文件个数
    int playtime; //显示当前歌曲进度时间

};

#endif // WEATHERWIDGET_H

