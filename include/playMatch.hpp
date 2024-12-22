#pragma once
//Play an individual match with these starting settings, and these

class Player;
class Game;

#include<cstdint>
#include<functional>

bool playMatch(bool graphics,int32_t& turns,const Player players[2],Game& G,std::function<int8_t()> roll);



bool playMatchBot(int32_t& turns,const Player players[2],Game& G,std::function<int8_t()> roll);


bool playMatchBot(int32_t& turns,const Player& A,const Player& B,Game& G,std::function<int8_t()> roll);
