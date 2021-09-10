#include "Actor.h"
#include "StudentWorld.h"

#include "GameConstants.h" //for road boundaries, sounds, etc. and randInt
#include <cmath> //for cos, sin

//***********************************************************************************************
//ABSTRACT BASE CLASS ACTOR IMPLEMENTATIONS:
//***********************************************************************************************
bool Actor::moveRelativeToRacer(){
    double vert_speed = getVertSpeed() - getWorld()->getRacerSpeed();
    double horiz_speed = getHorizSpeed();
    double new_y = (getY() + vert_speed);
    double new_x = (getX() + horiz_speed);
    moveTo(new_x, new_y);
    //if the border line has gone off the screen, it must be set to dead so studentworld can remove it
    if (hasExitedScreen()){
        killActor();
        return false;           //false return to indicate doSomething should not be continued, actor is dead from moving
    }
    return true;
}
void Actor::doSomething(){
    if (isAlive())
        actuallyDoSomething();
}
void Actor::hitByProjectile(const int& amt){
    if (!isAlive() || !isAffectedByProjectiles())                         //if dead or not affected by projectiles return. just an extra check... should never be called in this manner the way the world projectileMaybeDamageActor is setup
        return;
    else
        reactToProjectile(amt);
}
void Actor::reactToProjectile(const int& /* amt */){               //most actors instantly die if they are hit by a projectile and are affected by projectiles. MARK: VIRTUAL- enemies react differently and will redefine this method
    if (isAffectedByProjectiles())
        killActor();
}
bool Actor::isCollisionAvoidanceWorthy() const{             //all actors have true/false for this property
    return m_isCollisionAvoidanceWorthy;
}
bool Actor::isAffectedByProjectiles() const{                //all actors have true/false for this property
    return m_isAffectedByProjectiles;
}
double Actor::getVertSpeed() const{
    return m_vertSpeed;
}
double Actor::getHorizSpeed() const{
    return m_horizSpeed;
}
StudentWorld* Actor::getWorld() const{
    return m_world;
}
void Actor::setVertSpeed(const double& speed){
    m_vertSpeed = speed;
}
void Actor::setHorizSpeed(const double& speed){
    m_horizSpeed = speed;
}
bool Actor::hasExitedScreen(){
    if ( getX() < 0 || getX() > VIEW_WIDTH || getY() < 0 || getY() > VIEW_HEIGHT )  //MARK: Think it should be width-1 and height-1 but following spec
        return true;
    return false;
}
//***********************************************************************************************
//ABSTRACT BASE CLASS DAMAGEABLE ACTOR IMPLEMENTATIONS:
//***********************************************************************************************
void DamageableActor::getDamaged(const int& amt){
    if (!isAlive())                 //shouldn't be damaged if already dead
        return;
    m_hitPoints -= amt;             //otherwise decrement HP
    if (!isAlive())
        noHPLeft();                 //do unique dying stuff, virtual (empty by default, but redefined for zombie ped, cab, GRacer)
}
bool DamageableActor::isAlive() const{
    return (getHP() > 0);
}
//setter and getter for hitPoints added
void DamageableActor::setHP(const int& amt){
    m_hitPoints = amt;
}
int DamageableActor::getHP() const{
    return m_hitPoints;
}
void DamageableActor::killActor(){
    setHP(0);
}

//***********************************************************************************************
//ABSTRACT BASE CLASS ENEMY IMPLEMENTATIONS:
//***********************************************************************************************
void Enemy::reactToProjectile(const int& amt){
    if (isUndead())
        getDamaged(amt);        //enemies who are zombies will be damaged by holy water, when damaged they will further act a certain way automatically if killed
    if (isAlive())
        enemyWasHitByProjectile();          //they will do something else than the noHP method if they were hit but not killed. Human peds always just call this method
}

bool Enemy::isUndead() const{
    return m_undead;                //most enemies are zombies
}

void Enemy::actuallyDoSomething(){
    if ( messWithGhostRacer() ) //IF OVERLAPPING STUFF: enemies can 'mess' with the ghost racer if overlapping him
        return;                 //if the enemy was overlapping the ghost racer, it does not move or plan movements, instant return
    if (!moveRelativeToRacer()) //move relative to racer
        return;                 //stop if died by moving off screen
    planMovements();            //all enemies have movement plan stuff
}

int Enemy::getMovementPlanDistance() const{     //getter for movement plan distance (protected)
    return movementPlanDistance;
}
void Enemy::setMovementPlanDistance(const int& distance){       //setter for movement plan distance (protected)
    movementPlanDistance = distance;
}
void Enemy::makeNewPedMovementPlan(){
    int pedHorizontalSpeed = 0;                                         //otherwise come up with a new speed from -3 to 3 NOT including 0
    while (pedHorizontalSpeed == 0){
        pedHorizontalSpeed = randInt(-3, 3);
    }
    setHorizSpeed(static_cast<double>(pedHorizontalSpeed));                   //make a new movement plan with new speed
    setMovementPlanDistance(randInt(4,32));                                 //and stick with it from 4 to 32 steps
    if( getHorizSpeed() < 0 )                                           //re-orient the pedestrian to face in direction of movement
        setDirection(180);
    else
        setDirection(0);
}

//***********************************************************************************************
//ABSTRACT BASE CLASS ENVIRONMENT IMPLEMENTATIONS:
//***********************************************************************************************
bool Environment::isAlive() const{                               //the isAlive method now simply checks the on/off life status
    return m_alive;
}
void Environment::killActor(){                                   //killing an environment actor just involves setting its m_alive to false
    m_alive = false;
}

//***********************************************************************************************
//ABSTRACT BASE CLASS GOODIE IMPLEMENTATIONS:
//***********************************************************************************************
void Goodie::actuallyDoSomething(){
    if ( !moveRelativeToRacer() )
        return;                                 //return instantly if died going off screen
    if (getWorld()->overlapWithRacer(this) && getWorld()->racerIsAlive())     //Goodies actually interact with the racer if they run into her and she's alive (even though student world immediately returns when racer dies, just added safety)
        interactWithRacer();
    extraMovements();                           //Some Goodies do extra movements after potentially interacting with the racer
}

//***********************************************************************************************
//GHOSTRACER IMPLEMENTATIONS:
//***********************************************************************************************
void GhostRacer::actuallyDoSomething(){
    int keyVal(0);                                  //to hold user input for later
    if ( getX() <= ROAD_CENTER - ROAD_WIDTH/2 ){    //check if hitting left side of roadd
        if (getDirection() > 90){
            setHP(getHP() - 10);                    //HP automatically updates life status
            if (!isAlive()){
                noHPLeft();
            }
        }
        setDirection(82);
        getWorld()->playSound(SOUND_VEHICLE_CRASH);
    }
    else if ( getX() >= ROAD_CENTER + ROAD_WIDTH/2 ){       //check if hitting right side of road
        if ( getDirection() < 90){
            setHP(getHP() - 10);
            if (!isAlive()){
                noHPLeft();
            }
        }
        setDirection(98);
        getWorld()->playSound(SOUND_VEHICLE_CRASH);
    }
    else if ( getWorld()->getKey( keyVal) ){            //if the user pressed a key
        //shoot a holy water projectile and decrement ammo if pressed space
        if ( keyVal == KEY_PRESS_SPACE && m_holyWaterAmmo >= 1){
            double delta_x = SPRITE_HEIGHT * std::cos(getDirection() * 3.14159265359/180);
            double delta_y = SPRITE_HEIGHT * std::sin(getDirection() * 3.14159265359/180);
            Actor* newProjectile = new HolyWaterProjectile(getX() + delta_x, getY() + delta_y, getDirection(), getWorld());
            getWorld()->addActor(newProjectile);
            getWorld()->playSound(SOUND_PLAYER_SPRAY);
            m_holyWaterAmmo--;
        }
        //turn left if user pressed left and can turn more
        else if ( keyVal == KEY_PRESS_LEFT && getDirection() < 114 )
            setDirection(getDirection() + 8);
        //turn right if user pressed right and can turn more
        else if ( keyVal == KEY_PRESS_RIGHT && getDirection() > 66 )
            setDirection(getDirection() - 8);
        //speed up if user pressed up and can speed up more
        else if ( keyVal == KEY_PRESS_UP && getVertSpeed() < 5 )
            setVertSpeed(getVertSpeed() + 1);
        //slow down if user pressed down and can slow down more
        else if ( keyVal == KEY_PRESS_DOWN && getVertSpeed() > -1)
            setVertSpeed(getVertSpeed() - 1);
    }
    //GHOSTRIDER MOVEMENT ALGO:
    double max_shift_per_tick = 4.0;
    int direction = getDirection();
    double delta_x = std::cos(static_cast<double>(direction) * (3.14159265359 / 180)) * max_shift_per_tick;
    moveTo(getX() + delta_x, getY());
}

void GhostRacer::getSpun(){                     //ghost racer can be spun by oil slicks MARK: ASSUMPTION- WE STILL SPIN UP TO 120 OR DOWN to 60 IF OIL SLICK WANTING US TO SPIN EVEN MORE
    int spinLeftOrRight = randInt(0,1), newDirection = 0;
    switch(spinLeftOrRight)
    {
        case 0:     //RIGHT
        {
            newDirection = getDirection() - randInt(5,20);
            if ( newDirection < 60 )            //can't go less than 60
                setDirection(60);
            else
                setDirection(newDirection);
            break;
        }
        case 1:     //LEFT
        {
            newDirection = getDirection() + randInt(5,20);
            if ( newDirection > 120 )               //cant spin greater than 120
                setDirection(120);
            else
                setDirection(newDirection);
            break;
        }
    }
}

void GhostRacer::getHealed(const int& amt){             //GhostRacer can be healed by a certain amount
    int newHealth = getHP() + amt;
    if (newHealth > 100)
        setHP(100);
    else
        setHP(newHealth);
}
void GhostRacer::noHPLeft(){
    //status automatically set to not alive by isAlive methods return functionality
    getWorld()->playSound(SOUND_PLAYER_DIE);
}

void GhostRacer::giveAmmo(const int& amt){          //GhostRacer can gain ammo
    m_holyWaterAmmo += amt;
}

unsigned int GhostRacer::getAmmo() const{               //other objects and the world can see GhostRacer's ammo amount
    return m_holyWaterAmmo;
}

//***********************************************************************************************
//BORDER LINE IMPLEMENTATIONS:
//***********************************************************************************************
void BorderLine::actuallyDoSomething(){
    moveRelativeToRacer();
}

//***********************************************************************************************
//HUMAN PEDESTRIAN IMPLEMENTATIONS:
//***********************************************************************************************
bool HumanPedestrian::messWithGhostRacer(){
    if ( getWorld()->overlapWithRacer(this) && getWorld()->racerIsAlive()){                          //rider instantly loses a life if it runs into a human pedestrian. extra check for racer being alive (unnecessary but added safety)
        getWorld()->killRacer(); //world checks for death after every actor does something so it will notice and end level
        return true;
    }
    return false;
}

void HumanPedestrian::planMovements(){
    setMovementPlanDistance(getMovementPlanDistance() - 1);             //decrement movement plan distance
    
    if (getMovementPlanDistance() > 0)                                 //if still following a movement plan, done, stick w it
        return;
    
    makeNewPedMovementPlan();                                       //otherwise make a new pedestrian movement plan
}

void HumanPedestrian::enemyWasHitByProjectile(){            //run when hit by projectile and not dead (always the case for human peds being hit)
    setHorizSpeed(getHorizSpeed() * -1);      //when hit by holy water human ped reverses direction
    if( getHorizSpeed() < 0 )                          //re-orient the pedestrian to face in direction of movement
        setDirection(180);
    else
        setDirection(0);
    getWorld()->playSound(SOUND_PED_HURT);      //plays a sound that the ped was hurt
}

//***********************************************************************************************
//ZOMBIE PEDESTRIAN IMPLEMENTATIONS:
//***********************************************************************************************
bool ZombiePedestrian::messWithGhostRacer(){
    if ( getWorld()->overlapWithRacer(this) && getWorld()->racerIsAlive()){                      //if overlapping with the racer, damage him by 5, damage zombie by 2. extra check for safety that racer is alive
        getWorld()->damageRacer(5);
        getDamaged(2);
        return true;                    //immediately return, zombie is done messing with ghost racer cus its DEAD
    }
    int racer_x = getWorld()->getRacerX(), racer_y = getWorld()->getRacerY();               //otherwise...
    if ( (getX() >= racer_x - 30 && getX() <= racer_x + 30) && getY() > racer_y ){              //if less than or equal to 30 pixels left or right of racer and above racer
        setDirection(270);
        if ( getX() < racer_x )                 //move right if to left of racer
            setHorizSpeed(1);
        else if ( getX() > racer_x )            //move left if to right of racer
            setHorizSpeed(-1);
        else
            setHorizSpeed(0);
        m_ticksUntilGrunt--;                    //decrease # of ticks until next grunt
        if (m_ticksUntilGrunt <= 0){            //if ready to grunt
            getWorld()->playSound(SOUND_ZOMBIE_ATTACK);                 //make grunt sound and rest ticks until next grunt to 20
            m_ticksUntilGrunt = 20;
        }
    }
    return false;           //keep going with do something, still alive for now
}

void ZombiePedestrian::planMovements(){
    if (getMovementPlanDistance() > 0){                         //if we still have time on the old movement plan, decrememnt that time and return
        setMovementPlanDistance(getMovementPlanDistance()-1);
        return;
    }
    //otherwise, make a new movement plan
    makeNewPedMovementPlan();
}

void ZombiePedestrian::enemyWasHitByProjectile(){
    getWorld()->playSound(SOUND_PED_HURT);
}

void ZombiePedestrian::noHPLeft(){
    //auto-set to not alive by HP becoming 0, not needed here
    getWorld()->playSound(SOUND_PED_DIE);
    if ( !getWorld()->overlapWithRacer(this) ){     //if zombie was killed by projectile and not overlapping racer
        if ( randInt(0,4) == 0 ){           //1 in 5 chance
            Actor* newHealingGoodie = new HealingGoodie(getX(), getY(), getWorld());     //add a new healing goodie at zombies location
            getWorld()->addActor(newHealingGoodie);
        }
    }
    getWorld()->increaseScore(KILLED_ZPED_SCORE_BONUS);         //player gets 150 points
}

//***********************************************************************************************
//ZOMBIE CAB IMPLEMENTATIONS:
//***********************************************************************************************
bool ZombieCab::messWithGhostRacer(){
    if ( getWorld()->overlapWithRacer(this) && getWorld()->racerIsAlive() && !m_hasDamagedGhostRacer){                      //if overlapping with the racer and racer is alive and hasn't damaged racer yet (skip to movement via false return)
        getWorld()->playSound(SOUND_VEHICLE_CRASH);         //play crash and damage racer by 20 hp
        getWorld()->damageRacer(20);
        if (getX() <= getWorld()->getRacerX() ){            //if left of racer veer off road to left
            setHorizSpeed(-5);
            setDirection(120 + randInt(0,19));
        }
        else{                                               //if right of racer veer off road to right
            setHorizSpeed(5);
            setDirection(60 - randInt(0,19));
        }
        m_hasDamagedGhostRacer = true;                      //remember now that it has damaged ghost racer
    }
    return false;
}

void ZombieCab::planMovements(){
    double pixelsFront(VIEW_HEIGHT), pixelsBack(-VIEW_HEIGHT);
    getWorld()->closestCAWActorsInLane(this, pixelsFront, pixelsBack);             //check to see where the closest ALIVE CAW actors in front or behind the cab are. default would be VIEW_HEIGHT pixels in front if no actors in front and -VIEW_HEIGHT pixels behind if no actors behind. DOES NOT include Ghost Racer
    if ( getVertSpeed() > getWorld()->getRacerSpeed() && pixelsFront < 96){         //if non-GR CAW actor <96 pixels in front, slow down .5
        setVertSpeed(getVertSpeed() - 0.5);
        return;     //immediately return in this case
    }
    else if ( getVertSpeed() <= getWorld()->getRacerSpeed() && pixelsBack > -96 ){   //if non-GR CAW actor <96 pixels behind, speed up .5
        setVertSpeed(getVertSpeed() + 0.5);
        return;     //immediately return in this case
    }
    //otherwise...
    setMovementPlanDistance(getMovementPlanDistance()-1);           //decrement movement plan distance
    if (getMovementPlanDistance() > 0)
        return;     //stick with movement plan
    
    //otherwise, it's time for a new movement plan
    setMovementPlanDistance(randInt(4,32));
    setVertSpeed(getVertSpeed() + randInt(-2,2));
}

void ZombieCab::enemyWasHitByProjectile(){
    getWorld()->playSound(SOUND_VEHICLE_HURT);
}

void ZombieCab::noHPLeft(){
    //life status automatically updated by HP
    getWorld()->playSound(SOUND_VEHICLE_DIE);
    if ( randInt(0,4) == 0 ){           //1 in 5 chance
        Actor* newOilSlick = new OilSlick(getX(), getY(), getWorld());     //add a new oil slick at zombies location
        getWorld()->addActor(newOilSlick);
    }
    getWorld()->increaseScore(KILLED_CAB_SCORE_BONUS);
}

//***********************************************************************************************
//OIL SLICK IMPLEMENTATIONS:
//***********************************************************************************************
void OilSlick::interactWithRacer(){
    getWorld()->playSound(SOUND_OIL_SLICK);
    getWorld()->spinRacer();
}

//***********************************************************************************************
//HEALING GOODIE IMPLEMENTATIONS:
//***********************************************************************************************
void HealingGoodie::interactWithRacer(){
    getWorld()->healRacer(HEALGOODIE_HEALTH_BONUS);
    this->killActor();
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->increaseScore(HEALGOODIE_SCORE_BONUS);
}

//***********************************************************************************************
//HOLY WATER GOODIE IMPLEMENTATIONS:
//***********************************************************************************************
void HolyWaterGoodie::interactWithRacer(){
    getWorld()->giveRacerAmmo(HOLYWATER_AMMO_BONUS);
    this->killActor();
    getWorld()->playSound(SOUND_GOT_GOODIE);
    getWorld()->increaseScore(HOLYWATER_SCORE_BONUS);
}

//***********************************************************************************************
//SOUL GOODIE IMPLEMENTATIONS:
//***********************************************************************************************
void SoulGoodie::interactWithRacer(){
    getWorld()->saveASoul();
    this->killActor();
    getWorld()->playSound(SOUND_GOT_SOUL);
    getWorld()->increaseScore(SOULGOODIE_SCORE_BONUS);
}
void SoulGoodie::extraMovements(){
    setDirection(getDirection()-10);        //rotate by 10 degrees clockwise
}

//***********************************************************************************************
//HOLY WATER PROJECTILE IMPLEMENTATIONS:
//***********************************************************************************************
void HolyWaterProjectile::actuallyDoSomething()
{
    //if activated
    if ( getWorld()->projectileMaybeDamageActor(this)){
        killActor();            //projectile kills itself if it found an affected AND ALIVE actor overlapping it and attempted to damage the actor
        return;     //return immediately
    }
    //otherwise keep the projectile moving
    moveForward(SPRITE_HEIGHT);
    m_travelDistance -= SPRITE_HEIGHT;          //decrement its range/travel distance MARK: Assumed we should decrement by this amount to match the sample game range
    if (hasExitedScreen()){       //if it's gone off the screen, must be marked to be killed
        killActor();
        return;     //return immediately
    }
    if (m_travelDistance <= 0)
        killActor();                //mark projectile for death if it has now traveled 160 pixels
}

