#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "waypoint.h"
#include "enemy.h"
#include "bullet.h"
#include "audioplayer.h"
#include "plistreader.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtGlobal>
#include <QMessageBox>
#include <QTimer>
#include <QXmlStreamReader>
#include <QtDebug>
#include <utility.h>
#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QAction>
#include <QPushButton>

static const int TowerCost = 300;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, m_waves(0)
    , m_playerHp(55)//原先5
    , m_playrGold(1000)//原先1000
    , m_HPpercent_forenemy(-1)
    , targetenemy(NULL)
	, m_gameEnded(false)
	, m_gameWin(false)
    , ispause(0)
    , m_selectmenu(NULL)
    , m_upgrademenu(NULL)
    , m_current(0)
    , temp(0,0)//
    , m_towercurrent(0)
{
	ui->setupUi(this);
    setFixedSize(1160,640);//调整画面大小origin:960*640
    //原先：1160，640
    stage = 1;
    preLoadWavesInfo();//载入敌人进攻波数
    loadTowerPositions();//载入塔的位置
    addWayPoints();//敌人进攻路线

	m_audioPlayer = new AudioPlayer(this);
	m_audioPlayer->startBGM();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateMap()));
    timer->start(1);//每1ms刷新一次
    QCursor *mycursor= new QCursor(QPixmap(":/image/icon_gold.png"),-1,-1);
    QWidget::setCursor(*mycursor);//设置鼠标
	// 设置300ms后游戏启动
    QTimer::singleShot(300, this, SLOT(gameStart()));
    setMenu();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setMenu()
{

    m_selectmenu=new QMenu(this);
    QAction *p = new QAction(m_selectmenu);
    QAction *q = new QAction(m_selectmenu);
    p->setText("建造地狱塔!");
    q->setText("建造光塔!");
    m_selectmenu->addAction(p);
    m_selectmenu->addSeparator();
    m_selectmenu->addAction(q);
    connect(q, SIGNAL(triggered()), this, SLOT(setLightTower()));
    connect(p, SIGNAL(triggered()), this, SLOT(setTower()));

    m_upgrademenu = new QMenu(this);
    QAction *info = new QAction(m_upgrademenu);
    QAction *up0 = new QAction(m_upgrademenu);
    QAction *up1 = new QAction(m_upgrademenu);
    QAction *up2 = new QAction(m_upgrademenu);
    QAction *up3 = new QAction(m_upgrademenu);
//    if(temptower != NULL)
//        up0->setText(tr("全面升级：伤害：%1 ---> %2").arg(temptower->m_damage).arg(temptower->m_damage+10));
//    if(templighttower != NULL)
//        up0->setText(tr("全面升级：伤害：%1 ---> %2").arg(templighttower->m_damage).arg(templighttower->m_damage+10));
//
    info->setText("显示基本信息");
    up0->setText("全面升级");
    up1->setText("升级伤害");
    up2->setText("升级范围");
    up3->setText("移除防御塔");
    m_upgrademenu->addAction(info);
    m_upgrademenu->addSeparator();
    m_upgrademenu->addAction(up0);
    m_upgrademenu->addSeparator();
    m_upgrademenu->addAction(up1);
    m_upgrademenu->addSeparator();
    m_upgrademenu->addAction(up2);
    m_upgrademenu->addSeparator();
    m_upgrademenu->addAction(up3);
    connect(info, SIGNAL(triggered()), this, SLOT(showinfo()));
    connect(up0,SIGNAL(triggered()), this, SLOT(upgradelevel()));
    connect(up1,SIGNAL(triggered()), this, SLOT(upgradeDamage()));
    connect(up2,SIGNAL(triggered()), this, SLOT(upgradeRange()));
    connect(up3,SIGNAL(triggered()), this, SLOT(removethis()));
}
void MainWindow::showinfo(){

    if(temptower){tower_for_show = temptower;light_tower_for_show = NULL;}
    if(templighttower){light_tower_for_show = templighttower;tower_for_show = NULL;}
}
void MainWindow::removethis(){
    foreach (Tower *tower, m_towersList) {//清除地狱塔
        if(collisionWithCircle(tower->getpos(), 20, temp, 20)){
            auto it=m_towerPositionsList.begin();
            while(it!=m_towerPositionsList.end()){
                if(collisionWithCircle(it->centerPos(), 20, temp, 20)){
                    it->setHasTower(0);break;
                }
                ++it;
            }
            foreach(Enemy* enemy, m_enemyList){
                if(enemy->m_attackedTowersList.indexOf(tower)!=-1){
                    enemy->m_attackedTowersList.removeOne(tower);
                }
            }
            //tower->getRemoved();不能写这个！！写了就闪退了
            foreach (Enemy *enemy, enemyList())
            {
                if(enemy->m_attackedTowersList.indexOf(tower)!=-1)
                enemy->gotLostSight_by_tower(tower);
            }
            tower->m_chooseEnemy = NULL;
            m_towersList.removeOne(tower);
            tower->originrange();//这里的操作：将塔移出游戏的塔的列表，但它本身存在，因此将其攻击范围设置为0
            break;
        }
    }
    foreach (light_tower *tower, m_light_towersList) {//清除光塔
        if(collisionWithCircle(tower->getpos(),20, temp, 20)){
            auto it=m_towerPositionsList.begin();
            while(it!=m_towerPositionsList.end()){
                if(collisionWithCircle(it->centerPos(), 20, temp, 20)){
                    it->setHasTower(0);break;
                }
                ++it;
            }
            foreach(Enemy* enemy, m_enemyList){
                if(enemy->m_attackedlight_towersList.indexOf(tower)!=-1){
                    enemy->m_attackedlight_towersList.removeOne(tower);
                }
            }
            //tower->getRemoved();不能写这个！！写了就闪退了
            m_light_towersList.removeOne(tower);
            tower->originrange();//这里的操作：将塔移出游戏的塔的列表，但它本身存在，因此将其攻击范围设置为0
            break;
        }
    }
}
void MainWindow::upgradelevel(){
    if(temptower){
        temptower->level++;
        temptower->setrange(50);
        temptower->setdamage(5);
    }
    if(templighttower){
        templighttower->level++;
        templighttower->setrange(50);
        templighttower->setdamage(50);
    }
}
void MainWindow::upgradeDamage(){
    if(temptower){
        temptower->setdamage(5);
    }
    if(templighttower){
        templighttower->setdamage(50);
    }
}
void MainWindow::upgradeRange(){
    if(temptower){
        temptower->setrange(50);
    }
    if(templighttower){
        templighttower->setrange(50);
    }
}
void MainWindow::setLightTower(){
    if(!m_towerPositionsList.at(m_current).hasTower())
    {
        m_towerPositionsList.at(m_current).m_hasTower = 1;
        light_tower *tower = new light_tower(m_towerPositionsList.at(m_current).centerPos(), this);
        m_light_towersList.push_back(tower);
        m_playrGold -= TowerCost + m_towersList.size() * 100;
    }
}
void MainWindow::setTower(){
    if(!m_towerPositionsList.at(m_current).hasTower())
    {
        m_towerPositionsList.at(m_current).m_hasTower = 1;
        Tower *tower = new Tower(m_towerPositionsList.at(m_current).centerPos(), this);
        m_towersList.push_back(tower);
        m_playrGold -= TowerCost + m_towersList.size() * 100;
    }
}
void MainWindow::loadTowerPositions()
{
	QFile file(":/config/TowersPosition.plist");
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		QMessageBox::warning(this, "TowerDefense", "Cannot Open TowersPosition.plist");
		return;
	}

	PListReader reader;
	reader.read(&file);

	QList<QVariant> array = reader.data();
	foreach (QVariant dict, array)
	{
		QMap<QString, QVariant> point = dict.toMap();
		int x = point.value("x").toInt();
		int y = point.value("y").toInt();
        m_towerPositionsList.push_back(QPoint(x, y));//读取TowersPosition中的文件
	}

	file.close();
}
void MainWindow::paintEvent(QPaintEvent *)//有关画图的所有函数都集合在这当中
{
    if (m_gameEnded || m_gameWin)//如果游戏结束，使屏幕中央出现结算
	{
		QString text = m_gameEnded ? "YOU LOST!!!" : "YOU WIN!!!";
		QPainter painter(this);
        painter.setPen(QPen(Qt::red));
		painter.drawText(rect(), Qt::AlignCenter, text);
		return;
    }
    if(stage==1){
    QPixmap cachePix(":/image/sample_map.png");

    cachePix= cachePix.scaled(960,640);//调整背景图片大小
    QPainter cachePainter(&cachePix);//画出塔防地图
    foreach (const TowerPosition &towerPos, m_towerPositionsList)
		towerPos.draw(&cachePainter);

    foreach (/*const*/ light_tower *tower, m_light_towersList){
        tower->draw(&cachePainter);
        if(light_tower_for_show == tower){
            cachePainter.save();
            cachePainter.setPen(Qt::red);
            cachePainter.drawEllipse(tower->getpos(),30,30);
            cachePainter.restore();
        }
    }
    foreach (/*const*/ Tower *tower, m_towersList){
		tower->draw(&cachePainter);
        if(tower_for_show == tower){
            cachePainter.save();
            cachePainter.setPen(Qt::red);
            cachePainter.drawEllipse(tower->getpos(),30,30);
            cachePainter.restore();
        }
    }
    foreach (const WayPoint *wayPoint, m_wayPointsList)
		wayPoint->draw(&cachePainter);

    foreach (const Enemy *enemy, m_enemyList){
		enemy->draw(&cachePainter);
        if(targetenemy==enemy){
            cachePainter.save();
            cachePainter.setPen(Qt::red);
            cachePainter.drawEllipse(enemy->pos(),30,30);
            cachePainter.restore();
        }
    }

    foreach (const Bullet *bullet, m_bulletList)
		bullet->draw(&cachePainter);

    foreach (const bullet_wandering *bullet, m_bullet_wanderingList) {
        bullet->draw(&cachePainter);
    }
    foreach (light_tower *ltower, this->m_light_towersList)
    {
        foreach(Enemy *enemy, this->m_enemyList)
        {
            if(enemy->m_attackedlight_towersList.indexOf(ltower)!=-1 && collisionWithCircle(ltower->getpos(), ltower->getrange(), enemy->pos(), 0))
            {
                cachePainter.setPen(QColor(255+ltower->x/*0*/,255+ltower->y/*160*/,255+ltower->z/*230*/));
                cachePainter.drawLine(enemy->pos(),QPoint(ltower->posx()+5,ltower->posy()-40));
            }
        }
    }


	drawHP(&cachePainter);
    drawPlayerGold(&cachePainter);
    drawcurrentHP(&cachePainter);

	QPainter painter(this);
    painter.drawPixmap(0, 0, cachePix);//这一步画出地图
    drawWave(&painter);
    drawinfo(&painter);
    }
    if(stage==2){
        QPixmap cachePix(":/image/map.jpg");

        cachePix= cachePix.scaled(960,640);//调整背景图片大小
        QPainter cachePainter(&cachePix);//画出塔防地图
        foreach (const TowerPosition &towerPos, m_towerPositionsList)
            towerPos.draw(&cachePainter);

        foreach (const light_tower *tower, m_light_towersList)
            tower->draw(&cachePainter);

        foreach (const Tower *tower, m_towersList)
            tower->draw(&cachePainter);

        foreach (const WayPoint *wayPoint, m_wayPointsList)
            wayPoint->draw(&cachePainter);

        foreach (const Enemy *enemy, m_enemyList){
            enemy->draw(&cachePainter);
            if(targetenemy==enemy){
                cachePainter.save();
                cachePainter.setPen(Qt::red);
                cachePainter.drawEllipse(enemy->pos(),30,30);
                cachePainter.restore();
            }
        }

        foreach (const Bullet *bullet, m_bulletList)
            bullet->draw(&cachePainter);

        foreach (const bullet_wandering *bullet, m_bullet_wanderingList) {
            bullet->draw(&cachePainter);
        }
        foreach (light_tower *ltower, this->m_light_towersList)
        {
            foreach(Enemy *enemy, this->m_enemyList)
            {
                if(enemy->m_attackedlight_towersList.indexOf(ltower)!=-1 && collisionWithCircle(ltower->getpos(), ltower->getrange(), enemy->pos(), 0))
                {
                    cachePainter.setPen(QColor(255+ltower->x/*0*/,255+ltower->y/*160*/,255+ltower->z/*230*/));
                    cachePainter.drawLine(enemy->pos(),QPoint(ltower->posx()+5,ltower->posy()-40));
                }
            }
        }

        drawHP(&cachePainter);
        drawPlayerGold(&cachePainter);
        drawcurrentHP(&cachePainter);


        QPainter painter(this);
        painter.drawPixmap(0, 0, cachePix);//这一步画出地图
        drawinfo(&painter);
        drawWave(&painter);
    }
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_I)
    {
        foreach (Enemy* enemy, enemyList()) {
            enemy->setmaxcurrentHP();//将所有敌人的血量回满
        }
    }
    if(event->key()==Qt::Key_O){
        if(enemyList().indexOf(targetenemy)!=-1){
            removedEnemy(targetenemy);//秒杀targetenemy这个敌人
        }
    }
    if(event->key()==Qt::Key_P){
        foreach (Enemy* enemy, enemyList()) {
            if(collisionWithCircle(enemy->pos(), 30, targetenemy->pos(), 30)){
                enemy->setmaxcurrentHP();//使targetenemy60码范围内全满生命值
            }
        }
    }
}
void MainWindow::mousePressEvent(QMouseEvent *event)//有关鼠标点击的所有操作,如果有两张地图所点击的内容不一样呢？如何做？
{
    QPoint pressPos = event->pos();
    temp = pressPos;
    foreach (Tower *tower, m_towersList) {
        if(collisionWithCircle(tower->getpos(), 30, temp ,30)){
            temptower = tower;
            templighttower = NULL;
            break;
        }
    }
    foreach (light_tower *tower, m_light_towersList) {
        if(collisionWithCircle(tower->getpos(), 30,temp, 30)){
            templighttower = tower;
            temptower = NULL;
            break;
        }
    }
    if(event->button()==Qt::LeftButton)
    {
        foreach(Enemy* enemy, enemyList())
        {
            if(collisionWithCircle(enemy->pos(),10, pressPos, 10))
            {
                targetenemy = enemy;
                break;
            }
        }
    }
    if(event->button()==Qt::LeftButton){
        for(int i=0;i!=m_towerPositionsList.size();i++){
            if(m_towerPositionsList.at(i).containPoint(pressPos)){
                if(!m_towerPositionsList.at(i).hasTower()){
                    m_selectmenu->popup(event->globalPos());
                    m_current = i;
                    break;
                }
                else{
                    m_upgrademenu->popup(event->globalPos());
                    m_current = i;
                    break;
                }
            }
        }
    }
}
void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint pos=event->pos();

//    foreach (Tower *tower, m_towersList) {//双击清除地狱塔
//        if(collisionWithCircle(tower->getpos(), 20, pos, 20)){
//            auto it=m_towerPositionsList.begin();
//            while(it!=m_towerPositionsList.end()){
//                if(collisionWithCircle(it->centerPos(), 20, pos, 20)){
//                    it->setHasTower(0);break;
//                }
//                ++it;
//            }

//            foreach(Enemy* enemy, m_enemyList){
//                if(enemy->m_attackedTowersList.indexOf(tower)!=-1){
//                    enemy->m_attackedTowersList.removeOne(tower);
//                }
//            }

//            //tower->getRemoved();不能写这个！！写了就闪退了
//            m_towersList.removeOne(tower);
//            break;
//        }
//    }
//    foreach (light_tower *tower, m_light_towersList) {//双击清除光塔
//        if(collisionWithCircle(tower->getpos(),20, pos, 20)){
//            auto it=m_towerPositionsList.begin();
//            while(it!=m_towerPositionsList.end()){
//                if(collisionWithCircle(it->centerPos(), 20, pos, 20)){
//                    it->setHasTower(0);break;
//                }
//                ++it;
//            }
//            foreach(Enemy* enemy, m_enemyList){
//                if(enemy->m_attackedlight_towersList.indexOf(tower)!=-1){
//                    enemy->m_attackedlight_towersList.removeOne(tower);
//                }
//            }
//            //tower->getRemoved();不能写这个！！写了就闪退了
//            m_light_towersList.removeOne(tower);
//            break;
//        }
//    }


}
bool MainWindow::canBuyTower() const//如果金钱大于塔的价格，则返回真
{
    if (m_playrGold >= TowerCost + m_towersList.size() * 100)//塔的价格 = 300 + 塔的个数 * 100
		return true;
	return false;
}
void MainWindow::drawWave(QPainter *painter)
{
    painter->setPen(QPen(Qt::white));
    painter->drawText(QRect(980,150,200,25), QString("当前关卡 : %1").arg(stage));
    painter->drawText(QRect(980,175,200,25), QString("当前波数 : %1").arg(m_waves + 1));
}
void MainWindow::drawcurrentHP(QPainter *painter)
{
    /*
    if(enemyList().indexOf(targetenemy)!=-1 && targetenemy!=NULL){
        painter->setPen(QPen(Qt::black));
        double temporary = targetenemy->getcurrentHp() * 10000 / targetenemy->getmaxHP() / 100.0;
        QString str = QString::number(temporary, 'd', 2);
        painter->drawText(QRect(200,25,400,55),QString("PERCENT : %1%    %2   /  %3").arg(str).arg(targetenemy->getcurrentHp()).arg(targetenemy->getmaxHP()));
    }
    else{
        painter->drawText(QRect(200,25,400,55),QString("PERCENT : %1%    %2   /  %3").arg(0).arg(0).arg(0));//不能删除一个NULL
    }
    */
    //这部分内容放到drawinfo()完成
}
void MainWindow::drawHP(QPainter *painter)
{
    painter->setPen(QPen(Qt::black));
	painter->drawText(QRect(30, 5, 100, 25), QString("HP : %1").arg(m_playerHp));
}
void MainWindow::drawPlayerGold(QPainter *painter)
{
    painter->setPen(QPen(Qt::black));
	painter->drawText(QRect(200, 5, 200, 25), QString("GOLD : %1").arg(m_playrGold));
}
void MainWindow::doGameOver()
{
	if (!m_gameEnded)
	{
		m_gameEnded = true;
		// 此处应该切换场景到结束场景
		// 暂时以打印替代,见paintEvent处理
	}
}
void MainWindow::drawinfo(QPainter *painter){
    if(enemyList().indexOf(targetenemy)!=-1){
        painter->setPen(QPen(Qt::white));
        //painter->drawText(QRect(960,200,1160,400),QString("这是一个示范%1").arg(targetenemy->getcurrentHp()));
        double temporary = targetenemy->getcurrentHp() * 10000 / targetenemy->getmaxHP() / 100.0;
        QString str = QString::number(temporary, 'd', 2);
        if(targetenemy->kind == 1)
        painter->drawText(QRect(980,200,160,600),
                          QString("当前血量与百分比：\n%1%   \n%2  /   %3\n描述：\n这是一个非常普通的敌人\n千万不要大意了！")
                          .arg(str).arg(targetenemy->getcurrentHp()).arg(targetenemy->getmaxHP())
                          );
    }
    //if(tower_for_show){
    if(m_towersList.indexOf(tower_for_show)!=-1){
        painter->setPen(QPen(Qt::white));
        painter->drawText(QRect(980,500,160,600),
                          QString("伤害：%1\n攻击范围：%2\n")
                          .arg(tower_for_show->m_damage).arg(tower_for_show->m_attackRange));
    }
    else
    //if(light_tower_for_show){
    if(m_light_towersList.indexOf(light_tower_for_show)!=-1){
        painter->setPen(QPen(Qt::white));
        painter->drawText(QRect(980,500,160,600),
                          QString("伤害：%1\n攻击范围：%2\n")
                          .arg(light_tower_for_show->m_damage).arg(light_tower_for_show->getrange()));
    }
}
void MainWindow::awardGold(int gold)
{
	m_playrGold += gold;
    update();//更新窗口画面！
}
AudioPlayer *MainWindow::audioPlayer() const
{
	return m_audioPlayer;
}
void MainWindow::addWayPoints()//设置路线转弯的点坐标
{
    //WayPoint *wayPoint1 = new WayPoint(QPoint(420, 285));
    WayPoint *wayPoint1 = new WayPoint(QPoint(10, 570));
    m_wayPointsList.push_back(wayPoint1);

    //WayPoint *wayPoint2 = new WayPoint(QPoint(35, 285));
    WayPoint *wayPoint2 = new WayPoint(QPoint(30, 570));
	m_wayPointsList.push_back(wayPoint2);
	wayPoint2->setNextWayPoint(wayPoint1);

    //WayPoint *wayPoint3 = new WayPoint(QPoint(35, 195));
    WayPoint *wayPoint3 = new WayPoint(QPoint(30, 470));
	m_wayPointsList.push_back(wayPoint3);
	wayPoint3->setNextWayPoint(wayPoint2);

    //WayPoint *wayPoint4 = new WayPoint(QPoint(445, 195));
    WayPoint *wayPoint4 = new WayPoint(QPoint(120, 470));
	m_wayPointsList.push_back(wayPoint4);
	wayPoint4->setNextWayPoint(wayPoint3);

    //WayPoint *wayPoint5 = new WayPoint(QPoint(445, 100));
    WayPoint *wayPoint5 = new WayPoint(QPoint(120, 376));
    m_wayPointsList.push_back(wayPoint5);
	wayPoint5->setNextWayPoint(wayPoint4);

    //WayPoint *wayPoint6 = new WayPoint(QPoint(35, 100));
    WayPoint *wayPoint6 = new WayPoint(QPoint(274, 376));
    m_wayPointsList.push_back(wayPoint6);
	wayPoint6->setNextWayPoint(wayPoint5);

    WayPoint *wayPoint7 = new WayPoint(QPoint(274, 470));
    m_wayPointsList.push_back(wayPoint7);
    wayPoint7->setNextWayPoint(wayPoint6);

    WayPoint *wayPoint8 = new WayPoint(QPoint(514, 470));
    m_wayPointsList.push_back(wayPoint8);
    wayPoint8->setNextWayPoint(wayPoint7);

    WayPoint *wayPoint9 = new WayPoint(QPoint(514, 580));
    m_wayPointsList.push_back(wayPoint9);
    wayPoint9->setNextWayPoint(wayPoint8);

    WayPoint *wayPoint10 = new WayPoint(QPoint(840, 580));
    m_wayPointsList.push_back(wayPoint10);
    wayPoint10->setNextWayPoint(wayPoint9);

    WayPoint *wayPoint11 = new WayPoint(QPoint(840, 480));
    m_wayPointsList.push_back(wayPoint11);
    wayPoint11->setNextWayPoint(wayPoint10);

    WayPoint *wayPoint12 = new WayPoint(QPoint(600, 480));
    m_wayPointsList.push_back(wayPoint12);
    wayPoint12->setNextWayPoint(wayPoint11);

    WayPoint *wayPoint13 = new WayPoint(QPoint(600, 376));
    m_wayPointsList.push_back(wayPoint13);
    wayPoint13->setNextWayPoint(wayPoint12);

    WayPoint *wayPoint14 = new WayPoint(QPoint(760, 376));
    m_wayPointsList.push_back(wayPoint14);
    wayPoint14->setNextWayPoint(wayPoint13);

    WayPoint *wayPoint15 = new WayPoint(QPoint(760, 270));
    m_wayPointsList.push_back(wayPoint15);
    wayPoint15->setNextWayPoint(wayPoint14);

    WayPoint *wayPoint16 = new WayPoint(QPoint(600, 270));
    m_wayPointsList.push_back(wayPoint16);
    wayPoint16->setNextWayPoint(wayPoint15);

    WayPoint *wayPoint17 = new WayPoint(QPoint(600, 170));
    m_wayPointsList.push_back(wayPoint17);
    wayPoint17->setNextWayPoint(wayPoint16);

    WayPoint *wayPoint18 = new WayPoint(QPoint(840, 170));
    m_wayPointsList.push_back(wayPoint18);
    wayPoint18->setNextWayPoint(wayPoint17);

    WayPoint *wayPoint19 = new WayPoint(QPoint(840, 60));
    m_wayPointsList.push_back(wayPoint19);
    wayPoint19->setNextWayPoint(wayPoint18);

    WayPoint *wayPoint20 = new WayPoint(QPoint(514, 60));
    m_wayPointsList.push_back(wayPoint20);
    wayPoint20->setNextWayPoint(wayPoint19);

    WayPoint *wayPoint21 = new WayPoint(QPoint(514, 170));
    m_wayPointsList.push_back(wayPoint21);
    wayPoint21->setNextWayPoint(wayPoint20);

    WayPoint *wayPoint22 = new WayPoint(QPoint(280, 170));
    m_wayPointsList.push_back(wayPoint22);
    wayPoint22->setNextWayPoint(wayPoint21);

    WayPoint *wayPoint23 = new WayPoint(QPoint(280, 270));
    m_wayPointsList.push_back(wayPoint23);
    wayPoint23->setNextWayPoint(wayPoint22);

    WayPoint *wayPoint24 = new WayPoint(QPoint(120, 270));
    m_wayPointsList.push_back(wayPoint24);
    wayPoint24->setNextWayPoint(wayPoint23);

    WayPoint *wayPoint25 = new WayPoint(QPoint(120, 170));
    m_wayPointsList.push_back(wayPoint25);
    wayPoint25->setNextWayPoint(wayPoint24);

    WayPoint *wayPoint26 = new WayPoint(QPoint(40, 170));
    m_wayPointsList.push_back(wayPoint26);
    wayPoint26->setNextWayPoint(wayPoint25);

    WayPoint *wayPoint27 = new WayPoint(QPoint(40, 50));
    m_wayPointsList.push_back(wayPoint27);
    wayPoint27->setNextWayPoint(wayPoint26);

    WayPoint *wayPoint28 = new WayPoint(QPoint(10, 50));
    m_wayPointsList.push_back(wayPoint28);
    wayPoint28->setNextWayPoint(wayPoint27);

    //wayPoint1->setNextWayPoint(wayPoint28);
}
void MainWindow::getHpDamage(int damage/* = 1*/)//基地受到伤害
{
	m_audioPlayer->playSound(LifeLoseSound);
	m_playerHp -= damage;
	if (m_playerHp <= 0)
        doGameOver();//如果受伤后体力小于等于0，则执行doGameOver()，触发游戏结束
}
void MainWindow::removedEnemy(Enemy *enemy)
{
    if(enemy){
    Q_ASSERT(enemy);        //断言，用于调试代码。
                            //如果括号内值为真，则内联一个空函数；值为假，则断开，程序反馈这里出bug。下同

    m_enemyList.removeOne(enemy);//每次消灭一个敌人，执行一次removedEnemy()
    delete enemy;//移除队列中一个enemy

    if (m_enemyList.empty())//如果队列空，说明这波敌人消灭完了
	{

        ++m_waves;          //则波数+1
        if(m_waves==2){

            stage = 2;
            m_towersList.clear();
            m_light_towersList.clear();
            auto it = m_towerPositionsList.begin();
            while(it!=m_towerPositionsList.end()){
                it->setHasTower(0);
                ++it;
            }
            m_wayPointsList.at(0)->setNextWayPoint(m_wayPointsList.at(27));
        }
        if (!loadWave())    //见274行loadWave()定义
		{
			m_gameWin = true;
			// 游戏胜利转到游戏胜利场景
			// 这里暂时以打印处理
		}
	}
    }
}
void MainWindow::removedTower(Tower *tower)
{
    Q_ASSERT(tower);

    m_towersList.removeOne(tower);
    delete tower;
}
void MainWindow::removed_light_tower(light_tower *tower)
{
    Q_ASSERT(tower);
    m_light_towersList.removeOne(tower);
    delete tower;

}
void MainWindow::removedBullet(Bullet *bullet)
{
	Q_ASSERT(bullet);

	m_bulletList.removeOne(bullet);
	delete bullet;
}
void MainWindow::removedBullet_wandering(bullet_wandering *bullet){
    Q_ASSERT(bullet);

    m_bullet_wanderingList.removeOne(bullet);
    delete bullet;
}
void MainWindow::addBullet(Bullet *bullet)
{
	Q_ASSERT(bullet);

	m_bulletList.push_back(bullet);
}
void MainWindow::addwandering_bullet(bullet_wandering *bullet){
    Q_ASSERT(bullet);

    m_bullet_wanderingList.push_back(bullet);
}
void MainWindow::updateMap()//进行游戏
{
    if(!ispause){
	foreach (Enemy *enemy, m_enemyList)
		enemy->move();
	foreach (Tower *tower, m_towersList)
		tower->checkEnemyInRange();
    foreach (light_tower *tower, m_light_towersList)
        tower->checkEnemyInRange();
    update();
    }
}
void MainWindow::preLoadWavesInfo()
{
	QFile file(":/config/Waves.plist");
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		QMessageBox::warning(this, "TowerDefense", "Cannot Open TowersPosition.plist");
		return;
	}

	PListReader reader;
	reader.read(&file);

	// 获取波数信息
	m_wavesInfo = reader.data();

	file.close();
}
bool MainWindow::loadWave()//这个函数很关键，与plist有关
{
	if (m_waves >= m_wavesInfo.size())
		return false;

    WayPoint *startWayPoint = m_wayPointsList.back();//预处理的时候是从基地位置到入口位置存的，所以要获取最后一个位置
	QList<QVariant> curWavesInfo = m_wavesInfo[m_waves].toList();

	for (int i = 0; i < curWavesInfo.size(); ++i)
	{
		QMap<QString, QVariant> dict = curWavesInfo[i].toMap();

        int spawnTime = dict.value("spawnTime").toInt();
		Enemy *enemy = new Enemy(startWayPoint, this);
        int HealthPoint = dict.value("HP").toInt();
        enemy->setMaxHP(HealthPoint);
        qreal WalkingSpeed = dict.value("WalkingSpeed").toDouble();
        enemy->setm_walkingspeed(WalkingSpeed);
        QString temp = dict.value("address").toString();
        enemy->setQPixmap(temp);
        if(temp == ":/image/enemy.png")
            enemy->kind = 1;
        /*
        if(temp ==
            enemy->kind = 2;
        */
		m_enemyList.push_back(enemy);

		QTimer::singleShot(spawnTime, enemy, SLOT(doActivate()));
	}

	return true;
}
QList<Enemy *> MainWindow::enemyList() const
{
	return m_enemyList;
}
void MainWindow::gameStart()
{
	loadWave();
}
void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    foreach(Enemy *enemy, enemyList()){
        enemy->setm_walkingspeed(arg1/100);
    }
}
void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked==1)
    {
        foreach (Enemy*enemy, enemyList()) {
            enemy->invincible=1;
        }
    }
    else
    {
        foreach(Enemy*enemy,enemyList()){
            enemy->invincible=0;}
    }
}
void MainWindow::on_pushButton_clicked()
{
    foreach(Enemy *enemy , enemyList()){
        enemy->setHP(enemy->getmaxHP()*0.1);
    }
}
void MainWindow::on_checkBox_2_clicked(bool checked)
{
    if(checked==1){
        ispause=1;
    }
    else ispause=0;
}
