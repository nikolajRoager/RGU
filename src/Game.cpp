#include"Game.hpp"
#include<curses.h>
#include<array>
#include<iostream>


Game::Game(int32_t _state, int8_t _unstarted) noexcept: state(_state), unstarted(_unstarted)
{
    state=_state;
    unstarted=_unstarted;

    int8_t pieces_counted0 = getPlayerUnstarted(false);
    int8_t pieces_counted1 = getPlayerUnstarted(true);
    int32_t state0 =state;
    int32_t state1 =state>>14;



    for (int8_t i=0; i<14; ++i)
    {
        if (state0&1)
        {
            if (pieces_counted0<pieces)
                ++pieces_counted0;
             else//No more pieces than allowed
                state=state ^ (1<<i);
        }
        if (state1&1)
        {
            if (pieces_counted1<pieces)
                ++pieces_counted1;
            else//No more pieces than allowed
                state=state ^ (1<<(14+i));
        }
        state0=state0>>1;
        state1=state1>>1;
    }

    finished = ((7-pieces_counted1)<<4) | ((7-pieces_counted0)&right4bits);

}






int8_t Game::getPlayerPos(bool player,int8_t piece)const noexcept
{
    //Loop through up to all 7 pieces on the board, and check if any is the one we ask for
    int8_t current_piece = getPlayerUnstarted(player);
    int8_t last_piece = pieces-getPlayerFinished(player);
    if      (piece < current_piece)
        return 0;//This piece is not started yet
    else if (piece > last_piece)
        return 15;//This piece is finished
    else
    {
        for (int32_t _state =state>>(player ? 14 : 0); current_piece<piece;_state=_state>>1)
            if (_state&1)
                ++current_piece;
        return current_piece;
    }

    return 0;
}



void Game::print(int locx,int locy)const noexcept
{
    //Make sure to clear the line with the state
    mvprintw(locy,locx,"state=%d %d                          ",state,unstarted);
    mvprintw(locy+1,locx,"P0 waiting: %d Home: %d",getPlayerUnstarted(false),getPlayerFinished(false));
    mvprintw(locy+2,locx,"P1 waiting: %d, Home: %d",getPlayerUnstarted(true),getPlayerFinished(true));


/*
The board looks like this
  ________________
 / /\ |      | /\ \
  <  >        <  >|
 \_\/_/\_  _/\_\/_/
 /    \/    \/    \
 |    ||    ||    |
 \_  _/\_  _/\_  _/
 /    \/    \/    \
 |    ||    ||    |
 \_  _/\_  _/\_  _/
 /    \/ /\ \/    \
 |    ||<  >||    |
 \^^^^/\_\/_/\^^^^/
       /    \
       |    |
       \_  _/
       /    \
       |    |
       \_  _/
 /^/\^\/    \/^/\^\
 |<  >||    ||<  >|
 \_\/_/\_  _/\_\/_/
 /    \/    \/    \
 |                |
 \____|______|____/

Player 0 places @@ at the center, player 1 ##


The need to use \\ for \ makes it a little hard to see how the print commands below work
*/

mvprintw(locy+3 ,locx," ________________ ");
mvprintw(locy+4 ,locx,"/3/\\ | 4    |3/\\ \\");
mvprintw(locy+5 ,locx," <%s>   %s   <%s>|",(getPosOccupant(false,0x3)? "@@" : "  "),(getPosOccupant(false,0x4)? "@@" : (getPosOccupant(true,0x4)? "##" : "  ")),(getPosOccupant(true,0x3)? "##" : "  "));
mvprintw(locy+6 ,locx,"\\_\\/_/\\_  _/\\_\\/_/");
mvprintw(locy+7 ,locx,"/2   \\/5   \\/2   \\");
mvprintw(locy+8 ,locx,"| %s || %s || %s |",(getPosOccupant(false,0x2)? "@@" : "  "),(getPosOccupant(false,0x5)? "@@" : (getPosOccupant(true,0x5)? "##" : "  ")),(getPosOccupant(true,0x2)? "##" : "  "));
mvprintw(locy+9 ,locx,"\\_  _/\\_  _/\\_  _/");
mvprintw(locy+10,locx,"/1   \\/6   \\/1   \\");
mvprintw(locy+11,locx,"| %s || %s || %s |",(getPosOccupant(false,0x1)? "@@" : "  "),(getPosOccupant(false,0x6)? "@@" : (getPosOccupant(true,0x6)? "##" : "  ")),(getPosOccupant(true,0x1)? "##" : "  "));
mvprintw(locy+12,locx,"\\_  _/\\_  _/\\_  _/");
mvprintw(locy+13,locx,"/0   \\/7/\\ \\/0   \\");
mvprintw(locy+14,locx,"| %s ||<%s>|| %s |",(getPosOccupant(false,0x0)? "@@" : "  "),(getPosOccupant(false,0x7)? "@@" : (getPosOccupant(true,0x7)? "##" : "  ")),(getPosOccupant(true,0x0)? "##" : "  "));
mvprintw(locy+15,locx,"\\    /\\_\\/_/\\    /");
mvprintw(locy+16,locx," ^^^^ /8   \\ ^^^^ ");
mvprintw(locy+17,locx,"      | %s |      ",(getPosOccupant(false,0x8)? "@@" : (getPosOccupant(true,0x8)? "##" : "  ")));
mvprintw(locy+18,locx,"      \\_  _/      ");
mvprintw(locy+19,locx,"      /9   \\      ");
mvprintw(locy+20,locx,"      | %s |      ",(getPosOccupant(false,0x9)? "@@" : (getPosOccupant(true,0x9)? "##" : "  ")));
mvprintw(locy+21,locx," ^^^^ \\_  _/ ^^^^ ");
mvprintw(locy+22,locx,"/D/\\ \\/A   \\/D/\\ \\");
mvprintw(locy+23,locx,"|<%s>|| %s ||<%s>|",(getPosOccupant(false,0xD)? "@@" : "  "),(getPosOccupant(false,0xA)? "@@" : (getPosOccupant(true,0xA)? "##" : "  ")),(getPosOccupant(true,0xD)? "##" : "  "));
mvprintw(locy+24,locx,"\\_\\/_/\\_  _/\\_\\/_/");
mvprintw(locy+25,locx,"/C   \\/B   \\/C   \\");
mvprintw(locy+26,locx,"| %s    %s    %s |",(getPosOccupant(false,0xC)? "@@" : "  "),(getPosOccupant(false,0xB)? "@@" : (getPosOccupant(true,0xB)? "##" : "  ")),(getPosOccupant(true,0xC)? "##" : "  "));
mvprintw(locy+27,locx,"\\____|______|____/");







}




//Get the list of legal moves here
std::vector<std::pair<int8_t/*What pieces are involved in this move*/,Game> > Game::getLegalMoves(bool player, int8_t roll)
{
    std::vector<std::pair<int8_t,Game>> out;

    //Loop through up to all 7 pieces on the board, and check if any is the ones can move

    if (roll==0)
        return out;

    if (getPlayerUnstarted(player)>0)
    {
        //We can not take enemy pieces directly from our start, as we can not reach the combat zone, so only check against my own pieces
        if (!getPosOccupant(player,roll-1/*It takes one step to move out of the start*/))
        {
            //The new state will have one fewer pieces in my home, and one more at position roll, if roll=4 the turn bit will not be flipped
            //mvprintw(2,0,"Can move from 0 to %d",roll);
            out.push_back(std::make_pair(-1/*-1= not started yet*/,Game
            (
                //Set the bit at where we move to (we already know it is 0)
                (state | 1<<(roll-1+(player? 14 : 0)))^(roll!=4 ? turnFlag : 0/*If we land on field 3, we do not flip the turn flag*/),
                //Calculate the new unstarted halfbytes
                ((getPlayerUnstarted(true)-(player?1:0))<<4) | ( (getPlayerUnstarted(false)-(player?0:1) )&right4bits),//New unstarted, one lower on the respective side
                finished//Nothing can reach the finish like this
            ))
            );
        }


    }
    for (int8_t i = 0; i < 14; ++i)
    {
        if ((state>>((player ? 14 : 0)+i))&1)
        {
            //Enemies only block, if on the safe-spot in the middle
            int8_t new_pos = i+roll;
            if (new_pos==14)
            {
                //mvprintw(3,0,"Can move from %d to finish",i);
                out.push_back(std::make_pair(i,Game
                (
                    //Set the bit we left to 0
                    ((state)^(1<<(i+(player? 14 : 0))))^turnFlag,
                    unstarted,
                    //Calculate the new unstarted halfbytes
                    ((getPlayerFinished(true)+(player?1:0))<<4) | ( (getPlayerFinished(false)+(player?0:1) )&right4bits)
                ))
                );

            }
            else if (new_pos<14)
            {
                if (!getPosOccupant(player,new_pos) && (new_pos!=7 || !getPosOccupant(!player,new_pos)))
                {
                    //mvprintw(3,0,"Can move from %d to %d",i,new_pos);
                    if (new_pos>3 && new_pos<12 && getPosOccupant(!player,new_pos))
                    {
                        out.push_back(std::make_pair(i,Game
                        (
                            //Set the bit at where we move to 1, and where we moved from to 0 ... and also set the bit we landed on to 0

                            (((state | (1<<(new_pos +(player? 14 : 0))) )^(1<<(i+(player? 14 : 0))))^(1<<(new_pos+(player? 0 : 14))))^turnFlag/*There is no free turn + take piece*/,
                            //Calculate the new unstarted halfbytes
                            ((getPlayerUnstarted(true)+(player?0:1))<<4) | ( (getPlayerUnstarted(false)+(player?1:0) )&right4bits),//New unstarted, one lower on the respective side
                            finished
                        ))
                        );

                    }
                    else
                    {

                        out.push_back(std::make_pair(i,Game
                        (
                            //Set the bit at where we move to 1, and where we moved from to 0
                            ((state | (1<<(new_pos +(player? 14 : 0))) )^(1<<(i+(player? 14 : 0))))^((new_pos==3 || new_pos==7 || new_pos==13)? 0:turnFlag),
                            //Calculate the new unstarted halfbytes
                            unstarted,
                            finished
                        ))
                        );

                    }


                }
            }


        }
    }



    return out;
}


void Game::display_board(std::function<void(std::pair<int,int>,bool)> displayfunc, std::function<std::pair<int,int>(int,bool)> posfunc,int hide_piece_id, bool player,std::deque<piece_animation>& animations, uint32_t millis) const noexcept
{
    for (int i = 0; i<14 ; ++i)
        if (getPosOccupant(false,i))
            if (player || hide_piece_id!=i+1)
            {
                bool animated=false;
                for (piece_animation& A : animations)
                    if (A.hide==i+1 && !A.player && millis<=A.end)
                        animated=true;
                if (!animated)
                    displayfunc(posfunc(i+1,false),false);
            }

    for (int i = 0; i<14 ; ++i)
        if (getPosOccupant(true,i))
            if (!player || hide_piece_id!=i+1)
            {
                bool animated=false;
                for (piece_animation& A : animations)
                    if (A.hide==i+1 && A.player && millis<=A.end)
                        animated=true;
                if (!animated)
                    displayfunc(posfunc(i+1,true),true);
            }

}

void Game::display_below(std::function<void(std::pair<int,int>,bool)> displayfunc, std::function<std::pair<int,int>(int,bool)> posfunc,int hide_piece_id, bool player,std::deque<piece_animation>& animations,uint32_t millis) const noexcept
{
    //Print unstarted
    int p0_wait = getPlayerUnstarted(false);
    int p1_wait = getPlayerUnstarted(true);
    for (int i = (!player && hide_piece_id==0)?1:0; i<p0_wait; ++i)
    {
        bool animated=false;
        for (piece_animation& A : animations)
            if (A.hide==-i && !A.player && millis<=A.end)
                animated=true;
        if (!animated)
            displayfunc(posfunc(-i,false),false);
    }
    for (int i = (player && hide_piece_id==0)?1:0; i<p1_wait; ++i)
    {
        bool animated=false;
        for (piece_animation& A : animations)
            if (A.hide==-i && A.player && millis<=A.end)
                animated=true;
        if (!animated)
            displayfunc(posfunc(-i,true),true);

    }

    //Print finished
    int p0_fin = getPlayerFinished(false);
    int p1_fin = getPlayerFinished(true);
    for (int i = 0; i<p0_fin ; ++i)
    {
        bool animated=false;
        for (piece_animation& A : animations)
            if (A.hide==i+16 && !A.player && millis<A.end)
                animated=true;
        if (!animated)
            displayfunc(posfunc(16+i,false),false);
    }
    for (int i = 0; i<p1_fin ; ++i)
    {
        bool animated=false;
        for (piece_animation& A : animations)
            if (A.hide==i+16 && A.player && millis<A.end)
                animated=true;
        if (!animated)
            displayfunc(posfunc(16+i,true),true);
    }
}
