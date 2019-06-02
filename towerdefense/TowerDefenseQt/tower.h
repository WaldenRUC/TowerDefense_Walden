#ifndef TOWER_H
#define TOWER_H

#include <QPoint>
#include <QSize>
#include <QPixmap>
#include <QObject>
#include <tower_baseclass.h>

class QPainter;
class Enemy;
class MainWindow;
class QTimer;

class Tower : QObject, public tower_baseclass
{
	Q_OBJECT
public:
    Tower(QPoint pos, MainWindow *game, const QPixmap &sprite = QPixmap(":/image/tower.png"));
    ~Tower();
    int level;
	void draw(QPainter *painter) const;
	void checkEnemyInRange();
	void targetKilled();
	void attackEnemy();
	void chooseEnemyForAttack(Enemy *enemy);
	void removeBullet();
	void damageEnemy();
	void lostSightOfEnemy();
    void getRemoved();
    void originrange(){
        m_basic_range = 0;
    }
    int				m_damage;		// 代表攻击敌人时造成的伤害
    const QPoint getpos() {
        return this->m_pos;
    }
    void setrange(int range){
        this->m_basic_range+=range;
    }

    Enemy *			m_chooseEnemy;
    void setdamage(int damage){
        this->m_basic_damage += damage;
    }
    int				m_attackRange;	// 代表塔可以攻击到敌人的距离

private slots:
	void shootWeapon();
    //地狱塔专用槽
    void shootOTK();
    void shootOTKultimate();
    //地狱塔专用槽

private:
	bool			m_attacking;
	int				m_fireRate;		// 代表再次攻击敌人的时间间隔
	qreal			m_rotationSprite;
    int             m_basic_range;
    int             m_basic_damage;

	MainWindow *	m_game;
	QTimer *		m_fireRateTimer;

    QTimer *        m_lasttime;
    QTimer *        m_llasttime;

    QPoint          m_pos;
	const QPixmap	m_sprite;

	static const QSize ms_fixedSize;
};

#endif // TOWER_H
