#pragma once

#include<cstdint>
#include<istream>
#include<array>
#include<vector>

class Game;

//For the sake of speed, the player is not a polymorphic class (polymorphism is SLOOOW) instead a few internal flags change how the player behaves

class Player
{
private:
    bool bot = false;

    //points for me/them having pieces on this position, or having pieces finished
    //The points are calculated like N[a]N[b]weights[a,b], where N[a] is the number of pieces on position a (usually just 0 or 1), for the purpose of this,
    std::array<int8_t,465> brain_weights=
    {

         1,
         0,  3,
         0,  0,  3,
         0,  0,  0,  4,
         0,  0,  0,  0,  5,
         0,  0,  0,  0,  0,  6,
         0,  0,  0,  0,  0,  0,  7,
         0,  0,  0,  0,  0,  0,  0,  12 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,11 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,12 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,14 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,17 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,24 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,24 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,-1 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-3 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-3 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-4 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-5 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-6 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-7 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-12,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-11,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-12,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-14,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-17,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-24,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-24,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,50 ,
         0,  0,  0,  0,  0,  0,  0,  0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,-50

    };




public:


    Player(bool bot) noexcept;
    Player(bool bot, std::istream& input);
    Player(bool bot,std::array<int8_t,465> weights) noexcept;

    float getBrainWeight(int8_t j, int8_t i) const {return brain_weights[(i+1)*i/2+j];}

    void setBrainWeight(int8_t j, int8_t i,float val)
    {
        if ( (i+1)*i/2+j<(int)brain_weights.size())
            brain_weights[(i+1)*i/2+j]=val;
    }

    std::array<int8_t,465> getBrain() const noexcept {return brain_weights;}


    //Playing this turn, does not change my internal state, only the game board
    //This is done without explicit reference to my opponent
    void play(Game& board,int8_t roll,bool graphics) const noexcept;

    //Version without even considering graphics (for bot training)
    void play(Game& board,int8_t roll) const noexcept;

    int playMove(std::vector<std::pair<int8_t,Game> >& moves, bool me, int8_t roll ) const noexcept;

    bool isBot() const noexcept {return bot;}

    int16_t evaluate(const Game& board,bool whoami) const noexcept;
};
