#include<iostream>
#include<string>
#include<random>
#include<fstream>
#include<armadillo>
#include<complex>
#include<cmath>

//An example optimization of the gamma function, offset by a binomial distribution

std::complex<double> stirling(std::complex<double> z)
{

    if (z.real()==0 && z.imag() ==0)//NOT A BUG, I only get NaN if z is EXACTLY 0, even if it is 0.000001 it works fine, since the assymptote at 0 is 1
        return 1;
    //The approximation does NOT work for complex numbers Stirlings approximation do yes do work, but the greater than or less than operators, needed to bring this in range, do not
    //We still need to ... somehow ... remap this to be assymptotical, ut what does that mean? gues assymptotical in Re only

    //We do have do that Γ(x-i y)=Γ(x+i y)

    //After this we have something positive
    if(z.real()<0)return M_PI/sin(M_PI*z)/stirling(std::complex<double>({1,0})-z);


    if(z.real()<9)return stirling(z+std::complex<double>({1,0}))/z;

    std::complex<double> lngamma=z*log(z+std::complex<double>({1,0})/(std::complex<double>({12,0})*z-std::complex<double>({1,0})/z/std::complex<double>({10,0})))-z+log(std::complex<double>({2*M_PI,0})/z)/std::complex<double>({2,0});

    return exp(lngamma);
}


int F(double x, double y,std::default_random_engine& rng,int n)
{

    double p = 1/(1+exp(-std::abs(stirling({x,y}))));
    std::binomial_distribution<int> dist(n,p);

    return dist(rng);
}


int main(int argc, char* argv[])
{
    std::default_random_engine rng(time(NULL));



    float x_min = -4;
    float x_max =  4;

    float y_min = -4;
    float y_max =  4;

    int n = (argc>=2 ? atoi(argv[1]) : 100);

    for (float x = x_min; x<=x_max; x+=(x_max-x_min)/64)
        std::cout<<'\t'<<x;
    std::cout<<std::endl;
    for (float y = y_min; y<=y_max; y+=(y_max-y_min)/64)
    {
        std::cout<<y;
        for (float x = x_min; x<x_max; x+=(x_max-x_min)/64)
        {


            std::cout<<'\t'<<(F(x,y,rng,n))<<std::flush;
        }
        std::cout<<std::endl;
    }


    return 0;
}

