#include"Player.hpp"
#include"Game.hpp"

#include<curses.h>
#include<cstring>
#include<iostream>

Player::Player(bool _bot) noexcept: bot(_bot)
{

}


Player::Player(bool _bot, std::array<int8_t,465> weights ) noexcept: bot(_bot), brain_weights(weights)
{
}

Player::Player(bool _bot,std::istream& input): bot(_bot)
{
    size_t i=0;
    int I;
    while (input>>I && i<465){brain_weights[i++]=I;}
}



int Player::playMove(std::vector<std::pair<int8_t,Game> >& moves, bool me, int8_t /*roll*/ ) const noexcept
{
    //std::cout<<"-Player "<<me<<" evaluation-"<<std::endl;
    int8_t moveNumber = static_cast<int8_t>(moves.size());

    int16_t best_score = 0.0;
    int8_t best_move = -1;
    for (int8_t i =0; i < moveNumber; ++i)
    {
        int16_t score = evaluate(moves[i].second,me);

        //std::cout<<" Move "<<(int)i<<" (move "<<(int)moves[i].first<<" to "<<(int)(moves[i].first+roll)<<"), score = "<<score<<std::endl;
        if (best_move==-1 || score>best_score)
        {
            best_score=score;
            best_move=i;
        }

    }
    return best_move;

}

void Player::play(Game& board,int8_t roll) const noexcept
{

    //This assumes I am the player, whose turn it is
    bool me = board.whoseTurn();

    std::vector<std::pair<int8_t,Game> > moves = board.getLegalMoves(me, roll);

    int8_t moveNumber = static_cast<int8_t>(moves.size());

    if (moveNumber>0)
    {
        int16_t best_score = 0.0;
        int8_t best_move = -1;
        for (int8_t i =0; i < moveNumber; ++i)
        {
            int16_t score = evaluate(moves[i].second,me);
            if (best_move==-1 || score>best_score)
            {
                best_score=score;
                best_move=i;
            }

        }
        board=Game (moves[best_move].second);
    }
}

//int debug_print_offset=40;

void Player::play(Game& board,int8_t roll,bool graphics) const noexcept
{

    //This assumes I am the player, whose turn it is
    bool me = board.whoseTurn();

    std::vector<std::pair<int8_t,Game> > moves = board.getLegalMoves(me, roll);

    int8_t input;
    int8_t moveNumber = static_cast<int8_t>(moves.size());

    if (moveNumber>0)
    {
        if (bot)
        {
            if (graphics)
                mvprintw(2,0,          "Legal moves    | evaluation");
            int16_t best_score = 0.0;
            int8_t best_move = -1;
            for (int8_t i =0; i < moveNumber; ++i)
            {
                //debug_print_offset+=18;
                int16_t score = evaluate(moves[i].second,me);
                if (best_move==-1 || score>best_score)
                {
                    best_score=score;
                    best_move=i;
                }

                if (graphics)
                {
                    if (moves[i].first==-1)
                        mvprintw(3+i,0," %d) start to %x | %d",i,moves[i].first+roll,score);
                    else if (moves[i].first+roll==14)
                        mvprintw(3+i,0," %d) %x to finish| %d",i,moves[i].first,score);
                    else
                        mvprintw(3+i,0," %d) %x to %x    | %d",i,moves[i].first,moves[i].first+roll,score);
                }
            }
            if (graphics)
                mvprintw(3+moveNumber,0,"Bot chooses %d",best_move);
            board=Game (moves[best_move].second);
            getch();

        }
        else
        {
            do
            {
                if (graphics)
                    mvprintw(2,0,          "Legal moves    | evaluation");

                for (int8_t i =0; i < moveNumber; ++i)
                {

                    int16_t score = evaluate(moves[i].second,me);

                    if (graphics)
                    {
                        if (moves[i].first==-1)
                            mvprintw(3+i,0," %d) start to %x | %d",i,moves[i].first+roll,score );
                        else if (moves[i].first+roll==14)
                            mvprintw(3+i,0," %d) %x to finish| %d",i,moves[i].first,score );
                        else
                            mvprintw(3+i,0," %d) %x to %x    | %d",i,moves[i].first,moves[i].first+roll,score );
                    }
                }
                if (graphics)
                    mvprintw(3+moveNumber,0," Enter action:");
                input = getch()-'0';

            }
            while (input<0 || input>=moveNumber);

            board=Game (moves[input].second);
        }
    }
    else
    {
        if (graphics)
            mvprintw(2,0,"No legal moves!");
        board.flipTurn();
    }
}

int16_t Player::evaluate(const Game& board,bool whoami) const noexcept
{
    //Loop through my and their positions
    //int32_t state    = board.getPlayerData();

    //After some experimenting, using these two states and using if-then statements to check which one to look up in offers slightly better performance, this is mainly important for training time
    //Using int32_t is better for performance, since casting state to int16_t takes time
    int32_t my_state    = board.getPlayerData( whoami);
    int32_t their_state = board.getPlayerData(!whoami);

    int16_t score = 0;


    int8_t multiplier;
    int8_t this_multiplier;

    //mvaddstr(5,debug_print_offset,"eval");
    //int debug_y=5;
    {
        //We are only going to to use a lower-triangular matrix
        for (int8_t j = 0; j < 30; ++j)
        {
            multiplier=1;
            if (j==28)
            {
                multiplier*=board.getPlayerFinished(whoami);
          //      mvprintw(++debug_y,debug_print_offset," finished me: %d",multiplier);
            }
            else if (j==29)
            {
                multiplier*=board.getPlayerFinished(!whoami);
            }
            else if (j<14)
            {
                if (!((my_state>>j)&1))
                    continue;//Set multiplier to 0 (but that makes everything else is pointless, so skip it)
            }
            else if (j>14)
            {
                if (!((their_state>>(j-14))&1))
                    continue;
            }
            //If the multiplier has been set to 0, nothing in this column matters
            if (multiplier==0)
                continue;


            for (int8_t i = j; i < 30; ++i)
            {
                this_multiplier=multiplier;

                if (i==28)
                {
                    this_multiplier*=board.getPlayerFinished(whoami);
                }
                else if (i==29)
                {
                    this_multiplier*=board.getPlayerFinished(!whoami);
                }
                else if (i<14)
                {
                    if (!((my_state>>i)&1))
                        continue;//Set multiplier to 0 (but that makes everything else is pointless, so skip it)
                }
                else if (i>14)
                {
                    if (!((their_state>>(i-14))&1))
                        continue;
                }
    //            if (i==28 || j ==28)
    //            {
    //                if(this_multiplier>0)
   //                 {
 //                       mvprintw(++debug_y,debug_print_offset," i=%d j=%d:%d:%d",i,j,(i+1)*i/2+j,brain_weights[(i+1)*i/2+j]);
 //                   }

      //          }
                score+=this_multiplier*((int16_t)brain_weights[(i+1)*i/2+j]);
            }
        }

    }
   // mvprintw(++debug_y,debug_print_offset,"re %d",score);



    return score;
}
