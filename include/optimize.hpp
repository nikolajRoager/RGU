#pragma once


#include<array>
#include<functional>
#include<thread>
#include<random>
#include<string>



std::array<int8_t,465> discrete_optimize_gains(std::function<size_t(std::array<int8_t,465>,size_t)> F, std::array<int8_t,465> x0, size_t matches,size_t depth, std::default_random_engine& rng, int32_t random_repeats, std::array<int8_t,6> weights ={1,2,8,16,30,50},std::vector<int> IDs ={0,  2,  5,  9, 14, 20, 27, 35, 44, 54, 65, 77, 90,104,119,135,152,170,189,209,230,252,275,299,324,350,377,405,434,464},  bool verbose=true);


std::array<int8_t,465> discrete_optimize_gains_all(std::function<size_t(std::array<int8_t,465>,size_t)> F, std::array<int8_t,465> x0, size_t matches,size_t depth, std::default_random_engine& rng, int32_t random_repeats,const std::string& outname, std::array<int8_t,6> weights ={1,2,8,16,30,50},  bool verbose=true);
