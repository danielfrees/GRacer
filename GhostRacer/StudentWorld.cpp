#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>

#include "Actor.h"              //for Actor operations
#include <cmath>                //for sin and cos operations etc.
#include <vector>               //for holding actors in one container and using vector operations
#include <unordered_set>        //for cab generation randomizing (random lane choice)
#include <iostream>             //for << operator
#include <sstream>              //for std::ostringstream
#include <algorithm>            //for std::min and std::max

using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    m_racer = nullptr;                  //initialize racer pointer to nullptr so that we don't have bad access if exiting game w/o starting
    m_souls2Save = 0;                   //initialize integer/double member values that will be set up properly at each init()
    m_bonusPoints = 0;
    m_highestWhiteBorder = 0;
}

int StudentWorld::init()
{
    //initialize basic data structures to keep track of level
    m_souls2Save = 2 * getLevel() + 5;
    m_bonusPoints = 5000;
    
    //insert ghost racer into the world
    m_racer = new GhostRacer( this );
    
    //initialize road lines:
    
    //initialize left yellow borders
    for (int N = 0; N < VIEW_HEIGHT/SPRITE_HEIGHT; N++){
        Actor* newYellowBorder = new BorderLine(IID_YELLOW_BORDER_LINE, LEFT_EDGE, N * SPRITE_HEIGHT, this);
        m_actors.push_back(newYellowBorder);
    }
    //right yellow borders
    for (int N = 0; N < VIEW_HEIGHT/SPRITE_HEIGHT; N++){
        Actor* newYellowBorder = new BorderLine(IID_YELLOW_BORDER_LINE, RIGHT_EDGE, N * SPRITE_HEIGHT, this);
        m_actors.push_back(newYellowBorder);
    }
    //left white borders
    for (int M = 0; M < VIEW_HEIGHT/ (4*SPRITE_HEIGHT); M++){
        Actor* newWhiteBorder = new BorderLine(IID_WHITE_BORDER_LINE, LEFT_EDGE + ROAD_WIDTH/3, M * 4*SPRITE_HEIGHT, this);
        m_actors.push_back(newWhiteBorder);
        m_highestWhiteBorder = newWhiteBorder->getY();
    }
    //right white borders
    for (int M = 0; M < VIEW_HEIGHT/ (4*SPRITE_HEIGHT); M++){
        Actor* newWhiteBorder = new BorderLine(IID_WHITE_BORDER_LINE, RIGHT_EDGE - ROAD_WIDTH/3, M * 4*SPRITE_HEIGHT, this);
        m_actors.push_back(newWhiteBorder);
        m_highestWhiteBorder = newWhiteBorder->getY();
    }
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    //MARK:GIVE EACH ACTOR A CHANCE TO DO SOMETHING
    if ( m_racer->isAlive() ){          //Ghost Racer gets a chance to do something
        m_racer->doSomething();
        if ( !m_racer->isAlive() ){
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
    }
    //rest of the actors
    for (int i = 0; i < m_actors.size(); i++ ){
        if ( m_actors[i]->isAlive() )
            m_actors[i]->doSomething();
        if ( !m_racer-> isAlive() ){
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
    }
    //check if done with level
    if (m_souls2Save <= 0){
        increaseScore(m_bonusPoints);
        playSound(SOUND_FINISHED_LEVEL);        //must play this sound when level finished
        return GWSTATUS_FINISHED_LEVEL;
    }
    
    //keep track of highest white border
    m_highestWhiteBorder -= (4 + getRacerSpeed());  //border lines move at speed -4 - the racer's speed
    
    //REMOVE DEAD OBJECTS
    for (int i = 0; i < m_actors.size(); ){
        if ( !m_actors[i]->isAlive() ){
            vector<Actor*>::iterator deleterIt = m_actors.begin() + i;
            delete *deleterIt;
            m_actors.erase(deleterIt);
        }
        else
            i++;
    }
    
    //MARK:POTENTIALLY ADD NEW ACTORS:
    //add borderlines
    addBorderLines();
    //add zombie cabs
    addZombieCabs();
    //add oil slicks
    addOilSlicks();
    //add zombie peds
    addZombiePeds();
    //add human peds
    addHumanPeds();
    //add holy water goodies
    addHolyWaterGoodies();
    //add lost soul goodies
    addSoulGoodies();
    
    //MARK:Update the Game Status Line
    //Update display text // update the score/lives/level text at screen top
    gameStats << "Score: " << getScore() << "  Lvl: " << getLevel() << "  Souls2Save: " << m_souls2Save <<  "  Lives: " << getLives() << "  Health: " << m_racer->getHP() << "  Sprays: " << m_racer->getAmmo() << "  Bonus: " << m_bonusPoints;
    setGameStatText(gameStats.str());
    gameStats.str("");      //clear the stringstream after displaying
    
    //increment bonus points -1 each tick
    m_bonusPoints--;
    
    // the player hasn’t completed the current level and hasn’t died, so
    // continue playing the current level
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    vector<Actor*>::iterator deleterIt = m_actors.begin();
    while (!m_actors.empty()){
        delete *deleterIt;
        deleterIt = m_actors.erase(deleterIt);
    }
    if ( m_racer != nullptr )
        delete m_racer;
}


//***********************************************************************************************
//RACER GETTERS
//***********************************************************************************************
double StudentWorld::getRacerSpeed() const{
    return m_racer->getVertSpeed();
}
int StudentWorld::getRacerDirection() const{
    return m_racer->getDirection();
}
double StudentWorld::getRacerX() const{
    return m_racer->getX();
}
double StudentWorld::getRacerY() const{
    return m_racer->getY();
}
bool StudentWorld::racerIsAlive() const{
    return m_racer->isAlive();
}
//***********************************************************************************************
//RACER SETTERS
//***********************************************************************************************
void StudentWorld::killRacer(){
    if (!m_racer->isAlive())                //can't kill racer if already dead
        return;
    m_racer->killActor();
}
void StudentWorld::damageRacer(const int& amt){
    if (!m_racer->isAlive())                //can't damage racer if already dead
        return;
    m_racer->getDamaged(amt);
}
void StudentWorld::spinRacer(){
    if (!m_racer ->isAlive())               //can't spin racer if already dead
        return;
    m_racer->getSpun();
}
void StudentWorld::healRacer(const int& amt){
    if (!m_racer->isAlive())
        return;                             //can't heal racer if racer is dead
    m_racer->getHealed(amt);
}
void StudentWorld::giveRacerAmmo(const int& amt){
    if (!m_racer->isAlive())
        return;                             //can't give racer ammo if racer is dead
    m_racer->giveAmmo(amt);
}

//***********************************************************************************************
//OTHER PUBLIC SETTERS
//***********************************************************************************************
void StudentWorld::addActor(Actor* a){
    m_actors.push_back(a);
}
void StudentWorld::saveASoul(){
    m_souls2Save --;
}

//***********************************************************************************************
//OTHER PUBLIC HELPERS/OTHER METHODS
//***********************************************************************************************
bool StudentWorld::overlapWithRacer(const Actor* a) const{ 
    if (theyOverlap(a, m_racer))
        return true;
    return false;
}
void StudentWorld::closestCAWActorsInLane(const Actor* a, double& pixelsFront, double& pixelsBack) const{       //finds closest CAW actors in lane that are still alive and should be avoided
    int actorALane = getActorLane(a);   //get the actors lane
    pixelsFront = VIEW_HEIGHT;          //reset these to max pixels, this will be the default if we don't find any actors in the lane in front or behind
    pixelsBack = -VIEW_HEIGHT;
    
    for (int i = 0; i < m_actors.size(); i++){
        if ( m_actors[i]->isCollisionAvoidanceWorthy() && m_actors[i]->isAlive() && m_actors[i]!=a){  //MARK: Added check for aliasing
            if (getActorLane(m_actors[i]) == actorALane){
                double pixelsBetween = m_actors[i]->getY() - a->getY();
                if ( pixelsBetween > 0 && pixelsBetween < pixelsFront )
                    pixelsFront = pixelsBetween;
                else if ( pixelsBetween < 0 && pixelsBetween > pixelsBack )
                    pixelsBack = pixelsBetween;
            }
        }
    }
}
bool StudentWorld::projectileMaybeDamageActor(const Actor* projectile) const{
    for (int i = 0; i< m_actors.size(); i++){
        if (theyOverlap(projectile, m_actors[i]) && m_actors[i]!= projectile && m_actors[i]->isAffectedByProjectiles() && m_actors[i]->isAlive()){
            m_actors[i]->hitByProjectile(1);            //projectile attempts to damage actor by 1. This method may cause instant death for relevant goodies, reverse a human ped, or damage zombie enemies
            return true;        //we hit an actor that could be damaged via projectile (AND WAS ALIVE), return true immediately don't damage multiple. This true tells the projectile to mark itself for death
        }
    }
    return false;               //we didn't hit any projectile damageable actors. false tells projectile to stay alive since it didn't hit anything
}

//***********************************************************************************************
//PRIVATE HELPERS
//***********************************************************************************************
bool StudentWorld::theyOverlap(const Actor* a, const Actor* b) const{
    double delta_x = std::abs( a->getX() - b->getX() );
    double delta_y = std::abs( a->getY() - b->getY() );
    int radius_sum = a->getRadius() + b->getRadius();
    if (delta_x < radius_sum* 0.25 && delta_y < radius_sum * 0.6)           //if condition is met, two actors are said to overlap
        return true;
    return false;                                                           //otherwise return false
}

bool StudentWorld::findSafePlaceForCab(int& chosenLane, double& startY, double& startSpeed){
    //generate a random unordered set of lanes to check
    std::unordered_set<int> laneChecker;    //hold the lanes to check (in random order)
    int cur_lane = -1;  //vars for checking lanes and also the initialization of zombie cab
    while (laneChecker.size() < 2)
        laneChecker.insert(randInt(1,3));   //left lane is defied lane 1, middle is lane 2, right is lane 3. randomly add the first two lanes
    for (int finalAdd = 1; laneChecker.size() < 3; finalAdd++)
        laneChecker.insert(finalAdd);       //add whatever lane wasn't in there yet (saves time and still completely random since third in the set's prob is defined by the inclusion of the other two lanes)
    //now check each lane for a safe spot
    while (laneChecker.size() > 0){
        std::unordered_set<int>::iterator it;
        it = laneChecker.begin();
        cur_lane = *it;                 //get the random lane
        it = laneChecker.erase(it);     //and remove it from the set
        //determine closes CAW actor to bottom of screen in candidate lane
        double lowestY = lowestCAWActorInLane(cur_lane); //MARK: return val of this func implictly checks whether any actor in the lane at all for the sake of the below if statement.
        if (lowestY > VIEW_HEIGHT / 3){
            //if no actors in bottom third of lane, add cab to lane with starting Y and speed as follows:
            chosenLane = cur_lane;
            startY = SPRITE_HEIGHT/2;
            startSpeed = getRacerSpeed() + randInt(2,4);
            return true;                                //like break in the spec, returning true because we found a place to add the cab
        }
        double highestY = highestCAWActorInLane(cur_lane);  //MARK: return val of this func implictly checks whether any actor in the lane at all for the sake of the below if statement.
        if (highestY < VIEW_HEIGHT * 2/3){                  //if there were actors in bottom third, but not top third, set up as following:
            chosenLane = cur_lane;
            startY = VIEW_HEIGHT - SPRITE_HEIGHT/2;
            startSpeed = getRacerSpeed() - randInt(2,4);
            return true;                                //like break in the spec, returning true because we found a place to add the cab
        }
        //otherwise, current lane is too dangerous to add zombie lab (continue loop if other lanes to check)
        //restart loop to check the next lane
    }
    return false;               //we never found a safe place for the cab in any lane :(
}

int StudentWorld::getActorLane(const Actor* a) const{               //gets the lane a given actor falls in (>= left of lane, < right of lane per spec)
    int laneNumber = -1;
    if (a->getX() >= LEFT_EDGE && a->getX() < LEFT_EDGE + ROAD_WIDTH/3)
        laneNumber = 1;
    else if (a->getX() >= LEFT_EDGE + ROAD_WIDTH/3 && a->getX() < RIGHT_EDGE - ROAD_WIDTH/3)
        laneNumber = 2;
    else if (a->getX() >= RIGHT_EDGE - ROAD_WIDTH/3 && a->getX() < RIGHT_EDGE)
        laneNumber = 3;
    return laneNumber;
}

double StudentWorld::lowestCAWActorInLane(const int& laneNumber){       //finds y location of lowest alive CAW actor in the lane given
    double lowestY = VIEW_HEIGHT + 1;              //default of VIEW_HEIGHT + 1 if no actor in lane
    for (int i = 0; i < m_actors.size(); i++){
        if (getActorLane(m_actors[i]) == laneNumber && m_actors[i]->isAlive() && m_actors[i]->isCollisionAvoidanceWorthy() && m_actors[i]->getY() < lowestY )
            lowestY = m_actors[i]->getY();
    }
    //also check ghost racer even tho she's not in the vector
    return std::min(getRacerY(), lowestY);                     //returns VIEW_HEIGHT + 1 if no CAW Actors found, otherwise returns y value of lowest CAW actor in the lane (INCL. Ghost Racer)
}

double StudentWorld::highestCAWActorInLane(const int& laneNumber){      //finds y location of highest CAW actor in the lane given
    double highestY = -1;              //default of -1 if no actor in lane
    for (int i = 0; i < m_actors.size(); i++){
        if (getActorLane(m_actors[i]) == laneNumber && m_actors[i]->isAlive() && m_actors[i]->isCollisionAvoidanceWorthy() && m_actors[i]->getY() > highestY )
            highestY = m_actors[i]->getY();
    }
    //also check ghost racer even tho she's not in the vector
    return std::max(getRacerY(), highestY);                     //returns VIEW_HEIGHT + 1 if no CAW Actors found, otherwise returns y value of lowest CAW actor in the lane INCL. GHOST RACER
}

void StudentWorld::addBorderLines(){
    //add border lines:
    double new_border_y = VIEW_HEIGHT - SPRITE_HEIGHT;
    double delta_y = new_border_y - m_highestWhiteBorder;
    if ( delta_y >= SPRITE_HEIGHT ){
        //need to add yellow borders
        Actor* newLeftYellowBorder = new BorderLine(IID_YELLOW_BORDER_LINE, LEFT_EDGE, new_border_y, this);
        Actor* newRightYellowBorder = new BorderLine(IID_YELLOW_BORDER_LINE, RIGHT_EDGE, new_border_y, this);
        m_actors.push_back(newLeftYellowBorder);
        m_actors.push_back(newRightYellowBorder);
    }
    if ( delta_y >= 4*SPRITE_HEIGHT ){
        Actor* newLeftWhiteBorder = new BorderLine(IID_WHITE_BORDER_LINE, LEFT_EDGE + ROAD_WIDTH/3, new_border_y, this);
        Actor* newRightWhiteBorder = new BorderLine(IID_WHITE_BORDER_LINE, RIGHT_EDGE - ROAD_WIDTH/3, new_border_y, this);
        m_actors.push_back(newLeftWhiteBorder);
        m_actors.push_back(newRightWhiteBorder);
        m_highestWhiteBorder = newLeftWhiteBorder->getY();
    }
}
void StudentWorld::addZombieCabs(){
    int ChanceVehicle = max(100- getLevel() * 10, 20);
    if (randInt(0, ChanceVehicle - 1) == 0){            //if we meet our defined zombie cab odds, atempt to add a zombie cab to the road
        int chosenLane = -1;
        double startX = -1, startY = -1;
        double startSpeed = 0;
        if ( findSafePlaceForCab(chosenLane, startY, startSpeed) ){     //try to find a safe starting condition for the cab!
            switch (chosenLane)
            {
                case 1:
                {
                    startX = ROAD_CENTER - ROAD_WIDTH/3;
                    break;
                }
                case 2:
                {
                    startX = ROAD_CENTER;
                    break;
                }
                case 3:
                {
                    startX = ROAD_CENTER + ROAD_WIDTH/3;
                    break;
                }
            }
            Actor* newZombieCab = new ZombieCab(startX, startY, startSpeed, this);
            m_actors.push_back(newZombieCab);
        }
        //otherwise, we didn't find a safe place for the cab to start, so we won't add one this tick
    }
}
void StudentWorld::addOilSlicks(){
    int ChanceOilSlick = std::max(150 - getLevel() * 10, 40);
    if (randInt(0, ChanceOilSlick -1) == 0){
        Actor* newOilSlick = new OilSlick(randInt(ROAD_CENTER - ROAD_WIDTH /2, ROAD_CENTER + ROAD_WIDTH/2), VIEW_HEIGHT, this);
        m_actors.push_back(newOilSlick);
    }
}
void StudentWorld::addZombiePeds(){
    int ChanceZombiePed = max(100 - getLevel() * 10, 20);
    if (randInt(0, ChanceZombiePed - 1) == 0){
        double xPlacement = randInt(0, VIEW_WIDTH-1);  //MARK: Fixed to this after talking with profs, technically unclear in spec
        double yPlacement = VIEW_HEIGHT;
        Actor* newHumanPed = new ZombiePedestrian(xPlacement, yPlacement, this);
        m_actors.push_back(newHumanPed);
    }
}
void StudentWorld::addHumanPeds(){
    int ChanceHumanPed = max(200 - getLevel() * 10, 30);
    if (randInt(0, ChanceHumanPed - 1) == 0 ){
        int xPlacement = randInt(0, VIEW_WIDTH-1);      //MARK: Fixed to this after talking with profs, technically unclear in spec
        int yPlacement = VIEW_HEIGHT;
        Actor* newHumanPed = new HumanPedestrian(xPlacement, yPlacement, this);
        m_actors.push_back(newHumanPed);
    }
}
void StudentWorld::addHolyWaterGoodies(){
    int ChanceOfHolyWater = 100 + 10 * getLevel();
    if (randInt(0, ChanceOfHolyWater - 1) == 0){
        Actor* newHWGoodie = new HolyWaterGoodie(randInt(ROAD_CENTER - ROAD_WIDTH /2, ROAD_CENTER + ROAD_WIDTH/2), VIEW_HEIGHT, this);
        m_actors.push_back(newHWGoodie);
    }
}
void StudentWorld::addSoulGoodies(){
    int ChanceOfLostSoul = 100;
    if (randInt(0, ChanceOfLostSoul -1) == 0){
        Actor* newLostSoul = new SoulGoodie(randInt(ROAD_CENTER - ROAD_WIDTH /2, ROAD_CENTER + ROAD_WIDTH/2), VIEW_HEIGHT, this);
        m_actors.push_back(newLostSoul);
    }
}

