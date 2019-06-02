#include "light_tower.h"
#include "enemy.h"
#include "bullet.h"
#include "mainwindow.h"
#include "utility.h"
#include <QPainter>
#include <QColor>
#include <QTimer>
#include <QVector2D>
#include <QtMath>


const QSize light_tower::ms_fixedSize(100, 100);//原来42，42;66,66
static int RANGE = 250;
static int DAMAGE = 100;
static int FIRERATE = 200;
static int ATTACKMAX = 5;
light_tower::light_tower(QPoint pos, MainWindow *game, const QPixmap &sprite/* = QPixmap(":/image/light_tower.jpg"*/)
    : m_attacking(false)
    , m_attackRange(RANGE)//原先70
    , m_damage(DAMAGE)//原先10
    , m_basic_damage(DAMAGE)
    , m_basic_range(RANGE)
    , level(1)
    , x(0)
    , y(0)
    , z(0)
    , m_fireRate(FIRERATE)//原先1000
    , m_rotationSprite(0.0)
    , m_chooseEnemy(NULL)
    , m_game(game)
    , m_pos(pos)
    , m_sprite(sprite)
    , attacknumber(0)//攻击的数量
{
    //m_fireRateTimer释放出timeout()信号，执行Tower的shootWeapon()函数槽
    //含义：射击的冷却时间到了则有timeout()信号，执行射击shootWeapon()
    m_lightTowerTimer = new QTimer(this);
    connect(m_lightTowerTimer, SIGNAL(timeout()), this, SLOT(chooseEnemyForAttack()));
    m_lightTowerTimer->start(FIRERATE);
    //光塔专用时间槽
    changethecolor = new QTimer(this);
    connect(changethecolor, SIGNAL(timeout()), this, SLOT(changecolor()));
}

light_tower::~light_tower(){
    delete m_fireRateTimer;
    m_fireRateTimer = NULL;

    delete m_lightTowerTimer;
    m_lightTowerTimer = NULL;

    delete changethecolor;
    changethecolor = NULL;
}
void light_tower::checkEnemyInRange()
{}

void light_tower::draw(QPainter *painter) const
{
    painter->save();
    painter->setPen(Qt::white);
    painter->drawEllipse(m_pos, m_attackRange, m_attackRange);
    static const QPoint offsetPoint(-ms_fixedSize.width() / 2, -ms_fixedSize.height() / 2);
    painter->translate(m_pos);
    QPoint levelpoint(-15,-40);
    painter->setPen(Qt::black);
    painter->drawText(levelpoint,QString("Lv. %1").arg(level));

    painter->drawPixmap(offsetPoint, m_sprite);
    painter->restore();
}

void light_tower::attackEnemy()
{
    m_fireRateTimer->start(m_fireRate);//延迟m_fireRate时间后开始射击
}

void light_tower::changecolor()
{
    x=0;y=0;z=0;
}

void light_tower::chooseEnemyForAttack(/*Enemy *enemy*/)
{
    foreach (Enemy *enemy, m_game->enemyList())
    {
        if(collisionWithCircle(m_pos, m_attackRange, enemy->pos(),1) && enemy->getcurrentHp()>0 /*&& attacknumber<ATTACKMAX*/)
        {
            attacknumber++;
            enemy->getAttacked_by_light_tower(this);
            x=-255;y=-95;z=-25;
            changethecolor->start(20);
            if(enemy->invincible==0)enemy->getDamage(m_damage + enemy->getcurrentHp() / 100);
        }
        else if(!collisionWithCircle(m_pos, m_attackRange, enemy->pos(), 1))
        {
            enemy->gotLostSight_by_light_tower(this);
        }
        if(enemy->getcurrentHp()<=0 && m_game->enemyList().indexOf(enemy)!=-1)
        {
            m_game->awardGold(200);
            m_game->removedEnemy(enemy);
        }
    }
}

void light_tower::targetKilled()
{
    attacknumber--;
    m_chooseEnemy->gotLostSight_by_light_tower(this);
    if (m_chooseEnemy)
        m_chooseEnemy = NULL;
    m_fireRateTimer->stop();

    m_rotationSprite = 0.0;//将炮塔旋转回原先的位置
}

void light_tower::lostSightOfEnemy()
{
    attacknumber--;
    m_chooseEnemy->gotLostSight_by_light_tower(this);
    if (m_chooseEnemy)
        m_chooseEnemy = NULL;

    m_fireRateTimer->stop();
    m_rotationSprite = 0.0;//将炮塔旋转回原先的位置
}

void light_tower::getRemoved()
{
    foreach (Enemy *enemy, m_game->enemyList())
    {
        if(enemy->m_attackedlight_towersList.indexOf(this)!=-1)
        enemy->gotLostSight_by_light_tower(this);//如果enemy的attacked列表里没有这个塔呢？
    }
    m_game->removed_light_tower(this);
}

