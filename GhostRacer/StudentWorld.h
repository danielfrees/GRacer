#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>

#include <vector>       //for vector container to actors
#include <sstream>      //for stringstream member tracking game data



class Actor;
class GhostRacer;

class StudentWorld : public GameWorld
{
public:
    //StudentWorld constants
    const int LEFT_EDGE = ROAD_CENTER - ROAD_WIDTH/2;
    const int RIGHT_EDGE = ROAD_CENTER + ROAD_WIDTH/2;
    
    //MAIN METHODS:
    StudentWorld(std::string assetPath);
    virtual ~StudentWorld(){
        cleanUp();
    }
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    
    //public getters to learn about Ghost Racer:
    double getRacerSpeed() const;
    int getRacerDirection() const;
    double getRacerX() const;
    double getRacerY() const;
    bool racerIsAlive() const;
    
    //public setters to affect the GhostRacer:
    void killRacer();
    void damageRacer(const int& amt);
    void spinRacer();
    void healRacer(const int& amt);
    void giveRacerAmmo(const int& amt);
    
    //Other Public setters:
    void saveASoul();
    void addActor(Actor* a);                                            //allows actors to add another actor to the world
    
    //Public helpers/ other methods
    bool overlapWithRacer(const Actor* a) const;                                                //allows actors to determine whether they overlap with the racer
    void closestCAWActorsInLane(const Actor* a, double& pixelsFront, double& pixelsBack) const;       //finds the closest live CAW actors in front or behind the given actor (in the same lane). Does not count GRacer, used for cab slowing and speeding up and cabs don't avoid GRacer.
    bool projectileMaybeDamageActor(const Actor* projectile) const;
    
private:
    std::vector< Actor* >       m_actors;                                   //container for all actors
    GhostRacer*                 m_racer;                                    //separate pointer to GhostRacer
    int                         m_souls2Save;                               //holds # of souls left to save on current level
    int                         m_bonusPoints;                              //holds # of bonus points awarded if win level rn
    double                      m_highestWhiteBorder;                       //holds y coord of highest white border line
    std::ostringstream          gameStats;                                  //stringstream which is edited and set as game text each tick
    
    //private helpers:
    bool theyOverlap(const Actor* a, const Actor* b) const;                             //finds whether two actors overlap
    bool findSafePlaceForCab(int& chosenLane, double& startY, double& startSpeed);         //tries to find a safe place to spawn a Zombie Cab
    int getActorLane(const Actor* a) const;                                             //returns lane an Actor is in
    double lowestCAWActorInLane(const int& laneNumber);                                    //returns the y coordinate of the lowest CAW Actor in any given lane. used for cab spawning and includes GRacer
    double highestCAWActorInLane(const int& laneNumber);                                   //returns y coord of highest CAW Actor in any given lane. Used for cab spawning and includes GRacer
    void addBorderLines();
    void addZombieCabs();
    void addOilSlicks();
    void addZombiePeds();
    void addHumanPeds();
    void addHolyWaterGoodies();
    void addSoulGoodies();
    
};

#endif // STUDENTWORLD_H_
