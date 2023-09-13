#include "weather.h"

Weather::Weather(QObject *parent) : QObject(parent)
{
    netManager = new QNetworkAccessManager(this);  //新建QNetworkAccessManager对象
    initialization();
}

void Weather::initialization() {
    refresh();
    QNetworkRequest ipRequest;
    ipRequest.setUrl(QUrl("http://whois.pconline.com.cn/ipJson.jsp?ip"));
    QNetworkReply *netReply = netManager->get(ipRequest);
    connect(netReply, &QNetworkReply::finished, this, &Weather::slotGetCityNameReply); //关联信号和槽
}

void Weather::refresh() {
    this->city.clear();
    this->cityId.clear();
    this->dataMap.clear();
    this->isGetData = false;
    this->allInfo.clear();

    this->updateTime.clear();
    this->obsTime.clear();
    this->fxLink.clear();
    this->temp.clear();
    this->feelsLike.clear();
    this->icon.clear();
    this->weatherType.clear();
    this->wind.clear();
    this->humidity.clear();
    this->precip.clear();
    this->pressure.clear();
    this->vis.clear();
    this->cloud.clear();
    this->daysNumber = "3";
}

void Weather::slotGetCityNameReply() {
    QNetworkReply *replyCity = (QNetworkReply *)sender();
    codec = QTextCodec::codecForName("gbk");
    QString ip_data = codec->toUnicode(replyCity->readAll());
    replyCity->deleteLater();
    qDebug() << "ip获取当前位置数据:" << ip_data.toStdString().data();

    int city_Begin = ip_data.indexOf("city") + 7;
    int city_End = ip_data.indexOf("cityCode") - 3;
    QString cityName = ip_data.mid(city_Begin, city_End - city_Begin);
    qDebug() << cityName;
    this->city = cityName;
    setNowCity();
}
void Weather::setNowCity() {
    QString idUrl = QString("https://geoapi.qweather.com/v2/city/lookup?number=1&location=%1&key=30164cac9e1843268d34c50058e92cf4").arg(this->city);
    qDebug() << idUrl.toStdString().data() << endl;
    QNetworkRequest cityIdRequest;
    cityIdRequest.setUrl(QUrl(idUrl));
    /*错误原因：qt.network.ssl: QSslSocket::connectToHostEncrypted: TLS initialization failed*/
    qDebug() << "===>" << QSslSocket::sslLibraryBuildVersionString();
    /* ===> "OpenSSL 1.1.1d  10 Sep 2019"  下载openssL 1.1.1d 到目录
      https://slproweb.com/products/Win32OpenSSL.html 下载地址
      从安装目录下拷贝文件libeay32.dll 、libssl-1_1.dll到你工程.pro同级文件夹下即可。
      Win32 OpenSSL v1.1.1v Light
      EXE
     */
    QNetworkReply *idReply = netManager->get(cityIdRequest);
    connect(idReply, &QNetworkReply::finished, this, &Weather::slotGetCityIdReply);
}

void Weather::slotGetCityIdReply() {
    QNetworkReply *idReply = (QNetworkReply *)sender();
    codec = QTextCodec::codecForName("UTF-8");
    QString cityRequsetData = codec->toUnicode(idReply->readAll());
    idReply->deleteLater();
    qDebug() << "查询一个城市后的json数据" << cityRequsetData.toStdString().data();

    /*
      int id_begin = cityRequsetData.indexOf("id")+5;
      int id_end = cityRequsetData.indexOf("lat")-3;
      QString cityId = cityRequsetData.mid(id_begin,id_end-id_begin);
     */

    QJsonParseError jsonError;
    QJsonDocument   jsonDocument;
    QJsonObject     jsonObject;

    jsonDocument = QJsonDocument::fromJson(cityRequsetData.toUtf8(), &jsonError);
    if (!jsonDocument.isNull()) {
        if (jsonDocument.isObject()) {
            jsonObject = jsonDocument.object();
            if (jsonObject.value("location").isArray()) {
                QJsonArray cityArray = jsonObject.value("location").toArray();
                for (int i = 0; i < cityArray.size(); i++) {
                    jsonObject = cityArray[i].toObject();
                    this->city = jsonObject.value("name").toString();
                    this->cityId = jsonObject.value("id").toString();
                }
            }
        }
    }

    QString dailyUrl = QString("https://devapi.qweather.com/v7/weather/%1d?location=%2&key=30164cac9e1843268d34c50058e92cf4").arg(this->daysNumber).arg(this->cityId);
    qDebug() << "多日天气url：" << dailyUrl << endl;
    QNetworkRequest dailyRequest;
    dailyRequest.setUrl(QUrl(dailyUrl));
    /*connect()函数不会阻塞代码的执行，它只是设置了一个连接，当事件发生时执行相应的槽函数。
      这样可能会导致lambda里面的程序还未执行完就已经在执行后续程序代码，例如这其中就会导致出现两个网络请求返回数据而出错。*/
    //可以再添加一个connect或者使用同步请求即在第一个url完成后发送信号再执行第二个url
    QNetworkReply *dailyReply = netManager->get(dailyRequest);
    connect(dailyReply, &QNetworkReply::finished, this, &Weather::slotDailyRequestFinished);
}

void Weather::slotDailyRequestFinished() {
    QNetworkReply *dailyReply = (QNetworkReply *)sender();
    this->allInfo = codec->toUnicode(dailyReply->readAll());
    dailyReply->deleteLater();
    qDebug() << "查询多日天气json数据：" << allInfo.toStdString().data();

    QString weatherUrl = QString("https://devapi.qweather.com/v7/weather/now?location=%1&key=30164cac9e1843268d34c50058e92cf4").arg(this->cityId);
    qDebug() << "weatherUrl今日url：" << weatherUrl << endl;
    QNetworkRequest weatherRequest;
    weatherRequest.setUrl(QUrl(weatherUrl));
    QNetworkReply *weatherReply = netManager->get(weatherRequest);
    connect(weatherReply, &QNetworkReply::finished, this, &Weather::slotGetWeatherReplyFinished);
}

void Weather::slotGetWeatherReplyFinished() {
    QNetworkReply *weatherReply = (QNetworkReply *)sender();
    QString weatherData = codec->toUnicode(weatherReply->readAll());
    weatherReply->deleteLater();
    qDebug() << "查询今日天气的json数据" << weatherData.toStdString().data();
    QJsonParseError error;
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    jsonDoc = QJsonDocument::fromJson(weatherData.toUtf8(), &error);

    if (!jsonDoc.isNull()) {
        if (jsonDoc.isObject()) {
            jsonObj = jsonDoc.object();
            this->updateTime = jsonObj.value("updateTime").toString();
            this->fxLink = jsonObj.value("fxLink").toString();
            if (jsonObj.contains("now")) {
                QJsonValue value = jsonObj.value("now");
                if (value.isObject()) {     //now 是一个对象
                    QJsonObject todayWeatherObj = value.toObject();
                    this->obsTime    = todayWeatherObj.value("obsTime").toString();
                    this->temp       = todayWeatherObj.value("temp").toString();
                    this->feelsLike  = todayWeatherObj.value("feelsLike").toString();
                    this->icon       = todayWeatherObj.value("icon").toString();
                    this->weatherType = todayWeatherObj.value("text").toString();
                    this->wind       = todayWeatherObj.value("windDir").toString() + "[" + todayWeatherObj.value("windScale").toString() + "级]";
                    this->humidity   = todayWeatherObj.value("humidity").toString();
                    this->precip     = todayWeatherObj.value("precip").toString();
                    this->pressure   = todayWeatherObj.value("pressure").toString();
                    this->vis        = todayWeatherObj.value("vis").toString();
                    this->cloud      = todayWeatherObj.value("cloud").toString();
                    this->isGetData = true;
                }
            }
        }
    }
    if (isGetData) {
        emit this->getDataSuccessedSignal();
    } else {
        emit this->getDataFailedSignal();
    }
}


/**
 * @brief Weather::getDataMap
 * @param ok
 * @return dataMap
 * 获取多日天气，用Map存储
 */
QMap<QString, QMap<QString, QString> > Weather::getDataMap() {
    QJsonParseError err;
    QJsonDocument json_recv = QJsonDocument::fromJson(allInfo.toUtf8(), &err); //解析json数据

    if (!json_recv.isNull()) {
        QJsonObject object = json_recv.object();
        if (object.contains("daily")) {
            QJsonValue value = object.value("daily"); //获取指定key对应的value
            if (value.isArray()) {
                QJsonArray dailyArray = value.toArray();
for (int i = 0; i < dailyArray.size(); i++) {
    object = dailyArray[i].toObject();
    QMap<QString, QString> mapValue;
    mapValue["tempRange"] = object["tempMin"].toString() + "℃~" + object["tempMax"].toString() + "℃"; //daily.tempMin daily.tempMax 预报当天最低温度和最高气温
    mapValue["iconDay"] = object.value("iconDay").toString();                //daily.iconDay 预报天气状况的图标代码，另请参考天气图标项目
    mapValue["textDay"] = object["textDay"].toString();                      //daily.textDay 预报天气状况文字描述，包括阴晴雨雪等天气状态的描述
    mapValue["windDay"] = object["windDirDay"].toString() + "[" + object["windScaleDay"].toString() + "级]"; //daily.windDirDay daily.windScaleDay预报白天风向+白天风力等级
    mapValue["uvIndex"] = object["uvIndex"].toString();                      //daily.uvIndex 紫外线强度指数
    mapValue["sunRiseOrSet"] = object["sunrise"].toString() + "~" + object["sunset"].toString(); //日出日落时间
    dataMap[object.value("fxDate").toString()] = mapValue;                   //daily.fxDate 预报日期
}
return dataMap;
            }
        }
    }
    return dataMap;
}


void Weather::setCityLocation(QString cityLocation, QString days) {
    refresh();//刷新
    this->city = cityLocation;
    this->daysNumber = days;
    setNowCity();
}

QString Weather::GetCityName() {
    return this->city;
}

QString Weather::GetUpdateTime() {
    return this->updateTime;
}
QString Weather::GetObsTime() {
    return this->obsTime;
}
QString Weather::GetFxLink() {
    return this->fxLink;
}
QString Weather::GetTemp() {
    return this->temp;
}
QString Weather::GetFeelsLike() {
    return this->feelsLike;
}
QString Weather::GetIcon() {
    return this->icon;
}
QString Weather::GetWeatherType() {
    return this->weatherType;
}
QString Weather::GetWind() {
    return this->wind;
}
QString Weather::GetHumidity() {
    return this->humidity;
}
QString Weather::GetPrecip() {
    return this->precip;
}
QString Weather::GetPressure() {
    return this->pressure;
}
QString Weather::GetVis() {
    return this->vis;
}
QString Weather::GetCloud() {
    return this->cloud;
}
