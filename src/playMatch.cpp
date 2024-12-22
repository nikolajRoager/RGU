#include"playMatch.hpp"
#include"Game.hpp"
#include"Player.hpp"

bool playMatch(bool graphics,int32_t& turns,const Player players[2],Game& G,std::function<int8_t()> getRoll)
{
    int8_t c=' ';
    while(c!='q' && G.gameOver()==0)
    {
        ++turns;
        if (graphics)
        {
            clear();
            G.print();
        }
        bool player=G.whoseTurn();


        if (graphics)
        {
            mvprintw(0,0,"Player %d turn",player);
            mvprintw(1,0,"Roll:");
        }
        int8_t roll = getRoll();


        players[player].play(G,roll,graphics);

        if (graphics)
        {
            G.print();
            mvprintw(10,0,"Press any key to continue");
            c=getch();
        }
    }
    return G.gameOver()==1;
}


bool playMatchBot(int32_t& turns,const Player players[2],Game& G,std::function<int8_t()> getRoll)
{
    while(G.gameOver()==0)
    {
        ++turns;
        bool player=G.whoseTurn();
        int8_t roll = getRoll();
        players[player].play(G,roll);
    }
    return G.gameOver()==1;
}



bool playMatchBot(int32_t& turns,const Player& A,const Player& B,Game& G,std::function<int8_t()> getRoll)
{
    while(G.gameOver()==0)
    {
        ++turns;
        bool player=G.whoseTurn();
        int8_t roll = getRoll();
        if (player)
            B.play(G,roll);
        else
            A.play(G,roll);
    }
    return G.gameOver()==1;
}
