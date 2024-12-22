#include "optimize.hpp"

#include<iostream>
#include<functional>
#include<thread>
#include<array>
#include<set>
#include<deque>
#include<map>
#include<cmath>
#include<fstream>

//Pick a number of elements, with these sum weights (where all weights sum to W), it is assumed we pick far fewer elements than there are
std::set<int> pick_element(const std::map<int,int>& weight_index,int picks,std::default_random_engine& rng,int W)
{
    std::uniform_int_distribution<int> roll(0,W);

    std::set<int> picked;
    for (int i = 0; i < picks; ++i)
    {
        //We want to pick unique elements, so repeat until we get something unique (WE ASSUME WE HAVE MUCH FEWER PICKS THAN THERE ARE ITEMS IN THE LIST)
        while (true)
        {
            //A little uggly, we want to find the highest element below or equal to our roll
            int pick = roll(rng);
            auto it = weight_index.lower_bound(pick);

            if (pick<it->first || it == weight_index.end())
                --it;

            if (picked.count(it->second)==0)
            {
                picked.insert(it->second);
                break;
            }
        }
    }
    return picked;
}


//For printing all the progress correctly
size_t offset_matches=0;
int total_depth=0;


struct timeval start;

std::array<int8_t,465> discrete_optimize_gains_all(std::function<size_t(std::array<int8_t,465>,size_t)> F, std::array<int8_t,465> x0, size_t matches,size_t depth, std::default_random_engine& rng, int32_t random_repeats,const std::string& outname,  std::array<int8_t,6> weights,  bool verbose)
{

    gettimeofday(&start, NULL);

    offset_matches=0;
    total_depth=0;
    //First optimize the diagonal, we are going to do this 3 times, and pick the best, this is so important that it is well worth it.
    std::cout<<"Diagonal optimization 1/3"<<std::endl;
    std::array<int8_t,465> x1 =discrete_optimize_gains(F, x0, matches,depth*4/*The diagonal is the most important by far, so devote more processing power to that*/, rng, random_repeats, weights, {0,  2,  5,  9, 14, 20, 27, 35, 44, 54, 65, 77, 90,104,119,135,152,170,189,209,230,252,275,299,324,350,377,405,434,464}, verbose);

    offset_matches=0;
    total_depth=0;
    std::cout<<"Diagonal optimization 2/3"<<std::endl;
    std::array<int8_t,465> x2 =discrete_optimize_gains(F, x0, matches,depth*4/*The diagonal is the most important by far, so devote more processing power to that*/, rng, random_repeats, weights, {0,  2,  5,  9, 14, 20, 27, 35, 44, 54, 65, 77, 90,104,119,135,152,170,189,209,230,252,275,299,324,350,377,405,434,464}, verbose);

    offset_matches=0;
    total_depth=0;
    std::cout<<"Diagonal optimization 3/3"<<std::endl;
    std::array<int8_t,465> x3 =discrete_optimize_gains(F, x0, matches,depth*4/*The diagonal is the most important by far, so devote more processing power to that*/, rng, random_repeats, weights, {0,  2,  5,  9, 14, 20, 27, 35, 44, 54, 65, 77, 90,104,119,135,152,170,189,209,230,252,275,299,324,350,377,405,434,464}, verbose);

    std::cout<<"Testing"<<std::endl;
    size_t val_0 = F(x1,matches);
    size_t val_1 = F(x2,matches);
    size_t val_2 = F(x3,matches);
    std::array<int8_t,465> x;
    if (val_0>val_1 && val_0>val_2)
    {
        x=x1;
    }
    else if (val_1>val_0 && val_1>val_2)
    {
        x=x2;
    }
    else
    {
        x=x3;
    }

    //Now optimize every column, one at the time
    for (int column = 0; column<30; ++column)
    {
        std::cout<<"Optimizing column "<<column<<"/30"<<std::endl;
        std::vector<int> ID;
        for (int row = column+1; row<30; ++row)
            ID.push_back(((row*(row+1))/2+column));
        std::cerr<<"\n\n"<<std::flush;
        x =discrete_optimize_gains(F, x, matches,depth, rng, random_repeats, weights, ID, verbose);


        std::cout<<"Saving Current brain"<<std::endl;

        std::ofstream output(outname+"_col"+std::to_string(column)+".tsv");

        if (!output.is_open())
        {
            std::cout<<"Output file "<<outname<<"_col"<<column<<".tsv could not be opened for writing, can not save "<<std::endl;
        }
        else
        {

            for (int j = 0; j < 30; ++j)
            {
                for (int i = 0; i <= j; ++i)
                {
                    std::string out = std::to_string((int)x[((j*(j+1))/2+i)]);
                    output<<std::string(4-out.size(),' ')+out<<' ';
                }
                output<<std::endl;
            }

            std::cout<<"Done writing result to "<<outname<<"_col"<<column<<".tsv "<<std::endl;

            output.close();
        }

    }


    return x;
}


std::array<int8_t,465> discrete_optimize_gains(std::function<size_t(std::array<int8_t,465>,size_t)> F, std::array<int8_t,465> x0,  size_t matches,size_t max_depth, std::default_random_engine& rng, int32_t random_repeats, std::array<int8_t,6> weights,std::vector<int> IDs,  bool verbose)
{
    /*Semi-random gains search
    check and add all neighbours like thorough depth-first search, use these to calculate the gain in all directions.

    Then add and check additional neighbours randomly, based on how close this direction, is to the greatest gain,*/

    int optimize_elements = IDs.size();

    struct timeval now;


    if (verbose)
        std::cout<<"Discrete optimization with gains-based random search with "<<random_repeats<<" random searches, with these weights "<<std::flush<<weights[0]<<' '<<weights[1]<<' '<<weights[2]<<' '<<weights[3]<<' '<<weights[4]<<' '<<weights[5]<<std::endl;

    size_t depth=0;
    //Should we, check this direction? this is based on the gains in this direction
    std::uniform_int_distribution<int> stepper(0,255);

    //I want ALL THE THREADS
    uint64_t max_threads = std::thread::hardware_concurrency();

    if (max_threads==0)
    {
        std::cout<<"WARNING, could not get number of available cores, running single threaded"<<std::endl;
        max_threads=1;
    }

    if (verbose)
        std::cout<<"Running in "<<max_threads<<" threads "<<std::endl;

    auto F_threaded = [&F](size_t* out, std::array<int8_t,465>* room, size_t matches) -> void
    {
        (*out) = F(*room,matches);
    };

    //Visited, a well as un-visited "rooms"
    std::deque< std::pair<std::array<int8_t,465>,double > > all_rooms;



    //Same, but get the id if it already exists
    auto try_add_or_get = [&all_rooms](std::array<int8_t,465>&& R,size_t& id)
    {
        //Checking already added elements is slow, but not as slow as the function F itself
        for (size_t I = 0; I < all_rooms.size(); ++I)
        {
            std::array<int8_t,465>& r = all_rooms[I].first;
            bool match = true;
            for (uint16_t i = 0; i < 465; ++i)
                if (r[i]!=R[i])
                {
                    match=false;
                    break;
                }
            if (match)
            {
                id=I;
                return;
            }
        }
        id =all_rooms.size();
        all_rooms.push_back(std::make_pair(std::move(R),0.0));
    };

    double best_so_far = 0;
    double best_so_far_err = 0;
    size_t best_room=0;
    size_t total_matches = 0;

    //A function for running matches on a target room, for as many times as it takes to tell if this room is better or worse than the best ever
    auto compare=[matches,max_threads,&best_so_far, &best_so_far_err  ,&F_threaded,verbose](std::array<int8_t,465>* room_ptr,double* err, double* score,size_t* this_matches) -> void
    {
        size_t wins =0;

        //Use as many threads as possible, to run all the games concurrently
        std::vector<size_t> sub_wins (max_threads,0);
        std::vector<std::thread> myThreads(max_threads);

        //Repeat, until we are certain this is/isn't the best ever
        *score=0;
        *err=0;
        *this_matches=0;
        size_t my_matches=matches;
        uint8_t extra_level=0;
        do
        {

            for (uint64_t j=0; j<max_threads; j++)
            {
                myThreads[j]=std::thread(F_threaded,&sub_wins[j],room_ptr,( j==0 ? matches-(matches/max_threads)*(max_threads-1) : matches/max_threads));
            }

            for (uint64_t j=0; j<max_threads; j++)
            {
                myThreads[j].join();
                wins+=sub_wins[j];
            }

            *score = double(wins)/my_matches;
            *err =sqrt((*score)*(1-(*score))/my_matches);

            //If this could be the next best, or if it is above, we need to be certain
            if (std::abs(*score-best_so_far)>(*err)+best_so_far_err   && (*score<=best_so_far || extra_level>50))
            {
                break;
            }
            ++extra_level;
            my_matches+=matches;
        }
        while(extra_level<100);

        this_matches+=my_matches;
    };




    //Those things we wish to check out, ordered by the win-rate of their parent room
    std::multimap< double,const std::array<int8_t,465>* > to_check_out;

    size_t this_matches;
    compare(&x0,&best_so_far_err,&best_so_far,&this_matches);
    total_matches+=this_matches;

    std::cerr<<offset_matches<<'\t'<<total_depth<<'\t'<<best_so_far<<'\t'<<best_so_far<<'\t'<<best_so_far_err<<'\t'<<0<<'\t'<<0.0<<std::endl;




    std::deque<std::pair<double,double> > prev_best_and_err;
    prev_best_and_err.push_back(std::make_pair(best_so_far,best_so_far_err));

    all_rooms.push_back({x0,best_so_far});
    to_check_out.insert({-(best_so_far),&(all_rooms[0].first)});


    std::vector<double> gains(optimize_elements*2);//Gains in each direction, i*2 is the gain in the + direction in dimension i, i*2+1
    std::vector<size_t> neighbour_id(optimize_elements*2);//ID of all the neighbours in the all_rooms, keep in mind we may be neighbouring a room we already checked

    bool random_best=false;//Was the best found using random search

    size_t failed_steps=0;
    while (!to_check_out.empty() && depth++<max_depth)
    {
        bool we_added_best=false;
        auto this_it =to_check_out.begin();
        double this_score=-this_it->first;

        std::array<int8_t,465> this_room = *this_it->second;


        if (verbose)
        {
            std::cout<<"Check out room with score "<<-this_it->first<<" best ever "<<best_so_far<<" ± "<<best_so_far_err<<std::endl;


            gettimeofday(&now, NULL);



            std::cerr<<(total_matches+offset_matches)<<'\t'<<((int)depth+total_depth)<<'\t'<<-this_it->first<<'\t'<<best_so_far<<'\t'<<best_so_far_err<<'\t'<<(int)random_best<<'\t'<<(now.tv_sec - start.tv_sec)<<std::endl;
        }

        size_t current_top_of_rooms=all_rooms.size();
        for (int i = 0; i < optimize_elements; ++i)
        {
            //Increment or decrement this element by 1
            {
                std::array<int8_t,465> new_room = this_room;
                ++new_room[IDs[i]];
                try_add_or_get(std::move(new_room),neighbour_id[2*i]);
            }
            {
                std::array<int8_t,465> new_room = this_room;
                --new_room [IDs[i]];
                try_add_or_get(std::move(new_room),neighbour_id[2*i+1]);
            }
        }


        //Erase this element, before any new elements throw this off
        to_check_out.erase(this_it);

        double max_gain=0;
        //Now check all neighbouring directions, and
        for (int i = 0; i < optimize_elements*2; ++i)
        {
            if (neighbour_id[i]<current_top_of_rooms)
            {
                //Already calculated it previously
                gains[i]=all_rooms[neighbour_id[i]].second-this_score;

            }
            else
            {
                double err=0;
                double score =0;
                size_t this_matches;
                compare(&all_rooms[neighbour_id[i]].first,&err,&score,&this_matches);
                total_matches+=this_matches;
                all_rooms[neighbour_id[i]].second=score;

                gains[i]=all_rooms[neighbour_id[i]].second-this_score;

                if (verbose)
                    std::cout<<" Add neighbour "<<neighbour_id[i]<<"/"<<all_rooms.size()<<" with "<<score*100<<"%±"<<err*100<<'%'<<std::endl;

              //   if (verbose)
              //      std::cout<<'#'<<std::flush;

                to_check_out.insert({-score,&(all_rooms[neighbour_id[i]].first)});

                if (score>best_so_far)
                {
                    best_room=neighbour_id[i];
                    failed_steps=0;
                    if (verbose)
                        std::cout<<"New best "<<score<<" > "<<best_so_far<<" ("<<best_room<<") "<<std::endl;
                    best_so_far = score;
                    best_so_far_err=err;

                    we_added_best=true;
                    random_best  = false;
                }
            }

            max_gain = std::max(max_gain,gains[i]);
        }

        //As preparation for adding random neighbours
        std::vector<int> dimension_weight(optimize_elements*2);
     //   if (verbose)
     //       std::cout<<"Gains, from current score "<<this_score<<std::endl;

        //Summed Weights of each direction
        int W=0;
        std::map<int,int> weight_index;

        for (int i = 0; i < optimize_elements; ++i)
        {
            if (max_gain<0)//If we are at a peak, go fully random
            {
                dimension_weight[2*i]=4;
                weight_index.insert({W,2*i});
                W+=4;
            }
            else if (gains[i*2]>max_gain*0.75)
            {
                dimension_weight[2*i]=weights[5]; //12
                weight_index.insert({W,2*i});
                W+=weights[5];
            }
            else if (gains[i*2]>max_gain*0.5)
            {
                dimension_weight[2*i]=weights[4]; //8
                weight_index.insert({W,2*i});
                W+=weights[4];
            }
            else if (gains[i*2]>max_gain*0.25)
            {
                dimension_weight[2*i]=weights[3]; //6
                weight_index.insert({W,2*i});
                W+=weights[3];
            }
            else if (gains[i*2]>max_gain*0.125)
            {
                dimension_weight[2*i]=weights[2]; //4
                weight_index.insert({W,2*i});
                W+=weights[2];
            }
            else if (gains[i*2]>0)
            {
                dimension_weight[2*i]=weights[1]; //2
                weight_index.insert({W,2*i});
                W+=weights[1];
            }
            else
            {
                dimension_weight[2*i]=weights[0]; //1
                weight_index.insert({W,2*i});
                W+=weights[0];
            }

            if (max_gain<0)
            {
                dimension_weight[1+2*i]=4;
                weight_index.insert({W,1+2*i});
                W+=4;
            }
            else if (gains[1+i*2]>max_gain*0.75)
            {
                dimension_weight[1+2*i]=weights[5];
                weight_index.insert({W,1+2*i});
                W+=weights[5];
            }
            else if (gains[1+i*2]>max_gain*0.5)
            {
                dimension_weight[1+2*i]=weights[4];
                weight_index.insert({W,1+2*i});
                W+=weights[4];
            }
            else if (gains[1+i*2]>max_gain*0.25)
            {
                dimension_weight[1+2*i]=weights[3];
                weight_index.insert({W,1+2*i});
                W+=weights[3];
            }
            else if (gains[1+i*2]>max_gain*0.125)
            {
                dimension_weight[1+2*i]=weights[2];
                weight_index.insert({W,1+2*i});
                W+=weights[2];
            }
            else if (gains[1+i*2]>0)
            {
                dimension_weight[1+2*i]=weights[1];
                weight_index.insert({W,1+2*i});
                W+=weights[1];
            }
            else
            {
                dimension_weight[1+2*i]=weights[0];
                weight_index.insert({W,1+2*i});
                W+=weights[0];
            }


            //if (verbose)
            //    std::cout<<'['<<(int)i<<"]: + from: "<<neighbour_id[i*2]<<" "<<(int)dimension_weight[i*2]<<std::endl;




            //if (verbose)
            //    std::cout<<'['<<(int)i<<"]: - from: "<<neighbour_id[i*2+1]<<" "<<(int)dimension_weight[i*2+1]<<std::endl;

        }

        //Let us just decide to add as many random points, as there are directions we can go in, because why not:


         if (verbose)
            std::cout<<std::endl;
        current_top_of_rooms=all_rooms.size();

        //add between 20 and 5 modificitions, based on the random weights (but not more than there are elements)
        std::uniform_int_distribution<int> modifications(0,std::min(20,optimize_elements/2));
        std::vector<int> Picks;
        for (int16_t i = 0; i < optimize_elements*2*random_repeats; ++i)
        {

            std::array<int8_t,465> new_room = this_room;
            int picks = modifications(rng);
            std::set<int> picked =pick_element(weight_index,picks,rng,W);
            for (int id : picked)
            {
                if (id%2==0)
                {
                    ++new_room[IDs[id/2]];
                }
                else
                {
                    --new_room[IDs[id/2]];
                }
            }

            /*
            for (int j = 0; j < 465; ++j)
            {
                {
                    if (stepper(rng)<dimension_weight[2*j])
                    {
                        ++new_room[j];
                        //std::cout<<"Randomly added in dir "<<(int)j<<' '<<(int)dimension_weight[2*j]<<"/16"<<std::endl;
                    }

                }
                {
                    if (stepper(rng)<dimension_weight[2*j+1])
                    {
                        --new_room[j];
                        //std::cout<<"Randomly subtracted in dir "<<(int)j<<' '<<(int)dimension_weight[2*j+1]<<"/16"<<std::endl;
                    }
                }
            }
            */
            size_t temp=0;//Nevermind if we have been there before
            try_add_or_get(std::move(new_room),temp);
            if (temp+1==all_rooms.size())
                Picks.push_back(picks);
        }
        //std::cout<<current_top_of_rooms<<"=="<<all_rooms.size()<<std::endl;
        for (size_t i = current_top_of_rooms; i<all_rooms.size();++i)
        {
            double err=0;
            double score=0;
            size_t this_matches;


            compare(&all_rooms[i].first,&err,&score,&this_matches);
            total_matches+=this_matches;
            all_rooms[i].second=score;

         //if (verbose)
        //    std::cout<<'*'<<std::flush;
            if (verbose)
                std::cout<<" From random gain, add "<<i<<"/"<<all_rooms.size()<<" with "<<score*100<<"%±"<<err*100<<"% with "<<Picks[i-current_top_of_rooms]<<std::endl;
            to_check_out.insert({-score,&(all_rooms[i].first)});

            if (score>best_so_far)
            {
                failed_steps=0;
                best_room   = i;
                if (verbose)
                    std::cout<<"New best "<<score<<" > "<<best_so_far<<" ("<<best_room<<") "<<std::endl;
                best_so_far = score;
                best_so_far_err=err;

                we_added_best=true;
                random_best =true;
            }
        }

         if (verbose)
            std::cout<<std::endl;


        if (we_added_best)
        {
            failed_steps=0;
            if (prev_best_and_err.size()>2 && std::abs(best_so_far-prev_best_and_err[prev_best_and_err.size()-3].first)<prev_best_and_err[prev_best_and_err.size()-3].second+best_so_far_err)
            {
                std::cout<<"The past 3 improvements were within error, this is likely a plateau BREAK"<<std::endl;
                break;
            }
            prev_best_and_err.push_back(std::make_pair(best_so_far,best_so_far_err));
        }
        else
        {
            if (failed_steps>=max_depth/3)
            {
                std::cout<<"Spent "<<failed_steps<<" without any improvements, this is likely a plateau BREAK"<<std::endl;
                break;
            }
            ++failed_steps;
        }
    }

    if (verbose)
    {

        double err=0;
        double score=0;
        size_t this_matches;
        compare(&all_rooms[best_room].first,&err,&score,&this_matches);
        total_matches+=this_matches;

        std::cout<<"Returning best "<<best_so_far<<"="<<all_rooms[best_room].second<<" ("<<best_room<<"), verified score: ("<<score<<") "<<std::endl;
    }





    offset_matches+=total_matches;
    total_depth+=max_depth;

    gettimeofday(&now, NULL);
    std::cerr<<(offset_matches)<<'\t'<<((int)total_depth)<<'\t'<<best_so_far<<'\t'<<best_so_far<<'\t'<<best_so_far_err<<'\t'<<(int)random_best<<'\t'<<(now.tv_sec - start.tv_sec)<<std::endl;



    return all_rooms[best_room].first;
}
