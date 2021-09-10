#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;

class Actor: public GraphObject
{
public:
    //Actors are initialized with an imageID, coords,
    //direction, size, depth, speeds, and linked world
    Actor(const int& imageID, const double& x, const double&y, const int& dir,
          const double& size, const int& depth, const double& vertSpeed,
          const double& horizSpeed, const bool& collisionAvoidanceWorthy, const bool& affectedByProjectiles, StudentWorld* myWorld)
    :GraphObject(imageID, x, y, dir, size, depth),
    m_isCollisionAvoidanceWorthy(collisionAvoidanceWorthy),
    m_isAffectedByProjectiles(affectedByProjectiles), m_world(myWorld),
    m_vertSpeed(vertSpeed), m_horizSpeed(horizSpeed)
    {}
    virtual ~Actor(){}
    
    //public getters
    bool isCollisionAvoidanceWorthy() const;
    bool isAffectedByProjectiles() const;
    double getVertSpeed() const;
    double getHorizSpeed() const;
    virtual bool isAlive() const = 0;                               //MARK: PURE VIRTUAL, redefined separately for Environment and DamageableActors
    
    //other public methods
    void doSomething();
    void hitByProjectile(const int& amt);
    virtual void killActor() = 0;                                   //MARK: PURE VIRTUAL, either set HP to zero or change bool member depending on specific class
    
protected:
    //protected getters
    StudentWorld* getWorld() const;
    
    //protected setters
    void setVertSpeed(const double& speed);
    void setHorizSpeed(const double& speed);
    
    //other protected methods:
    bool moveRelativeToRacer();                                     //all Actors other than GhostRacer will use this
    bool hasExitedScreen();                                         //checks if actors have gone off the screen
    
private:
    //basic state characteristics
    bool                m_isCollisionAvoidanceWorthy;               //some actors will be collision avoidance worthy
    bool                m_isAffectedByProjectiles;                  //some actors will be affected by projectiles
    StudentWorld*       m_world;                                    //link each object to its world
    double              m_vertSpeed;                                //each actor has a vertical speed
    double              m_horizSpeed;                               //and a horizontal speed
    
    virtual void reactToProjectile(const int& /* amt */);
    virtual void actuallyDoSomething() = 0;                         //MARK: PURE VIRTUAL
};

class Environment: public Actor{
public:
    //construction constants
    static constexpr bool   ENV_CAW = false; //MARK: no env. actors are CAW
    
    //MARK: no Environment Actors are collision avoidance worthy so false passed up to Actor
    Environment(const int& imageID, const double& x, const double&y, const int& dir, const double& size, const int& depth, const double& vertSpeed, const double& horizSpeed, const bool& affectedByProjectiles, StudentWorld* myWorld)
    :Actor(imageID, x, y, dir, size, depth, vertSpeed, horizSpeed, ENV_CAW, affectedByProjectiles, myWorld), m_alive(true)
    {}
    virtual ~Environment(){}
    
    virtual bool isAlive() const;                               //the isAlive method now simply checks the on/off life status
    virtual void killActor();                                   //killing an environment actor just involves setting its m_alive to false

private:
    bool m_alive;                                        //Environment actors now track their alive status as on/off
};

class DamageableActor: public Actor
{
public:
    //construction constants
    static constexpr bool   DAMAGEABLE_ACTOR_CAW = true;       //MARK: All damageable actors are collision avoidance worthy
    
    DamageableActor(const int& imageID, const double& x, const double&y, const int& dir, const double& size, const int& depth, const double& vertSpeed, const double& horizSpeed, const bool& affectedByProjectiles, StudentWorld* myWorld, const int& HP)
    :Actor(imageID, x, y, dir, size, depth, vertSpeed, horizSpeed, DAMAGEABLE_ACTOR_CAW, affectedByProjectiles, myWorld), m_hitPoints(HP)
    {};
    virtual ~DamageableActor(){}
    
    //getters
    virtual bool isAlive() const;                               //redefined to return whether HP is > 0
    int getHP() const;                                          //return actual HP
    
    //setters
    virtual void killActor();                                   //damageable actors are killed by setting hp to 0
    void getDamaged(const int& amt);
    
protected:
    void setHP(const int& amt);
    
private:
    int                 m_hitPoints;                            //DamageableActor's now havehitPoints
    //other method for when damage kills one of these damageable actors
    virtual void noHPLeft(){}                                   //MARK: Damageable actors can lose all their HP when damaged and act each their unique way or do nothing (hence empty base case). base case currently applies to human peds
};

//ABC Goodie
class Goodie: public Environment{
public:
    Goodie(const int& imageID, const double&x, const double&y, const int& dir, const double& size, const int& depth, const double& vertSpeed, const double& horizSpeed, const bool& affectedByProjectiles, StudentWorld* myWorld)
    :Environment(imageID, x, y, dir, size, depth, vertSpeed, horizSpeed, affectedByProjectiles, myWorld)
    {}
    virtual ~Goodie(){}
       
private:
    virtual void actuallyDoSomething();
    virtual void interactWithRacer() = 0;                       //MARK: PURE VIRTUAL, all goodies interact with racer in some way
    virtual void extraMovements(){}                             //MARK: BY DEFAULT GOODIES WILL NOT DO EXTRAMOVEMENTS UNLESS THEY CHOOSE TO REIMPLEMENT extraMovements()
};

class Enemy: public DamageableActor{
public:
    //construction constants
    static constexpr int ENEMY_START_PLAN_DISTANCE = 0; //MARK: all enemies start with movement plan distance of 0
    static constexpr bool    ENEMY_PROJECTILE_AFFECTED = true;  //MARK: All enemies are projectile affected
    
    Enemy(const int& imageID, const double& x, const double&y, const int& dir, const double& size, const int& depth, const double& vertSpeed, const double& horizSpeed, StudentWorld* myWorld, const int& HP, const bool& zombieOrNot)
    :DamageableActor(imageID, x, y, dir, size, depth, vertSpeed, horizSpeed, ENEMY_PROJECTILE_AFFECTED, myWorld, HP), movementPlanDistance(ENEMY_START_PLAN_DISTANCE), m_undead(zombieOrNot)
    {}
    virtual ~Enemy(){}
       
protected:
    //protected getters and setters for the movement plan distance:
    int getMovementPlanDistance() const;
    void setMovementPlanDistance(const int& distance);
    
    //other protected methods:
    void makeNewPedMovementPlan();                              //zombie and human peds both use this
    
private:
    int                 movementPlanDistance;                   //now have a movement plan distance
    bool                m_undead;                               //some enemies are undead zombies and some aren't
    
    //private methods
    
    bool isUndead() const;                                      //new getter to tell diff between zombies and TECHNICALLY innocent humans (who happen to be v annoying)
    virtual void reactToProjectile(const int& amt);             //enemies are all affected by projectiles in a similar way
    virtual void actuallyDoSomething();                         //Enemies all have a similar pattern of doing something, defined in cpp
    virtual void enemyWasHitByProjectile() = 0;                 //MARK: PURE VIRTUAL. All enemies must do something when hit but not killed by projectile
    virtual bool messWithGhostRacer() = 0;                      //return type of true means that enemy actually collided with ghost racer and messed with him MARK: PV
    virtual void planMovements() = 0;                           //all enemies plan movements at the end of their doSomething() if they make it there MARK: PV
};

//MARK:Fully Derived Class GHOSTRACER
class GhostRacer: public DamageableActor
{
public:
    //construction constants:
    static constexpr double RACER_START_X = 128;
    static constexpr double RACER_START_Y = 32;
    static constexpr int    RACER_START_DIR = 90;
    static constexpr double RACER_START_SIZE = 4.0;
    static constexpr int    RACER_DEPTH = 0;
    static constexpr double RACER_START_VERT_SPEED = 0;
    static constexpr double RACER_START_HORIZ_SPEED = 0;
    static constexpr bool   RACER_PROJECTILE_AFFECTED = false;
    static constexpr int    RACER_START_HP = 100;
    static constexpr int    RACER_START_AMMO = 10;
    //Construct the GhostRacer with IID_GHOST_RACER, starts alive (automatically via its 100 HP)
    // 90 degree start dir, x = 128, 7 =32, size 4.0, depth 0
    //coords (128,32), size 4.0, depth 0, vertSpeed and horizSpeed 0,
    //collision avoidance worthy (true), not affected by projectiles (false)
    //linked to the world passed in
    //starts with 10 holyWaterAmmo
    GhostRacer( StudentWorld* myWorld )
    :DamageableActor(IID_GHOST_RACER, RACER_START_X, RACER_START_Y, RACER_START_DIR, RACER_START_SIZE, RACER_DEPTH, RACER_START_VERT_SPEED, RACER_START_HORIZ_SPEED, RACER_PROJECTILE_AFFECTED, myWorld, RACER_START_HP), m_holyWaterAmmo(RACER_START_AMMO)
    {}
    virtual ~GhostRacer(){}
    
    //public extra GhostRacer getters:
    unsigned int getAmmo() const;                               //the world and other objects could see Ghost Racer's ammo amount
    
    //public extra GhostRacer setters:
    void getSpun();                                             //ghost racer can be spun by oil slicks
    void getHealed(const int& amt);                             //GhostRacer can be healed by a certain amount
    void giveAmmo(const int& amt);                              //GhostRacer can gain ammo
    
private:
    unsigned int            m_holyWaterAmmo;                    //GhostRacer now has ammo
    
    virtual void noHPLeft();                                    //GRacer dies its own way
    virtual void actuallyDoSomething();                         //GhostRacer will redefine its own actuallyDoSomething();
};

//MARK:Fully Derived Class HUMAN PEDESTRIAN
class HumanPedestrian: public Enemy{
public:
    //construction constants
    static constexpr int    HUMAN_START_DIR = 0;
    static constexpr double HUMAN_START_SIZE = 2.0;
    static constexpr int    HUMAN_DEPTH = 0;
    static constexpr double HUMAN_START_VERT_SPEED = -4;
    static constexpr double HUMAN_START_HORIZ_SPEED = 0;
    static constexpr int    HUMAN_START_HP = 2;
    static constexpr bool   HUMAN_UNDEAD = false;

    //Human Pedestrians are given x, y at instantiation.
    //They have IID_HUMAN_PED and They have dir 0, size 2.0,
    //depth 0, vertSpeed -4, horizSpeed 0, ARE collision avoidance worthy and ARE affected by projectiles
    //they have 2HP and are not undead
    //they have movement plan distance of 0 like all enemies
    //starts alive via the 2 HP
    HumanPedestrian(const double& x, const double&y, StudentWorld* myWorld)
    :Enemy(IID_HUMAN_PED, x, y, HUMAN_START_DIR, HUMAN_START_SIZE, HUMAN_DEPTH, HUMAN_START_VERT_SPEED, HUMAN_START_HORIZ_SPEED, myWorld, HUMAN_START_HP, HUMAN_UNDEAD)
    {}
    virtual ~HumanPedestrian(){}
    
private:
    virtual bool messWithGhostRacer();                      //redefined methods for general enemy pattern of do something
    virtual void planMovements();
    virtual void enemyWasHitByProjectile();                 //humans do their own thing when hit by HW projectile
};

//MARK:Fully Derived Class ZOMBIE PEDESTRIAN
class ZombiePedestrian: public Enemy{
public:
    //construction constants
    static constexpr int    ZPED_START_DIR = 0;
    static constexpr double ZPED_START_SIZE = 3.0;
    static constexpr int    ZPED_DEPTH = 0;
    static constexpr double ZPED_START_VERT_SPEED = -4;
    static constexpr double ZPED_START_HORIZ_SPEED = 0;
    static constexpr int    ZPED_START_HP = 2;
    static constexpr bool   ZPED_UNDEAD = true;
    static constexpr int    ZPED_TICKS2GRUNT = 0;
    //Zombie Pedestrians are given x, y at instantiation. They have dir 0, size 3.0,
    //depth 0, vertSpeed -4, horizSpeed 0, are collision avoidance worthy and affected by projectiles
    //they also have 2 HP (auto makes them start alive), and 0 ticks until they next grunt. They are undead
    //movement plan distance starts at 0 like all enemies
    ZombiePedestrian(const double& x, const double&y, StudentWorld* myWorld)
    :Enemy(IID_ZOMBIE_PED, x, y, ZPED_START_DIR, ZPED_START_SIZE, ZPED_DEPTH, ZPED_START_VERT_SPEED, ZPED_START_HORIZ_SPEED, myWorld, ZPED_START_HP, ZPED_UNDEAD),  m_ticksUntilGrunt(ZPED_TICKS2GRUNT)
    {}
    virtual ~ZombiePedestrian(){}
     
private:
    //score adder constant
    static constexpr int    KILLED_ZPED_SCORE_BONUS = 150;
    
    int m_ticksUntilGrunt;                                  //zombie ped now tracks time until its next grunt
    
    virtual bool messWithGhostRacer();                      //redefined methods for the general enemy pattern of do something
    virtual void planMovements();
    virtual void enemyWasHitByProjectile();                 //zombies get hurt by projectile their own way
    virtual void noHPLeft();                                //and die their own way
};

//MARK:Fully Derived Class ZOMBIE CAB
class ZombieCab: public Enemy{
public:
    //construction constants
    static constexpr int    ZCAB_START_DIR = 90;
    static constexpr double ZCAB_START_SIZE = 4.0;
    static constexpr int    ZCAB_DEPTH = 0;
    static constexpr double ZCAB_START_HORIZ_SPEED = 0;
    static constexpr int    ZCAB_START_HP = 3;
    static constexpr bool   ZCAB_UNDEAD = true;
    static constexpr bool   ZCAB_START_DAMAGED_RACER = false;
    //Zombie Cabs are given x, y, and vertical speed at instantiation (and tied to the world).
    //They all have IID_ZOMBIE CAB and They have dir 90, size 4.0,
    //depth 0, horizSpeed 0, are collision avoidance worthy and affected by projectiles
    //they also have 3 HP (thus are alive) and ARE undead.
    //they start out not yet having damaged ghost racer
    ZombieCab(const double& x, const double&y, const double& vertSpeed, StudentWorld* myWorld)
    :Enemy(IID_ZOMBIE_CAB, x, y, ZCAB_START_DIR, ZCAB_START_SIZE, ZCAB_DEPTH, vertSpeed, ZCAB_START_HORIZ_SPEED, myWorld, ZCAB_START_HP, ZCAB_UNDEAD),
    m_hasDamagedGhostRacer(ZCAB_START_DAMAGED_RACER)
    {}
    virtual ~ZombieCab(){}
 
private:
    //score adder constant
    static constexpr int    KILLED_CAB_SCORE_BONUS = 200;
    //private data
    bool m_hasDamagedGhostRacer;                            //zombie cabs now have a bool tracking whether they have damaged GRacer already
    
    //private methods
    virtual bool messWithGhostRacer();                      //redefined methods for the general enemy pattern of do something
    virtual void planMovements();
    virtual void enemyWasHitByProjectile();                 //zombie cabs do their own thing when hurt by HW projectile
    virtual void noHPLeft();                                //and when they die
};

//MARK: FULLY DERIVED CLASS HOLY WATER PROJECTILE
class HolyWaterProjectile: public Environment{
public:
    //construction constants
    static constexpr double PROJECTILE_START_SIZE = 1.0;
    static constexpr int    PROJECTILE_DEPTH = 1;
    static constexpr double PROJECTILE_START_VERT_SPEED = 0;
    static constexpr double PROJECTILE_START_HORIZ_SPEED = 0;
    static constexpr bool   PROJECTILE_PROJECTILE_AFFECTED = false;
    static constexpr int    PROJECTILE_START_TRAVEL_DISTANCE = 160;

    //projectiles must have x, y, dir, world specified
    //all projectiles have same image, size 1.0, depth 1, are not affected by projectiles, are not CAW (handled by environment), and they start with a max travel distance of 160 pixels. 0 was chosen for horizontal and vertical speed because projectiles do not use their horizontal and vertical speed. MARK: Assumed 0 for speeds because they aren't used for this object
    HolyWaterProjectile(const double&x, const double&y, const int& dir, StudentWorld* myWorld)
    :Environment(IID_HOLY_WATER_PROJECTILE, x, y, dir, PROJECTILE_START_SIZE, PROJECTILE_DEPTH, PROJECTILE_START_VERT_SPEED, PROJECTILE_START_HORIZ_SPEED, PROJECTILE_PROJECTILE_AFFECTED, myWorld), m_travelDistance(PROJECTILE_START_TRAVEL_DISTANCE)
    {}
    virtual ~HolyWaterProjectile(){}

private:
    int m_travelDistance;                                       //holy water projectiles now have a travel distance
    
    virtual void actuallyDoSomething();                         //Holy Water Projectiles have a unique actuallyDoSomething()
};

//MARK: FULLY DERIVED CLASS BORDER LINE
class BorderLine: public Environment{
public:
    //construction constants
    static constexpr int    BORDER_START_DIR = 0;
    static constexpr double BORDER_START_SIZE = 2.0;
    static constexpr int    BORDER_DEPTH = 2;
    static constexpr double BORDER_START_VERT_SPEED = -4;
    static constexpr double BORDER_START_HORIZ_SPEED = 0;
    static constexpr bool   BORDER_PROJECTILE_AFFECTED = false;
    //BorderLines have IID, x, y, and world specified at creation
    //ALWAYS start with dir 0, size 2.0, depth 2, vertSpeed -4, horizSpeed 0,
    // are not affected by projectiles (or collisions like all env.)
    //start alive (like all Env by default)
    BorderLine(const int& imageID, const double&x, const double&y, StudentWorld* myWorld )
    :Environment(imageID, x, y, BORDER_START_DIR, BORDER_START_SIZE, BORDER_DEPTH, BORDER_START_VERT_SPEED, BORDER_START_HORIZ_SPEED, BORDER_PROJECTILE_AFFECTED, myWorld)
    {}
    virtual ~BorderLine(){}

private:
    virtual void actuallyDoSomething();                         //BorderLine has its own actuallyDoSomething() which is the most simple
};

//MARK: FULLY DERIVED CLASS OIL SLICK - technically Bad but works just like other Goodies
class OilSlick: public Goodie{
public:
    //construction constants
    static constexpr int    OIL_START_DIR = 0;
    static constexpr int    OIL_DEPTH = 2;
    static constexpr double OIL_START_VERT_SPEED = -4;
    static constexpr double OIL_START_HORIZ_SPEED = 0;
    static constexpr bool   OIL_PROJECTILE_AFFECTED = false;
    //Oil Slicks must be specified with coords and world
    //they all have same image, 0 dir, a random int size b/t 2 and 5, depth 2, vert speed -4, horiz speed 0, are not affected by projectiles (or CAW like all env.)
    OilSlick(const double& x, const double& y, StudentWorld* myWorld)
    :Goodie(IID_OIL_SLICK, x, y, OIL_START_DIR, randInt(2,5), OIL_DEPTH, OIL_START_VERT_SPEED, OIL_START_HORIZ_SPEED, OIL_PROJECTILE_AFFECTED, myWorld)
    {}
    virtual ~OilSlick(){}
    
private:
    virtual void interactWithRacer();                           //oil slick will interact with racer by making her spin etc.
};

//MARK: FULLY DERIVED CLASS HEALING GOODIE
class HealingGoodie: public Goodie{
public:
    //construction constants
    static constexpr int    HEALGOODIE_START_DIR = 0;
    static constexpr double HEALGOODIE_START_SIZE = 1.0;
    static constexpr int    HEALGOODIE_DEPTH = 2;
    static constexpr double HEALGOODIE_START_VERT_SPEED = -4;
    static constexpr double HEALGOODIE_START_HORIZ_SPEED = 0;
    static constexpr bool   HEALGOODIE_PROJECTILE_AFFECTED = true;
    //healing goodies must be specified with coords and world
    //they all have same image, 0 dir, size 1.0, depth 2, vert speed -4, horiz speed 0, ARE affected by projectiles, not CAW (taken care of by Environment)
    HealingGoodie(const double& x, const double& y, StudentWorld * myWorld)
    :Goodie(IID_HEAL_GOODIE, x, y, HEALGOODIE_START_DIR, HEALGOODIE_START_SIZE, HEALGOODIE_DEPTH, HEALGOODIE_START_VERT_SPEED, HEALGOODIE_START_HORIZ_SPEED, HEALGOODIE_PROJECTILE_AFFECTED, myWorld)
    {}
    virtual ~HealingGoodie(){}
    
private:
    //score and health bonus constants
    static constexpr int    HEALGOODIE_HEALTH_BONUS = 10;
    static constexpr int    HEALGOODIE_SCORE_BONUS = 250;
    
    virtual void interactWithRacer();                           //healing goodies will interact with racer by giving her health etc.
};

//MARK: FULLY DERIVED CLASS HOLY WATER GOODIE
class HolyWaterGoodie: public Goodie{
public:
    //construction constants
    static constexpr int    HOLYWATERGOODIE_START_DIR = 90;
    static constexpr double HOLYWATERGOODIE_START_SIZE = 2.0;
    static constexpr int    HOLYWATERGOODIE_DEPTH = 2;
    static constexpr double HOLYWATERGOODIE_START_VERT_SPEED = -4;
    static constexpr double HOLYWATERGOODIE_START_HORIZ_SPEED = 0;
    static constexpr bool   HOLYWATERGOODIE_PROJECTILE_AFFECTED = true;
    //holy water goodies must be specified with coords and world
    //they all have same image, 0 dir, size 2.0, depth 2, vert speed -4, horiz speed 0, ARE affected by projectiles, not CAW (taken care of by Environment)
    HolyWaterGoodie(const double&x, const double&y, StudentWorld* myWorld)
    :Goodie(IID_HOLY_WATER_GOODIE, x, y, HOLYWATERGOODIE_START_DIR, HOLYWATERGOODIE_START_SIZE, HOLYWATERGOODIE_DEPTH, HOLYWATERGOODIE_START_VERT_SPEED, HOLYWATERGOODIE_START_HORIZ_SPEED, HOLYWATERGOODIE_PROJECTILE_AFFECTED, myWorld)
    {}
    virtual ~HolyWaterGoodie(){}
    
private:
    //score and ammo adder constants
    static constexpr int    HOLYWATER_AMMO_BONUS = 10;
    static constexpr int    HOLYWATER_SCORE_BONUS = 50;
    
    virtual void interactWithRacer();                           //holy water goodies will interact with racer by giving her ammo etc.
};

//MARK: FULLY DERIVED CLASS SOUL GOODIE
class SoulGoodie: public Goodie{
public:
    //construction constants
    static constexpr int    SOULGOODIE_START_DIR = 0;
    static constexpr double SOULGOODIE_START_SIZE = 4.0;
    static constexpr int    SOULGOODIE_DEPTH = 2;
    static constexpr double SOULGOODIE_START_VERT_SPEED = -4;
    static constexpr double SOULGOODIE_START_HORIZ_SPEED = 0;
    static constexpr bool   SOULGOODIE_PROJECTILE_AFFECTED = false;
    //soul goodies must be specified with coords and world
    //they all have same image, dir 0, size 4.0, depth 2, vert spped -4, hor speed 0, are not affected by projectiles, not CAW (taken care of by Environment)
    SoulGoodie(const double&x, const double&y, StudentWorld* myWorld)
    :Goodie(IID_SOUL_GOODIE, x, y, SOULGOODIE_START_DIR, SOULGOODIE_START_SIZE, SOULGOODIE_DEPTH, SOULGOODIE_START_VERT_SPEED, SOULGOODIE_START_HORIZ_SPEED, SOULGOODIE_PROJECTILE_AFFECTED, myWorld)
    {}
    virtual ~SoulGoodie(){}
    
private:
    //score adder constant
    static constexpr int    SOULGOODIE_SCORE_BONUS = 100;
    
    virtual void interactWithRacer();                           //Soul Goodies interactWithRacer by decreasing the world's souls to save etc.
    virtual void extraMovements();                              //Soul Goodie has extraMovements that it performs at the end of its dosomething (spinning)
};

#endif // ACTOR_H_
