#include<iostream>
#include<string>
#include<random>
#include<fstream>
#include<armadillo>
#include<sys/time.h>
#include<thread>

#include"Game.hpp"
#include"Player.hpp"
#include"playMatch.hpp"

int main(int argc, char* argv[])
{
    std::default_random_engine rng(time(NULL));

    std::uniform_int_distribution<int> SingleDice(0,1);


    int32_t game_start_state = 0b0000000000000000000000000000;
    int32_t game_start_unstarted = 0b01110111;

    int32_t matches=5000;//In multi-match mode, multiple matches are run and the results are tallied up
    //int32_t turns=0;//For measuring performance of training scenarios (will be quite high when a human is involved)


    std::string brainfile0="brain0.txt";
    std::string brainfile1="brain1.txt";

    int xi = 7;
    int xj = 7;

    int yi = 7;
    int yj = 19;

    bool x_set= false;
    bool y_set= false;

    float x_min=-1;
    float x_max=-1;
    float y_min=-1;
    float y_max=-1;

    for (int i = 1; i < argc; ++i)
    {
        if (0 == std::string(argv[i]).compare("-m") && i+1<argc)
        {
            matches=atoi(argv[i+1]);
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-y") && i+4<argc)
        {
            yi=atoi(argv[i+1]);
            yj=atoi(argv[i+2]);
            y_min=atoi(argv[i+3]);
            y_max=atoi(argv[i+4]);
            y_set=true;
            i+=4;
        }
        else if (0 == std::string(argv[i]).compare("-x") && i+4<argc)
        {
            xi=atoi(argv[i+1]);
            xj=atoi(argv[i+2]);
            x_min=atoi(argv[i+3]);
            x_max=atoi(argv[i+4]);
            x_set=true;
            i+=4;
        }
        else if (0 == std::string(argv[i]).compare("-bf0") &&i+1<argc)
            brainfile0=argv[i+1];
        else if (0 == std::string(argv[i]).compare("-bf1") &&i+1<argc)
            brainfile1=argv[i+1];
    }

    std::ifstream brain0(brainfile0);
    std::ifstream brain1(brainfile1);

    Player players[] = {brain0.is_open() ? Player(true,brain0): Player(true),brain1.is_open() ? Player(true,brain1): Player(true)};

    if (brain0.is_open()) brain0.close();
    if (brain1.is_open()) brain1.close();

    if (!x_set)
    {
        float x_val = players[0].getBrainWeight(xi,xj);
        x_min=x_val*0.5;
        x_max=x_val*1.5;
    }

    if (!y_set)
    {
        float y_val = players[0].getBrainWeight(yi,yj);
        y_min=y_val*0.5;
        y_max=y_val*1.5;
    }


    //int32_t player0wins=0;
    struct timeval stop, start;
    gettimeofday(&start, NULL);


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

    auto Test =[&getRoll,game_start_state,game_start_unstarted,matches,&players,xi,xj,yi,yj](float x, float y, float* out) -> void
    {
        int32_t player0wins=0;
        int32_t turns=0;


        players[0].setBrainWeight(xi,xj,x);
        players[0].setBrainWeight(yi,yj,y);
        for (int32_t match = 0; match  < matches; ++match )
        {
            Game G=Game (match%2 == 0 ? game_start_state : game_start_state^turnFlag,game_start_unstarted);



            if ( playMatchBot(turns,players,G,getRoll))
                ++player0wins;
        }


        *out = float(player0wins)/matches;


    };




    //I want ALL THE THREADS
    uint64_t max_threads = std::thread::hardware_concurrency();






    for (float x = x_min; x<=x_max; x+=(x_max-x_min)/64)
        std::cout<<'\t'<<x;
    std::cout<<std::endl;
    int n = 0;
    for (float y = y_min; y<=y_max; y+=(y_max-y_min)/64)
    {
        std::cout<<y;
        for (float x = x_min; x<x_max; x+=max_threads*(x_max-x_min)/64)
        {


            std::vector<std::thread> myThreads(max_threads);
            std::vector<float> out(max_threads);
            for (uint64_t j=0; (j<max_threads ); j++){
                myThreads[j]=std::thread(Test,x+j*(x_max-x_min)/64,y,&out[j]);
            }

            for (uint64_t j=0; (j<max_threads ); j++){
                myThreads[j].join();
                std::cout<<'\t'<<out[j]<<std::flush;
            }
            n+=4;
            std::cerr<<n<<'/'<<64*64<<std::endl;
        }
        std::cout<<std::endl;
    }

/*
    for (int32_t match = 0; match  < matches; ++match )
    {
        G=Game (match%2 == 0 ? game_start_state : game_start_state^turnFlag,game_start_unstarted);

        if (!graphics && matches>1)
        {
            //Simple progress bar
            for (int8_t i = 0; i < 41; ++i)
                if (match+1>=(matches*i)/40 && match < (matches*i)/40)
                    std::cout<<'#'<<std::flush;
        }
        if (graphics)
        {
            if ( playMatch(graphics,turns,players,G,getRoll))
                ++player0wins;
        }
        else
        {
            if ( playMatchBot(turns,players,G,getRoll))
                ++player0wins;
        }
    }
*/



    gettimeofday(&stop, NULL);
 //   long unsigned int microseconds = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;

 //   std::cout<<"We played "<<matches<<" matches and "<<turns<<" turns in "<<microseconds/1000000.0 <<" s , with "<<((float)microseconds)/turns<<" μs per turn and "<<((float)microseconds)/matches<<" μs per match "<<std::endl;

    return 0;
}
