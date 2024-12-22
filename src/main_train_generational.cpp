#include<iostream>
#include<string>
#include<random>
#include<fstream>
#include<filesystem>
#include<sys/time.h>

#include"Game.hpp"
#include"Player.hpp"
#include"playMatch.hpp"
#include"optimize.hpp"
#include"2n_elimination_bracket.hpp"

namespace fs = std::filesystem;


int main(int argc, char* argv[])
{
    std::default_random_engine rng(time(NULL));

    std::uniform_int_distribution<int> SingleDice(0,1);

    int32_t game_start_state = 0b0000000000000000000000000000;
    int32_t game_start_unstarted = 0b01110111;

    int32_t matches=20000;
    int32_t depth=50;

    fs::path outfolder="generational_training_out";

    //How many times do we repeat random searches in the vicinity of the head (only when using gains method)
    int32_t random_repeats=60;


    //Weights used for the gain optimization
    std::array<int8_t,6> gain_weights={1,2,4,6,8,12};

    int generations=4;
    int parent_brains=4;
    int log2parents = 2;

    std::vector<std::string> brainfiles={std::string("brain0.txt"),std::string("brain1.txt"),std::string("brain2.txt"),std::string("brain3.txt")};


    uint64_t max_threads = std::thread::hardware_concurrency();
    if (max_threads==0)
    {
        std::cout<<"WARNING, could not get number of available cores, running single threaded"<<std::endl;
        max_threads=1;
    }



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
            outfolder=std::string(argv[i+1]);
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-g") &&i+1<argc)
        {
            generations=atoi(argv[i+1]);
            i+=1;
        }
        else if (0 == std::string(argv[i]).compare("-brains") && i+1<argc)
        {
            parent_brains=atoi(argv[i+1]);
            //Check that this is a power of
            if ( !((parent_brains  & (parent_brains - 1)) == 0 && parent_brains!=0))
            {
                std::cout<<" Number of parent brains must be a power of 2, but got "<<parent_brains<<std::endl;
                return 1;
            }

            log2parents = 0;
            while ((1<<log2parents)<parent_brains) ++log2parents;
            std::cout<<" verified that 2^"<<log2parents<<"="<<parent_brains<<std::endl;

            i+=1;

            if (i+1+parent_brains>argc)
            {
                std::cout<<" asked for "<<parent_brains<<" parent brains each generation, without enough brain files"<<std::endl;
                return 1;
            }

            brainfiles=std::vector<std::string>(parent_brains);
            for (int j = 0; j < parent_brains; ++j)
                brainfiles[j]=argv[i+1+j];
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

    int total_candidates = parent_brains*parent_brains;

    std::vector<Player> parents;
    std::vector<Player> candidates;

    for (int i = 0; i < parent_brains; ++i)
    {
        std::ifstream brain(brainfiles[i]);
        if (!brain.is_open())
        {
            std::cout<<"Could not open "<<brainfiles[i]<<" for reading"<<std::endl;
            return 0;
        }
        parents.push_back(Player(true,brain));
        brain.close();
    }


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

    int current_target = 0;

    auto Test =[&getRoll,game_start_state,game_start_unstarted,&parents,&current_target](const std::array<int8_t,465>& X,size_t matches) -> size_t
    {
        size_t player0wins=0;
        int32_t turns=0;


        Player P(true,X);
        for (size_t match = 0; match  < matches; ++match )
        {
            Game G=Game (match%2 == 0 ? game_start_state : game_start_state^turnFlag,game_start_unstarted);



            if ( playMatchBot(turns,P,parents[current_target],G,getRoll))
                ++player0wins;
        }


        return player0wins;
    };


    //Have these two people duel one another, used for the tournament to find the most promising candidates for the breeding program, this writes to a pointer to be run multi-threaded
    auto Duel = [&getRoll,game_start_state,game_start_unstarted](Player* A, Player* B,size_t* A_wins,size_t matches) -> void
    {
        int32_t turns=0;
        (*A_wins)=0;
        for (size_t match = 0; match  < matches; ++match )
        {
            Game G=Game (match%2 == 0 ? game_start_state : game_start_state^turnFlag,game_start_unstarted);

            if ( playMatchBot(turns,*A,*B,G,getRoll))
                ++(*A_wins);
        }
    };

    for (int g = 0; g < generations; ++g)
    {

        candidates=std::vector<Player>();

        std::cout<<"Generation "<<g<<" out of "<<generations<<std::endl;
        for (int i = 0; i < parent_brains; ++i)
            for (int j = 0; j < parent_brains; ++j)
            {
                current_target=j;
                std::cout<<"\n\n===================================================="<<std::endl;
                std::cout<<"Training candidate "<<(i*parent_brains+j)<<" with parent "<<i<<" on parent "<<j<<std::endl;
                std::cout<<"===================================================="<<std::endl;


                std::array<int8_t,465> res;

                res=discrete_optimize_gains(Test,parents[i].getBrain(),matches,depth,rng,random_repeats,gain_weights);

                candidates.push_back(Player(true,res));

                std::cout<<"Verified that we found score :"<<100*Test(res,matches)/double(matches)<<"% "<<std::endl;
                std::cerr<<'\n'<<std::endl;
            }
        std::vector<size_t> candidates_ID(total_candidates);

        for (int i = 0; i < total_candidates;++i)
        {
            candidates_ID[i]=i;
            std::cout<<i<<' ';
        }
        std::cout<<std::endl;


        twon_backet(
        [&Duel,&candidates,&matches,&max_threads](size_t A, size_t B) -> bool
        {
            size_t wins =0;

            //Use as many threads as possible, to run all the games concurrently
            std::vector<size_t> sub_wins (max_threads,0);
            std::vector<std::thread> myThreads(max_threads);

            //Repeat, until we are certain one is better
            double score=0;
            size_t my_matches=matches;
            uint8_t extra_level=0;
            double err;


            std::cout<<"Player "<<A<<" vs "<<B<<std::flush;

            do
            {

                for (uint64_t j=0; j<max_threads; j++)
                {
                    myThreads[j]=std::thread(Duel,&candidates[A],&candidates[B],&sub_wins[j],( j==0 ? matches-(matches/max_threads)*(max_threads-1) : matches/max_threads));
                }

                for (uint64_t j=0; j<max_threads; j++)
                {
                    myThreads[j].join();
                    wins+=sub_wins[j];
                }

                score = double(wins)/my_matches;
                err =sqrt(score*(1-score)/my_matches);

                if (std::abs(score-0.5)>err*2)
                {
                    break;
                }
                std::cout<<"."<<std::flush;
                ++extra_level;
                my_matches+=matches;

            }
            while(extra_level<100);
            std::cout<<(score>0.5 ? A :B)<<" wins "<<std::endl;

            return score>0.5;
        }
        ,&(candidates_ID[0]),log2parents+log2parents,log2parents);

        for (int c = 0; c < parent_brains;++c)
        {
            std::cout<<candidates_ID[c]<<' ';
            fs::path outfile = outfolder/(std::string("brain_gen")+std::to_string(g)+"_"+std::to_string(c)+".tsv");

            std::ofstream output(outfile);

            if (!output.is_open())
            {
                std::cout<<"Output file "<<outfile.string()<<" could not be opened for writing, can not proceed"<<std::endl;
                return 0;
            }

            std::array<int8_t,465> res=candidates[candidates_ID[c]].getBrain();

            std::cout<<"SAVING "<<c<<"/"<<parent_brains<<std::endl;

            parents[c]=candidates[candidates_ID[c]];

            for (int j = 0; j < 30; ++j)
            {
                for (int i = 0; i <= j; ++i)
                {
                    std::string out = std::to_string((int)res[((j*(j+1))/2+i)]);
                    output<<std::string(4-out.size(),' ')+out<<' ';
                }
                output<<std::endl;
            }


            std::cout<<"Done writing result to "<<outfile<<std::endl;

            output.close();

        }
        std::cout<<std::endl;











 //       vector<size_t> next_parent_id = twoN_elimination();
       //size_t id =i*parent_brains+j
    }

    return 0;
}
