#ifndef LIGHT_TOWER_H
#define LIGHT_TOWER_H

#include <QPoint>
#include <QSize>
#include <QPixmap>
#include <QObject>
#include <tower_baseclass.h>

class QPainter;
class Enemy;
class MainWindow;
class QTimer;

class light_tower:QObject, public tower_baseclass
{
    Q_OBJECT
public:
    light_tower(QPoint pos, MainWindow *game, const QPixmap &sprite = QPixmap(":/image/light_tower.png"));
    ~light_tower();
    int level;
    void draw(QPainter *painter) const;

    void checkEnemyInRange();
    void targetKilled();
    void attackEnemy();
    void damageEnemy();
    int				m_damage;		// 代表攻击敌人时造成的伤害
    void lostSightOfEnemy();
    void setdamage(int damage){
        this->m_damage += damage;
    }
    int             x;
    int             y;
    int             z;
    int             posx(){
        return this->m_pos.x();
    }
    int             posy(){
        return this->m_pos.y();
    }
    void getRemoved();
    double tempx;
    double tempy;

    int getrange(){
        return m_attackRange;
    }

    int attacknumber;

    const QPoint getpos(){
        return this->m_pos;
    }
    void originrange(){
        m_attackRange = 0;
    }
    void setrange(int range){
        this->m_attackRange+=range;
    }

private slots:
    void chooseEnemyForAttack();
    void changecolor();
private:
    bool			m_attacking;
    int				m_attackRange;	// 代表塔可以攻击到敌人的距离
    int				m_fireRate;		// 代表再次攻击敌人的时间间隔

    int             m_basic_damage;
    int             m_basic_range;

    qreal			m_rotationSprite;
    Enemy *			m_chooseEnemy;
    MainWindow *	m_game;
    QTimer *		m_fireRateTimer;
    //光塔专用时间槽
    QTimer *        m_lightTowerTimer;
    QTimer *        m_lightlazer;
    QTimer *        changethecolor;
    //光塔专用时间槽
    const QPoint    m_pos;
    const QPixmap	m_sprite;

    static const QSize ms_fixedSize;
};

#endif // LIGHT_TOWER_H
