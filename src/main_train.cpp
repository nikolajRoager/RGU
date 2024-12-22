#include<iostream>
#include<string>
#include<random>
#include<fstream>
#include<sys/time.h>

#include"Game.hpp"
#include"Player.hpp"
#include"playMatch.hpp"
#include"optimize.hpp"

int main(int argc, char* argv[])
{
    std::default_random_engine rng(time(NULL));

    std::uniform_int_distribution<int> SingleDice(0,1);


    int32_t game_start_state = 0b0000000000000000000000000000;
    int32_t game_start_unstarted = 0b01110111;

    int32_t matches=10000;
    int32_t depth=2;

    std::string outname="out_brain";
    std::string brainfile0="brain0.txt";
    std::string brainfile1="brain1.txt";


    int32_t random_repeats=60;//How many times do we repeat random searches in the vicinity of the head (only when using gains method)



    //Weights used for the gain optimization
    std::array<int8_t,6> gain_weights={1,2,4,6,8,12};

    for (int i = 1; i < argc; ++i)
    {
        if (0 == std::string(argv[i]).compare("-m") && i+1<argc)
        {
            matches=atoi(argv[i+1]);
            i+=1;
        }
        if (0 == std::string(argv[i]).compare("-d") && i+1<argc)
        {
            depth=atoi(argv[i+1]);
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-o") &&i+1<argc)
        {
            outname=argv[i+1];
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-bf0") &&i+1<argc)
        {
            brainfile0=argv[i+1];
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-bf1") &&i+1<argc)
        {
            brainfile1=argv[i+1];
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-weights") &&i+6<argc)
        {
            gain_weights[0]=atoi(argv[i+1]);
            gain_weights[1]=atoi(argv[i+2]);
            gain_weights[2]=atoi(argv[i+3]);
            gain_weights[3]=atoi(argv[i+4]);
            gain_weights[4]=atoi(argv[i+5]);
            gain_weights[5]=atoi(argv[i+6]);
            i+=6;
        }
        else if (0 == std::string(argv[i]).compare("-repeats") &&i+1<argc)
        {
            random_repeats=atoi(argv[i+1]);
            ++i;

        }
    }




    std::ifstream brain0(brainfile0);
    std::ifstream brain1(brainfile1);

    Player players[] = {brain0.is_open() ? Player(true,brain0): Player(true),brain1.is_open() ? Player(true,brain1): Player(true)};

    if (brain0.is_open()) brain0.close();
    if (brain1.is_open()) brain1.close();

    auto getRoll=[&SingleDice,&rng]()->int8_t
    {
        int8_t roll=0;

        int8_t r = SingleDice(rng);
        roll+=r;

        r = SingleDice(rng);
        roll+=r;
        r = SingleDice(rng);
        roll+=r;
        r = SingleDice(rng);
        roll+=r;
        return roll;
    };


    auto Test =[&getRoll,game_start_state,game_start_unstarted,&players](const std::array<int8_t,465>& X,size_t matches) -> size_t
    {
        size_t player0wins=0;
        int32_t turns=0;


        players[0]=Player(true,X);
        for (size_t match = 0; match  < matches; ++match )
        {
            Game G=Game (match%2 == 0 ? game_start_state : game_start_state^turnFlag,game_start_unstarted);



            if ( playMatchBot(turns,players,G,getRoll))
                ++player0wins;
        }


        return player0wins;
    };

    struct timeval stop, start;
    gettimeofday(&start, NULL);

    std::array<int8_t,465> res;

    res=discrete_optimize_gains_all(Test,players[0].getBrain(),matches,depth,rng,random_repeats,outname,gain_weights);

    std::cout<<"Verified that we found score :"<<100*Test(res,matches)/double(matches)<<"% "<<std::endl;

    gettimeofday(&stop, NULL);

    std::ofstream output(outname+"_final.tsv");

    if (!output.is_open())
    {
        std::cout<<"Output file "<<outname<<"_final.tsv could not be opened for writing, can not proceed"<<std::endl;
        return 0;
    }

    for (int j = 0; j < 30; ++j)
    {
        for (int i = 0; i <= j; ++i)
        {
            std::string out = std::to_string((int)res[((j*(j+1))/2+i)]);
            output<<std::string(4-out.size(),' ')+out<<' ';
        }
        output<<std::endl;
    }

    std::cout<<"Done writing result to "<<outname<<"_final.tsv"<<std::endl;

    output.close();

    std::cout<<"Finished in "<< (stop.tv_sec - start.tv_sec)<<" seconds and returning 0"<<std::endl;

    return 0;
}
