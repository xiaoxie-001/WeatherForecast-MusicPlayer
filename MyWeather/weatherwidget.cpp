#include "weatherwidget.h"
#include "ui_weatherwidget.h"
WeatherWidget::WeatherWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WeatherWidget)
{
    ui->setupUi(this);
    UI_Design();
    timer = new QTimer(this);  //定时器显示获取当前时间
    connect(timer, &QTimer::timeout, this, &WeatherWidget::updateCurrent);
    timer->start(1000);

    weather = new Weather(this);
    connect(weather, &Weather::getDataSuccessedSignal, this, &WeatherWidget::ShowThisWeather); //接收成功则SHOW
    connect(weather, &Weather::getDataFailedSignal, this, &WeatherWidget::GetWeatherFailed);

    myMediaPlayer = new MyMusicPlayer;
    connect(myMediaPlayer, &MyMusicPlayer::calculateFinished, this, &WeatherWidget::sethSliderValue);

    timer2 = new QTimer(this);  //定时器0.5s获取一次播放进度条 刷新进度条
    connect(timer2, &QTimer::timeout, myMediaPlayer, &MyMusicPlayer::setSliderPlayProgress);

    myT = new MyThread; //动态分配空间，不能指定父对象
    thread = new QThread(this);//创建子线程
    myT->moveToThread(thread);//把自定义线程加入到子线程中
    connect(myMediaPlayer, &MyMusicPlayer::playPosition, myT, &MyThread::handlePlayPosition);
    connect(myMediaPlayer, &MyMusicPlayer::playDuration, myT, &MyThread::handlePlayDuration);
    connect(myT, &MyThread::completePosition, this, &WeatherWidget::showMusicPosition);
    connect(myT, &MyThread::completeDuration, this, &WeatherWidget::showMusicDuration);
    qDebug() << "主线程号：" << QThread::currentThread();
    thread->start();
}
WeatherWidget::~WeatherWidget() {
    delete svgWidget;
    delete svgRender;
    delete weather;
    thread->quit(); //停止线程
    thread->wait(); //等待线程处理完手头动作
    delete myMediaPlayer;
    delete myT;
    delete ui;
}

void WeatherWidget::updateCurrent() {
    QDateTime currentTime = QDateTime::currentDateTime();
    QString nowDate = currentTime.toString("yyyy-MM-dd dddd");
    QString nowTime = currentTime.toString("hh:mm:ss");
    ui->label_currentTime->setText(nowTime);
    ui->label_currentDate->setText(nowDate);
}

void WeatherWidget::UI_Design() {
    this->setWindowTitle("xtc天气");
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //窗口为无边框，同时保留系统菜单和最小化按钮

    ui->lineEditCityName->setStyleSheet("QLineEdit{background-color: rgba(255,255,225,0.3);border:none;color:#FFFAF0; border-radius:16px;padding:4px 4px}");   //lineEdit圆角样式
    ui->pushButtonQuery->setStyleSheet("QPushButton::hover{font:24px;}" "QPushButton{background:transparent;color:#FFFAF0}");
    ui->pushButtonLocation->setStyleSheet("QPushButton::hover{font:19px;}" "QPushButton{background:rgba(255,255,225,0.2);color:#FFFAF0;border-radius:16px;padding:5px 5px}");
    ui->btnPause->setIcon(QIcon(":/Image/pause.png"));
    ui->btnAddMusic->setStyleSheet("QPushButton::hover{font:19px;}" "QPushButton{background:rgba(255,255,225,0.2);color:#FFDEAD;border-radius:16px;padding:5px 5px}");

    ui->tabWidget->setCurrentWidget(ui->tab);
    ui->tabWidget->setTabText(0, "今日天气");
    ui->tabWidget->setTabIcon(0, QIcon(":/Image/logo.ico"));
    ui->tabWidget->setTabText(1, "未来天气");
    ui->tabWidget->setTabIcon(1, QIcon(":/Image/weather.png"));
    ui->tabWidget->setTabText(2, "音乐播放");
    ui->tabWidget->setTabIcon(2, QIcon(":/Image/musicicon.png"));
    ui->tabWidget->setStyleSheet("QTabWidget:pane {border-top:0px solid #e8f3f9;background:rgba(255,255,255,0.2); border-radius:16px;padding:4px 4px}"
                                 "QTabBar::tab {background:#2F4F4F;color:#FFA500;font:19px;font-weight:bold;}");

    ui->pushButtonMini->setIcon(QIcon(":/Image/Min.png"));   //最小化按钮
    ui->pushButtonMini->setStyleSheet("QPushButton{border:none;color:rgb(122, 197, 205);}" "QPushButton:hover{background-color: #1E90FF;border:none;color:rgb(255, 255, 255);}");
    ui->pushButtonDestroy->setIcon(QIcon(":/Image/Close.png")); //关闭按钮
    ui->pushButtonDestroy->setStyleSheet("QPushButton{border:none;color:rgb(122, 197, 205);}" "QPushButton:hover{background-color: #FF0000;border:none;color:rgb(255, 255, 255);}");

    ui->tableWidget->setColumnWidth(0, 160);  //添加7列
    ui->tableWidget->setColumnWidth(1, 120);
    ui->tableWidget->setColumnWidth(2, 160);
    ui->tableWidget->setColumnWidth(3, 180);
    ui->tableWidget->setColumnWidth(4, 180);
    ui->tableWidget->setColumnWidth(5, 160);
    ui->tableWidget->setColumnWidth(6, 100);

    ui->tableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //水平滚动策略 关闭
    ui->tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//垂直滚动策略
    ui->tableWidget->setShowGrid(false); //无格子线

    //tableWidget表头样式
    ui->tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:transparent;}");
    ui->tableWidget->verticalHeader()->setVisible(false); //隐藏列表头
    //    ui->tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(75);  //设置行高 （除表头外） setDefaultSectionSize设置默认分段大小
    ui->tableWidget->horizontalHeader()->setFixedHeight(90);  //表头行高
    ui->vSliderVolume->setVisible(false);
    ui->lwMusicList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//关闭水平滚动条
    movieToLoad(); //加载gif
}
/**
 * @brief WeatherWidget::movieToLoad
 * 显示一些gif动画效果
 */
void WeatherWidget::movieToLoad() {
    movie = new QMovie(":/Image/movie.gif");   //设置GIF格式背景图
    movie->setScaledSize(QSize(1200, 750)); // 设置GIF动画的尺寸与QLabel的尺寸相同
    ui->label_background->setMovie(movie);
    movie->start();
    ui->label_cover->raise();// 将 label 显示在最上面
    movie2 = new QMovie(":/Image/move3.gif");
    movie2->setScaledSize(QSize(1200, 750)); // 设置GIF动画的尺寸与QLabel的尺寸相同
    ui->label_cover->setMovie(movie2);
    movie2->start();
    movie3 = new QMovie(":/Image/haimianbaby.gif");
    movie3->setScaledSize(QSize(55, 60)); // 设置GIF动画的尺寸与QLabel的尺寸相同
    ui->label_haimianbaby->setMovie(movie3);
    movie3->start();
    movie4 = new QMovie(":/Image/paidaxing.gif");
    movie4->setScaledSize(QSize(55, 60)); // 设置GIF动画的尺寸与QLabel的尺寸相同
    ui->label_paidaxing->setMovie(movie4);
    movie4->start();
}

void WeatherWidget::GetWeatherFailed() {
    if (movie2->isValid()) {
        movie2->stop();
        ui->label_cover->hide();
    }
    QMessageBox::information(this, "查询失败！", "请重新输入或检查网络连接后重试");
    ui->lineEditCityName->setText("");
}

void WeatherWidget::ShowThisWeather() {  //显示信息
    iconPic = ":/icon/icons/" + weather->GetIcon() + ".svg";     // 获取SVG图像路径
    svgRender = new QSvgRenderer(iconPic);
    QPixmap *pixmap = new QPixmap(200, 200);
    pixmap->fill(Qt::transparent);//设置背景透明
    QPainter p(pixmap);
    svgRender->render(&p);
    ui->label_Icon->setPixmap(*pixmap);
    ui->label_Icon->setAlignment(Qt::AlignCenter);

    ui->label_City->setText(weather->GetCityName());
    ui->label_Temp->setText(weather->GetTemp() + "℃");
    ui->label_FeelsLike->setText(weather->GetFeelsLike() + "℃");
    ui->label_WeatherType->setText(weather->GetWeatherType());
    ui->label_Wind->setText(weather->GetWind());
    ui->label_UpdateTime->setText(weather->GetUpdateTime());
    ui->label_ObsTime->setText(weather->GetObsTime());
    ui->label_Humidity->setText(weather->GetHumidity() + "%");
    ui->label_Precip->setText(weather->GetPrecip() + "mm");
    ui->label_Pressure->setText(weather->GetPressure() + "Pa");
    ui->label_Vis->setText(weather->GetVis() + "km");
    ui->label_Cloud->setText(weather->GetCloud() + "%");

    /*添加 超链接*/
    QString link = "<a style=color:orange; href=\"" + weather->GetFxLink() + "\">点击此查看更多天气</a>";
    ui->label_fxLink->setOpenExternalLinks(true);// 允许在外部浏览器中打开链接
    ui->label_fxLink->setTextFormat(Qt::RichText);  // 设置为富文本格式，以支持超链接
    ui->label_fxLink->setText(link);

    temp        = weather->GetTemp().toInt();
    feelTemp    = weather->GetFeelsLike().toInt();
    vis         = weather->GetVis().toInt();
    humidity    = weather->GetHumidity().toInt();
    precip      = weather->GetPrecip().toDouble();
    weatherType = weather->GetWeatherType();

    QMap<QString, QMap<QString, QString>>map = weather->getDataMap();

    ui->tableWidget->clearContents(); //清空表格
    //    for(QMap<QString, QMap<QString, QString>>::Iterator i= map.begin();i!=map.end();i++){
    int n = 0;
    foreach (QString fxDate, map.keys()) {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        QMap<QString, QString>mapvalue = map.value(fxDate);

        ui->tableWidget->setItem(n, 0, new QTableWidgetItem(fxDate));
        ui->tableWidget->setItem(n, 1, new QTableWidgetItem(mapvalue.value("textDay")));
        this->iconPic = ":/icon/icons/" + mapvalue.value("iconDay") + ".svg";
        setIcon(n);
        ui->tableWidget->setItem(n, 3, new QTableWidgetItem(mapvalue.value("tempRange")));
        ui->tableWidget->setItem(n, 4, new QTableWidgetItem(mapvalue.value("windDay")));
        ui->tableWidget->setItem(n, 5, new QTableWidgetItem(mapvalue.value("sunRiseOrSet")));
        ui->tableWidget->setItem(n, 6, new QTableWidgetItem(mapvalue.value("uvIndex")));
        if (n == 0) {
            uvIndex = mapvalue.value("uvIndex").toInt();
            ui->label_UV->setText(mapvalue.value("uvIndex"));
            ui->label_TempRange->setText(mapvalue.value("tempRange"));
            ui->label_sunRiseOrSet->setText(mapvalue.value("sunRiseOrSet"));
        }
        n++;
    }
    /*数据获取完成了，关闭启动动画*/
    if (movie2->isValid()) {
        movie2->stop();
        ui->label_cover->hide();
    }
    setReminder();
}
void WeatherWidget::setIcon(int n) {
    svgWidget = new QSvgWidget(iconPic); // 创建一个 QSvgWidget 来显示 SVG 图像 加载天气图标
    svgWidget->setFixedSize(100, 70);  // 设置图像大小
    ui->tableWidget->setCellWidget(n, 2, svgWidget);// 在表格的第一行第一列插入控件
}
/**
 * @brief WeatherWidget::setReminder
 * 设置提醒
 */
void WeatherWidget::setReminder() {
    QString str, str1, str2, str3, str4, str5;
    if (temp < 13 && feelTemp < 12)
        str1 = "天气较冷，适当增减衣物，预防感冒;\n";
    if (uvIndex > 5)
        str2 = QString("紫外线指数:%1,今日紫外线较强，出门请注意防晒;\n").arg(uvIndex);
    if (vis < 1)
        str3 = QString("能见度:%1,当前雾较大，开车请注意路况，小心驾驶;\n").arg(vis);
    if (humidity < 30)
        str4 = QString("空气湿度:%1,当前空气干燥，注意适当补充水分;\n").arg(humidity);
    if (weatherType.contains("雨") || precip > 0.1)
        str5 = QString("天气:%1,今天可能会下雨，出门请记得带伞;\n").arg(weatherType);
    str = str1 + str2 + str3 + str4 + str5;
    if (str.isNull()) {
        ui->label_warn->setText("");
    } else {
        ui->label_warn->setText("温馨提醒您！");
    }
    ui->WarmReminder->setText(str);
    str.clear();
}

//记录拖动起始位置
void WeatherWidget::mousePressEvent(QMouseEvent *event) {
    startPos = event->globalPos();//获取点击鼠标时的全局坐标
    startPos1 = event->pos();//获取点击鼠标的相对WeatherWidget的坐标
}

//窗口移动
void WeatherWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) { //判断是否是左键按下
        QPoint movePos = event->globalPos() - startPos;//计算鼠标移动位置
        if ( movePos.manhattanLength() > 4) //判断移动像素距离，防抖动
            this->move(event->globalPos() - startPos1);
        return;
    }
}

/*设置选择的天数*/
void WeatherWidget::on_selectDays_3_clicked() {
    ui->selectDays_7->setCheckState(Qt::Unchecked);
    daysNumber = "3";
    weather->setCityLocation(ui->label_City->text(), daysNumber);
}

void WeatherWidget::on_selectDays_7_clicked() {
    ui->selectDays_3->setCheckState(Qt::Unchecked);
    daysNumber = "7";
    weather->setCityLocation(ui->label_City->text(), daysNumber);
}
/**
 * @brief WeatherWidget::on_pushButtonQuery_clicked
 * 查询按钮 先刷新
 */
void WeatherWidget::on_pushButtonQuery_clicked() {
    qDebug() << daysNumber << endl;
    QString editCity = ui->lineEditCityName->text();
    if (editCity.isEmpty()) {
        QMessageBox::information(this, "输入提示", "请输入城市再点击查询");
        return;
    } else if (daysNumber.isEmpty()) {
        QMessageBox::information(this, "输入提示", "请选择查询天数再点击查询");
        return;
    }
    ui->tableWidget->setRowCount(0);//初始化设置为表格行数为0行
    weather->setCityLocation(editCity, daysNumber);
}

/**
 * @brief WeatherWidget::on_pushButtonLocation_clicked
 * 定位按钮
 */
void WeatherWidget::on_pushButtonLocation_clicked() {
    weather->initialization();
}

/**
 * @brief WeatherWidget::on_pushButtonMini_clicked
 *  窗口最小化按钮
 */
void WeatherWidget::on_pushButtonMini_clicked() {
    showMinimized();
    //showMaximized();//最大化
}
/**
 * @brief WeatherWidget::on_pushButtonDestroy_clicked
 * 窗口关闭按钮
 */
void WeatherWidget::on_pushButtonDestroy_clicked() {
    this->close();
}


/*
  cc1plus.exe: out of memory allocating 1073745919 bytes
  使用qrc资源文件，也就是在QT的工程中添加资源文件，就是添加的资源文件（如qrc.cpp）会直接被存放到静态数组中，从而一直占用内存，使内存的利用率不高。
  有的时候如果资源过大，可能编译都无法通过，会造成out of memory的错误。
  解决办法：
  在工程PRO文件中，添加：
  CONFIG += resources_big
 */

/**
 * @brief WeatherWidget::on_btnAddMusic_clicked
 * 添加音乐文件列表
 */
void WeatherWidget::on_btnAddMusic_clicked() {
    QFileDialog fileDialog(this);//创建文件对话框
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setNameFilter("Music Files (*.mp3 *.wav *.wma)");
    if (fileDialog.exec()) { //获取要添加的列表
        QStringList fileNames = fileDialog.selectedFiles();
        QString fileName = fileNames[0];
        ui->lwMusicList->addItem(fileName); //将每个文件添加到QListWidget控件
        musicFileIndex[fileName] = countFiles + 1; // 将索引he文件路径添加到映射中
        qDebug() << "value:" << musicFileIndex.value(fileName) << "key:" << musicFileIndex.key(countFiles + 1);
    }
    countFiles = ui->lwMusicList->count();
    qDebug() << "总添加音乐count:" << countFiles;
}

/**
 * @brief WeatherWidget::on_btnPause_clicked
 * 播放和暂停
 */
void WeatherWidget::on_btnPause_clicked() {
    if (this->mediaPath.isNull()) {
        return;
    } else {
        if (myMediaPlayer->isToplay()) { //为真 true 正在播放,将其关闭
            ui->btnPause->setIcon(QIcon(":/Image/pause.png"));
            myMediaPlayer->pauseMusic();
            timer2->stop();
        } else { //false 没有播放，点击后播放
            ui->btnPause->setIcon(QIcon(":/Image/stop.png"));
            myMediaPlayer->playMusic(this->mediaPath);
            timer2->start(300);
        }
    }
}

/**
 * @brief WeatherWidget::sethSliderValue
 * @param number
 * 设置进度条 当歌曲播放时，返回当前播放进度
 */
void WeatherWidget::sethSliderValue(const int &number) {
    ui->hSliderPlayProgress->setValue(number);
}

/**
 * @brief WeatherWidget::on_hSliderPlayProgress_valueChanged
 * @param value
 * 当滑动滑块时关闭播放，并且关闭进度条刷新定时器
 *
 * 目前设置的只有滑动有效，直接点击跳转位置无效
 */
void WeatherWidget::on_hSliderPlayProgress_valueChanged(int value) {
    if (ui->hSliderPlayProgress->isSliderDown()) { //isSliderDown判断滑块是否被按下
        ui->btnPause->setIcon(QIcon(":/Image/pause.png"));
        myMediaPlayer->pauseMusic(); //关闭播放
        if (timer2->isActive()) {
            timer2->stop();//关闭定时器
        }
        // 记录滑块的值
        this->sliderPlayValue = value;
    }
}

/**
 * @brief WeatherWidget::on_hSliderPlayProgress_sliderReleased
 * 当释放滑块后读取现在的值
 */
void WeatherWidget::on_hSliderPlayProgress_sliderReleased() {
    if (mediaPath.isNull()) {
        return;
    }
    myMediaPlayer->setMusicPosition(this->sliderPlayValue); //滑块的值传递改变音乐位置
    ui->btnPause->setIcon(QIcon(":/Image/stop.png")); //开始播放在setMusicPosition中完成
    timer2->start(300);
}

/**
 * @brief WeatherWidget::on_vSliderVolume_valueChanged
 * @param value
 * 设置音乐播放器的音量
 */
void WeatherWidget::on_vSliderVolume_valueChanged(int value) {
    myMediaPlayer->setMusicVolume(value);
}

/**
 * @brief WeatherWidget::on_btnVolume_clicked
 * 点击按钮设置音量条显示
 */
void WeatherWidget::on_btnVolume_clicked() {
    if (ui->vSliderVolume->isVisible()) {
        ui->vSliderVolume->setVisible(false);
    } else {
        ui->vSliderVolume->setVisible(true);
    }
}
/**
 * @brief WeatherWidget::on_vSliderVolume_sliderReleased
 * 音量条设置好后隐藏
 */
void WeatherWidget::on_vSliderVolume_sliderReleased() {
    ui->vSliderVolume->setVisible(false);
}

/**
 * @brief WeatherWidget::on_lwMusicList_itemDoubleClicked
 * @param item
 * 双击wMusicList清单播放
 */
void WeatherWidget::on_lwMusicList_itemDoubleClicked(QListWidgetItem *item) {
    timer2->stop();
    this->mediaPath = item->text();
    qDebug() << "双击成功" << mediaPath << endl;
    myMediaPlayer->pauseMusic();
    ui->btnPause->setIcon(QIcon(":/Image/stop.png"));
    myMediaPlayer->playMusic(this->mediaPath);
    timer2->start(300);
}
/*单击选中*/
void WeatherWidget::on_lwMusicList_itemClicked(QListWidgetItem *item) {
    this->mediaPath = item->text();
}

/**
 * @brief WeatherWidget::on_btnLast_clicked
 * 上一首
 */
void WeatherWidget::on_btnLast_clicked() {
    if (this->mediaPath.isNull()) {
        return;
    }
    QListWidgetItem *item = ui->lwMusicList->item(musicFileIndex.value(mediaPath) - 1);
    item->setSelected(false);        //设置是否选中 选中则有高亮 切换歌曲的时候当前这个高亮关闭

    valueindex = musicFileIndex.value(mediaPath) - 1; //通过哈希表的键找到对应的索引值 索引值必须大于等于0
    if (valueindex >= 1) {
        setPreOrNextPlay();
    } else {     //设置播放列表循环
        valueindex = countFiles;
        setPreOrNextPlay();
    }
}
/**
 * @brief WeatherWidget::on_btnNext_clicked
 * 下一首
 */
void WeatherWidget::on_btnNext_clicked() {
    if (this->mediaPath.isNull()) {
        return;
    }
    QListWidgetItem *item = ui->lwMusicList->item(musicFileIndex.value(mediaPath) - 1);
    item->setSelected(false);        //设置是否选中 选中则有高亮 切换歌曲的时候当前这个高亮关闭

    valueindex = musicFileIndex.value(mediaPath) + 1; //通过哈希表的键找到对应的索引值 索引值必须大于等于0
    if (valueindex <= countFiles) {
        qDebug() << "valueindex:" << valueindex;
        setPreOrNextPlay();
    } else {     //设置播放列表循环
        valueindex = 1;
        setPreOrNextPlay();
    }
}
/**
 * @brief WeatherWidget::setPreOrNextPlay
 * 当点击上一曲/下一曲后如果成功则播放
 */
void WeatherWidget::setPreOrNextPlay() {
    mediaPath = musicFileIndex.key(valueindex); //通过对应的索引值找到相应的键值
    qDebug() << "value:" << valueindex << "key:" << mediaPath;
    QListWidgetItem *item = ui->lwMusicList->item(musicFileIndex.value(mediaPath) - 1);
    item->setSelected(true);        //设置是否选中 选中则有高亮

    timer2->stop();
    myMediaPlayer->pauseMusic();
    //先关闭，然后播放
    ui->btnPause->setIcon(QIcon(":/Image/stop.png"));
    myMediaPlayer->playMusic(this->mediaPath);
    timer2->start(300);
}
/**
 * @brief WeatherWidget::setMusicTime
 * @param musicTime
 * 显示音乐播放进行时 当前时间
 */
void WeatherWidget::showMusicPosition(QString time) {
    ui->lblPlayTime->setText(time);
}
/**
 * @brief WeatherWidget::setMusicDuration
 * @param musicTime
 * 显示音乐总时间
 */
void WeatherWidget::showMusicDuration(QString time) {
    ui->lblMusicTime->setText(time);
}
