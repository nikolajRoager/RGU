#pragma once

#include<cstdint>
#include<vector>
#include<deque>
#include<curses.h>
#include<functional>


#define right4bits            0b00001111
#define right14bits 0b0000000000000011111111111111
#define turnFlag    0b10000000000000000000000000000000
#define pieces 7

struct piece_animation
{
    int hide=-8;
    bool player;
    int x0,y0;
    int x1,y1;
    uint32_t start,end;
    piece_animation(int h,bool p,int _x0,int _y0,int _x1,int _y1, uint32_t _start, uint32_t _end):
    hide(h),player(p),x0(_x0),y0(_y0),x1(_x1),y1(_y1),start(_start),end(_end)
    {}
};

class Game
{
private:
    //I do believe the entire game can be compressed to 32 bits, but I do not think the decompression/compression time would be worth the 16 bits saved.
    int32_t state;//14 bits counts which positions are occupied by player 0, the next which are occupied by player 1, this leaves 4 bits unclaimed at the (left) end of this, we simply use the first bit here to indicate whose turn it is


    int8_t unstarted;//2 4 bit integers, counting how many not-started pieces there are
    int8_t finished;//2 4 bit integers, counting how many not-started pieces there are

public:

    inline bool whoseTurn() const noexcept {return turnFlag & state;}

    Game (int32_t _state=0, int8_t _unstarted=0b01110111) noexcept;
    Game(const Game& That)
    {
        state=That.state;
        unstarted=That.unstarted;
        finished=That.finished;


    }
    Game(uint32_t That_state, uint8_t That_unstarted, uint8_t That_finished): state(That_state),unstarted(That_unstarted),finished(That_finished)
    {

    }

    Game& operator=(const Game& That)
    {
        state=That.state;
        unstarted=That.unstarted;
        finished=That.finished;



        return *this;
    }



    //Display the pieces NOT on the board (and thus affected by board shadows)
    void display_below(std::function<void(std::pair<int,int>,bool)> displayfunc, std::function<std::pair<int,int>(int,bool)> posfunc,int hide_piece_id, bool player,std::deque<piece_animation>& animations,uint32_t millis) const noexcept;


    void display_board(
std::function<void(std::pair<int,int>,bool)> displayfunc, std::function<std::pair<int,int>(int,bool)> posfunc,int hide_piece_id, bool player,std::deque<piece_animation>& animations,uint32_t millis) const noexcept;

    void print(int locx=30,int locy=0) const noexcept;

    //Read the first or last halfbyte from the started and unstarted bytes, sadly, everything will temporarily be promoted to 32 bits, and there is nothing to do about it
    inline int8_t getPlayerFinished(bool player)const noexcept
    {
        return player? (finished>>4) : (finished& right4bits) ;
    }

    inline int8_t getPlayerUnstarted(bool player)const noexcept
    {
        return player? (unstarted>>4) : (unstarted & right4bits);
    }

    inline int8_t getFinished()const noexcept {return finished;}
    inline int8_t getUnstarted()const noexcept {return unstarted;}

    int8_t getPlayerPos(bool player,int8_t piece)const noexcept;

    inline bool getPosOccupant(bool player,int8_t pos) const noexcept
    {
        return ((state>>( (player ? 14:0)+pos)) & 1);
    }


    inline int gameOver()const noexcept
    {
        return getPlayerFinished(false)==7 ? 1 : (getPlayerFinished(true)==7 ? 2 : 0);
    }

    int32_t getPlayerData() const noexcept
    {
        return state;
    }


    //Using int32_t rather than downscaling to 16 bits is better for performance
    int32_t getPlayerData(bool player) const noexcept
    {
        return ((state>>((player ? 14:0))) & right14bits);
    }

    void flipTurn()
    {
        state=state^turnFlag;
    }


    //Get the list of legal moves here and what does the game look like after
    std::vector< std::pair<int8_t/*What pieces are involved in this move*/,Game> > getLegalMoves(bool player, int8_t roll);
};
