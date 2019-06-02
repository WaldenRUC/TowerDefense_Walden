#ifndef ENEMY_H
#define ENEMY_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QPixmap>
#include <QTimer>
class WayPoint;
class QPainter;
class MainWindow;
class Tower;
class light_tower;

class Enemy : public QObject
{
	Q_OBJECT
public:
	Enemy(WayPoint *startWayPoint, MainWindow *game, const QPixmap &sprite = QPixmap(":/image/enemy.png"));
    ~Enemy();
	void draw(QPainter *painter) const;
	void move();
	void getDamage(int damage);
    void setMaxHP(int healthpoint){
        m_maxHp=healthpoint;
        m_currentHp = m_maxHp;
    }
    void getRemoved();
    void getRemoved_light_tower();

    void getAttacked_by_tower(Tower *attacker);
    void getAttacked_by_light_tower(light_tower *attacker);

    void gotLostSight_by_tower(Tower *attacker);
    void gotLostSight_by_light_tower(light_tower *attacker);
    int kind;
    void setm_walkingspeed(qreal p){m_walkingSpeed = p;}
    void setQPixmap(QString A){
        QPixmap temp =QPixmap(A);
        m_sprite = temp;
    }
    double slow;//减速比例
    bool invincible;//如果invincible,那么不能瞄准

    QList<light_tower *>    m_attackedlight_towersList;//光塔名单
    QList<Tower *>          m_attackedTowersList;//地狱塔名单

	QPoint pos() const;



    int m_attackRange;

    int getcurrentHp(){
        return m_currentHp;
    }
    int getmaxHP(){
        return m_maxHp;
    }
    void setmaxcurrentHP(){
        m_currentHp = m_maxHp;
    }
    void setHP(int temphp){
        m_currentHp+=temphp;
        if(m_currentHp>=m_maxHp)m_currentHp=m_maxHp;
    }
public slots:
	void doActivate();
private slots:
    void changespeed();//加速
    void back_to_origin();//回原速
private:
    double                  times;//加速倍数
    double                  stop;
    bool                	m_active;
    int                 	m_maxHp;
    int                 	m_currentHp;
    qreal               	m_walkingSpeed;
    qreal               	m_rotationSprite;

    QPoint              	m_pos;
    WayPoint *          	m_destinationWayPoint;
    MainWindow *            m_game;



    QTimer *                m_accelerate;//敌人加速
    QTimer *                m_back_to_origin;//敌人回原速

    QPixmap           m_sprite;//敌人的图片
    static const QSize      ms_fixedSize;
};

#endif // ENEMY_H
