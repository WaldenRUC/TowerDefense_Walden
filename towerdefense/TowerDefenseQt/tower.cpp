#include "tower.h"
#include "enemy.h"
#include "bullet.h"
#include "mainwindow.h"
#include "utility.h"
#include <QPainter>
#include <QColor>
#include <QTimer>
#include <QVector2D>
#include <QtMath>
#include "bullet_wandering.h"

const QSize Tower::ms_fixedSize(66, 66);//原来42，42
static int RANGE = 120;
static int DAMAGE = 5;//原：5
static int FIRERATE = 5;//原：5

Tower::Tower(QPoint pos, MainWindow *game, const QPixmap &sprite/* = QPixmap(":/image/tower.png"*/)
    : m_attacking(false)
    , level(1)
    , m_attackRange(RANGE)//原先70
    , m_damage(DAMAGE)//原先10
    , m_fireRate(FIRERATE)//原先1000
	, m_rotationSprite(0.0)
	, m_chooseEnemy(NULL)
	, m_game(game)
	, m_pos(pos)
	, m_sprite(sprite)
{
	m_fireRateTimer = new QTimer(this);
	connect(m_fireRateTimer, SIGNAL(timeout()), this, SLOT(shootWeapon()));
    //m_fireRateTimer释放出timeout()信号，执行Tower的shootWeapon()函数槽
    //含义：射击的冷却时间到了则有timeout()信号，执行射击shootWeapon()

    //地狱塔专用时间槽
    m_lasttime = new QTimer(this);
    m_llasttime = new QTimer(this);
    connect(m_lasttime, SIGNAL(timeout()), this, SLOT(shootOTK()));
    connect(m_llasttime, SIGNAL(timeout()), this, SLOT(shootOTKultimate()));

    //地狱塔专用时间槽
}

Tower::~Tower()
{
	delete m_fireRateTimer;
	m_fireRateTimer = NULL;
    //地狱塔
    delete m_lasttime;
    m_lasttime = NULL;
    delete m_llasttime;
    m_llasttime = NULL;
    //地狱塔
}

void Tower::checkEnemyInRange()
{

    if (m_chooseEnemy && /*m_chooseEnemy->invincible==0 &&*/ !m_game->enemyList().empty() && m_game->enemyList().indexOf(m_chooseEnemy)!=-1)//补充 empty
    {
		// 这种情况下,需要旋转炮台对准敌人
		// 向量标准化
        QVector2D normalized(m_chooseEnemy->pos() - m_pos);
        normalized.normalize();
        m_rotationSprite = qRadiansToDegrees(qAtan2(normalized.y(), normalized.x())) - 90;

		// 如果敌人脱离攻击范围
        if (!collisionWithCircle(m_pos, m_attackRange, m_chooseEnemy->pos(), 1)){
            lostSightOfEnemy();
        }
    }
    //如果已经选择了敌人，还要检验一下这个敌人是不是还在enemylist里面!!BUG改了两天了！！
    else if(!m_game->enemyList().empty())//补充：empty
    {
		// 遍历敌人,看是否有敌人在攻击范围内
        m_attackRange=RANGE;
        m_rotationSprite=0;
        m_damage=DAMAGE;
        QList<Enemy *> enemyList = m_game->enemyList();
        foreach (Enemy *enemy, enemyList)
        {
            if (/*enemy->invincible==0 &&*/ collisionWithCircle(m_pos, m_attackRange, enemy->pos(), 1) && enemy)
            {
                chooseEnemyForAttack(enemy);
                break;
                //不加break的时候，优先攻击最后面的敌人。是否说明游戏运行过程中chooseEnemyForAttack是一个进攻优先队列？在攻击第一个敌人时，后面的敌人仍然被选择为这个函数的目标，因此当
                //失去第一个敌人时，优先打最后一个（最迟入队的）敌人？
            }
       }
    }

}

void Tower::draw(QPainter *painter) const
{
	painter->save();
	painter->setPen(Qt::white);
	// 绘制攻击范围
	painter->drawEllipse(m_pos, m_attackRange, m_attackRange);

	// 绘制偏转坐标,由中心+偏移=左上
	static const QPoint offsetPoint(-ms_fixedSize.width() / 2, -ms_fixedSize.height() / 2);

	// 绘制炮塔并选择炮塔
	painter->translate(m_pos);
    QPoint levelpoint(-15,-40);
    painter->setPen(Qt::black);
    painter->drawText(levelpoint,QString("Lv. %1").arg(level));
	painter->rotate(m_rotationSprite);
	painter->drawPixmap(offsetPoint, m_sprite);

	painter->restore();
}

void Tower::attackEnemy()
{
    m_fireRateTimer->start(m_fireRate);//延迟m_fireRate时间后开始射击
    //m_fireRateTimer->start(500);
    //m_fireRate=1;
    m_lasttime->start(300);//地狱塔：0.5s之后开启秒杀模式
    m_llasttime->start(1000);////
}

void Tower::chooseEnemyForAttack(Enemy *enemy)
{
    if(!m_game->enemyList().empty() && enemy!=NULL && m_game->enemyList().indexOf(enemy)!=-1){
    m_chooseEnemy = enemy;
    QList<Enemy *> enemyList = m_game->enemyList();
    /*foreach (Enemy *aenemy, enemyList)
    {
        if(collisionWithCircle(enemy->pos(), 40, aenemy->pos(), 40))
        {
            if(aenemy)
            aenemy->getDamage(1000);
        }
    }*/
    //这里的溅射伤害仅仅会在选定目标时造成,这里注释掉，因为最好要在子弹打中的时候再写这一段
	attackEnemy();
    m_chooseEnemy->getAttacked_by_tower(this);}
}

void Tower::shootWeapon()
{
    if(this->m_chooseEnemy && !m_game->enemyList().empty() && m_game->enemyList().indexOf(m_chooseEnemy)!=-1){//补充empty
	Bullet *bullet = new Bullet(m_pos, m_chooseEnemy->pos(), m_damage, m_chooseEnemy, m_game);
    bullet_wandering *wander1 = new bullet_wandering(m_pos, m_damage, m_game);
    bullet_wandering *wander2 = new bullet_wandering(m_pos, m_damage, m_game);
    bullet_wandering *wander3 = new bullet_wandering(m_pos, m_damage, m_game);
    bullet_wandering *wander4 = new bullet_wandering(m_pos, m_damage, m_game);
    qreal temp = qAtan2(m_chooseEnemy->pos().y()-m_pos.y(),m_chooseEnemy->pos().x()-m_pos.x());
    wander1->angle = temp - 3.14/8.0;
    wander2->angle = temp + 3.14/8.0;
    wander3->angle = temp + 3.14/6.0;
    wander4->angle = temp - 3.14/6.0;
    wander1->move();
    wander2->move();
    wander3->move();
    wander4->move();
    bullet->move();
    m_game->addBullet(bullet);
    m_game->addwandering_bullet(wander1);
    m_game->addwandering_bullet(wander2);
    m_game->addwandering_bullet(wander3);
    m_game->addwandering_bullet(wander4);
    }
}
//地狱塔专用
void Tower::shootOTK()
{
    if(m_chooseEnemy){
    m_damage += 2;//秒杀！
    m_attackRange+=5;//原10
    }
    else 　{
        m_damage=DAMAGE;
        m_attackRange=RANGE;
    }
}
void Tower::shootOTKultimate()
{
    if(m_chooseEnemy)m_damage += 3;
    else{
        m_damage=DAMAGE;m_attackRange=RANGE;
    }
   // m_llasttime->stop();
}
//地狱塔专用
void Tower::targetKilled()
{
    m_chooseEnemy->gotLostSight_by_tower(this);
    if (m_chooseEnemy)
		m_chooseEnemy = NULL;
    //地狱塔
    m_lasttime->stop();
    m_llasttime->stop();
    m_damage=DAMAGE;
    m_attackRange=RANGE;
    //m_fireRate=1;
    //地狱塔
    m_fireRateTimer->stop();

    m_rotationSprite = 0.0;//将炮塔旋转回原先的位置
}

void Tower::lostSightOfEnemy()
{
    m_chooseEnemy->gotLostSight_by_tower(this);
    if (m_chooseEnemy)
		m_chooseEnemy = NULL;
    //仅限地狱塔
    m_lasttime->stop();
    m_llasttime->stop();
    m_damage=DAMAGE;
    m_attackRange=RANGE;
    //m_fireRate=1;
    //仅限地狱塔
    m_fireRateTimer->stop();
    m_rotationSprite = 0.0;//将炮塔旋转回原先的位置
}

void Tower::getRemoved()
{
    if(!m_chooseEnemy)return;
    foreach (Enemy *enemy, m_game->enemyList())
    {
        enemy->gotLostSight_by_tower(this);//如果enemy的attacked列表里没有这个塔呢？
    }
    m_game->removedTower(this);

}
