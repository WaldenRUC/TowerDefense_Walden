#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "towerposition.h"
#include "tower.h"
#include "light_tower.h"
#include <QMenu>
#include <QIcon>
#include <QAction>
#include <QStyle>
#include <bullet_wandering.h>
namespace Ui {
class MainWindow;
}

class WayPoint;
class Enemy;
class Bullet;
class AudioPlayer;

class MainWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
    int m_current;//towerposition的位置
    int m_towercurrent;//tower的位置
	void getHpDamage(int damage = 1);
	void removedEnemy(Enemy *enemy);
    void removedBullet_wandering(bullet_wandering *bullet);
    void removedTower(Tower *tower);
    void removed_light_tower(light_tower *tower);

	void removedBullet(Bullet *bullet);
	void addBullet(Bullet *bullet);

    void addwandering_bullet(bullet_wandering *bullet);

	void awardGold(int gold);
    Enemy* targetenemy;//画出这个targetenemy的血量比例

	AudioPlayer* audioPlayer() const;
    QList<Enemy *> enemyList() const;//敌人列表
    QList<Tower *>			m_towersList;//塔的列表

    //
    Tower* temptower = NULL;
    light_tower *templighttower = NULL;
    //

    //
    QString str_tower;
    QString str_lighttower;
    Tower* tower_for_show = NULL;
    light_tower *light_tower_for_show = NULL;
    //

    bool ispause;
    QPoint temp;
    void setMenu();
    int stage;

        QList<Enemy *>              m_enemyList;
protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
	void updateMap();
	void gameStart();

    void setTower();
    void setLightTower();
    void upgradelevel();
    void upgradeRange();
    void upgradeDamage();
    void removethis();
    void showinfo();

    void on_doubleSpinBox_valueChanged(double arg1);
    void on_checkBox_clicked(bool checked);
    void on_pushButton_clicked();
    void on_checkBox_2_clicked(bool checked);

private:
	void loadTowerPositions();
	void addWayPoints();
	bool loadWave();
	bool canBuyTower() const;
	void drawWave(QPainter *painter);
    void drawcurrentHP(QPainter *painter);
	void drawHP(QPainter *painter);
    void drawinfo(QPainter *painter);

	void drawPlayerGold(QPainter *painter);
	void doGameOver();
	void preLoadWavesInfo();


private:
    Ui::MainWindow *            ui;
    int                         m_waves;
    int                         m_HPpercent_forenemy;
    int                         m_playerHp;
    int                         m_playrGold;
    bool                        m_gameEnded;
    bool                        m_gameWin;
    AudioPlayer *               m_audioPlayer;
    QList<QVariant>             m_wavesInfo;
    QList<TowerPosition>        m_towerPositionsList;//防御塔位置的坐标列表，要用auto it遍历
    QList<light_tower *>        m_light_towersList;
    QList<WayPoint *>           m_wayPointsList;


    QList<Bullet *>             m_bulletList;
    QList<bullet_wandering *>   m_bullet_wanderingList;

    QMenu* m_selectmenu;//点击则显示菜单
    QMenu* m_upgrademenu;//升级菜单

};
#endif // MAINWINDOW_H
