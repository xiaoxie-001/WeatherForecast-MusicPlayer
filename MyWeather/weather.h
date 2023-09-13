#ifndef WEATHER_H
#define WEATHER_H

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextCodec>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>   //json数组

class Weather : public QObject {
    Q_OBJECT
public:
    explicit Weather(QObject *parent = nullptr);


public:
    void initialization(); //初始化
    void refresh();
    QMap<QString, QMap<QString, QString> > getDataMap();  //获取未来多日天气预报
    void setNowCity();
    void setCityLocation(QString,QString);  //需要查询的城市 he 查询天数

    QString GetCityName();  //获取城市
    QString GetUpdateTime();    //updateTime 当前API的最近更新时间
    QString GetObsTime();       //now.obsTime 数据观测时间
    QString GetFxLink();        //fxLink 当前数据的响应式页面，便于嵌入网站或应用
    QString GetTemp();          //now.temp 温度，默认单位：摄氏度
    QString GetFeelsLike();     //now.feelsLike 体感温度，默认单位：摄氏度
    QString GetIcon();          //now.icon 天气状况的图标代码，另请参考天气图标项目
    QString GetWeatherType();   //天气类型 获取天气类型天气状况的文字描述，包括阴晴雨雪等天气状态的描述
    QString GetWind();          //now.windDir 风向     now.windScale 风力等级    获取风向
    QString GetHumidity();      //now.humidity 相对湿度，百分比数值
    QString GetPrecip();        //now.precip 当前小时累计降水量，默认单位：毫米
    QString GetPressure();       //pressure 大气压强，默认单位：百帕
    QString GetVis();           //vis 能见度，默认单位：公里
    QString GetCloud();         //云量，百分比数值。可能为空



public slots:
    void slotGetCityNameReply();
    void slotGetCityIdReply();
    void slotDailyRequestFinished();
    void slotGetWeatherReplyFinished();

signals:
    void getDataSuccessedSignal();//获取数据成功的信号
    void getDataFailedSignal();//获取数据失败的信号


private:
    QNetworkAccessManager *netManager;
    QTextCodec *codec;
    QString city;
    QString cityId;
    QMap<QString, QMap<QString, QString>> dataMap;

    QString allInfo;    //读取未来天气的json数据
    bool isGetData = false; //是否成功获取数据
    QString daysNumber="3"; //初始需要查询的天气数

    QString updateTime  ;           //updateTime 当前API的最近更新时间
    QString obsTime     ;           //now.obsTime 数据观测时间
    QString fxLink      ;           //fxLink 当前数据的响应式页面，便于嵌入网站或应用
    QString temp        ;           //now.temp 温度，默认单位：摄氏度
    QString feelsLike   ;           //now.feelsLike 体感温度，默认单位：摄氏度
    QString icon        ;           //now.icon 天气状况的图标代码，另请参考天气图标项目
    QString weatherType ;           //now.text 天气状况的文字描述，包括阴晴雨雪等天气状态的描述
    QString wind        ;           //now.windDir 风向     now.windScale 风力等级
    QString humidity    ;           //now.humidity 相对湿度，百分比数值
    QString precip      ;           //now.precip 当前小时累计降水量，默认单位：毫米
    QString pressure    ;           //now.pressure 大气压强，默认单位：百帕
    QString vis         ;           //now.vis 能见度，默认单位：公里
    QString cloud       ;           //now.cloud 云量，百分比数值。可能为空

};

#endif // WEATHER_H

