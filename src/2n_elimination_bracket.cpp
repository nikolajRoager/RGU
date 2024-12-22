//This is mainly C code, because I first sketched this in C

/*
A 2^n elimination bracket for 2^N elements has 3 "steps": splitting, reduction, and finalization


Takes the 2^N elements, which must be a power of 2, greater than 2^n.

Splitting: a single split on 2^N elements has each neighbour play a match against each other (2^(N-1) matches), and splits them into 2 categories with 2^(N-1) elements
Repeat with each category set until there are 2^n separate categories with 2^(N-n) This takes 2^(N-1) matches first every step

Example 2^N=32 2^n=4, where the smaller number indicates the one which wins.

|00-01 02-03 04-05 06-07 08-09 10-11 12-13 14-15 16-17 18-19 20-21 22-23 24-25 26-27 28-29 30-31| Initial
|                                               |                                               |
|00 02 04 06 08 10 12 14 16 18 20 22 24 26 28 30|01 03 05 07 09 11 13 15 17 19 21 23 25 27 29 31| 1
|                       |                       |                       |                       |
|00 04 08 12 16 20 24 28|02 06 10 14 18 22 26 30|01 05 09 13 17 21 25 29|03 07 11 15 19 23 27 31| 2 (n)
|                       |                       |                       |                       |
| 0 losses              | 1 loss                | 2 losses              | 2^n-1=3 losses        |

After this we have our 2^n categories, of players which lost 0 ... 2^n-1 times

Then we reduce the 2^n categories, we start by looking at all categories, and matching all neighbours. The losers fall to the lower category, the winners stay (those who loose from the last category are cast into the outer darkness)

Repeat, but this time start with category 1, then category 2, until category 2^n-1, that is 2^n steps of reduction.
Now we again have 2^n-1 categories, but this time cut in half.

Continuing with the same example

Initial |00 04 08 12.16 20 24 28|02 06 10 14.18 22 26 30|01 05 09 13.17 21 25 29|03-07 11-15.19-23 27-31|
|       |           |                       |                       |                       | Outer darkness
0       |00 08 16 24|04 12 20 28.02 10 18 26|06 14 22 30.01 09 17 25|05 13 21 29.03 11 19 27|
|       |           |           |                       |                       |           |
1       |00 08 16 24|04 20 02 18|12 28 10 26 06 22 01 17|14 30 09 25 05 21 03 19|
|       |           |           |                       |                       |
2       |00 08 16 24|04 20 02 18|12 10 06 01.28 26 22 17|14 25 05 03.
|       |           |           |           |                       |
3       |00 08 16 24|04 20 02 18|12 10 06 01|28 22 14 05.           |

4 steps in total as 2^n=4

This is obviously repeated until the 2^(N-n) members have become 1, that is (N-m)*2^n steps

Going ahead with this example


Initial |00 04 08 12.16 20 24 28|02 06 10 14.18 22 26 30|01 05 09 13.17 21 25 29|03-07 11-15.19-23 27-31|
red 0   |           |                       |                       |                       | Outer darkness
0       |00 08 16 24|04 12 20 28.02 10 18 26|06 14 22 30.01 09 17 25|05 13 21 29.03 11 19 27|
|       |           |           |                       |                       |           |
1       |00 08 16 24|04 20 02 18|12 28 10 26 06 22 01 17|14 30 09 25 05 21 03 19|
|       |           |           |                       |                       |
2       |00 08 16 24|04 20 02 18|12 10 06 01.28 26 22 17|14 25 05 03.
|       |           |           |           |                       |
3       |00 08.16 24|04 20.02 18|12 10.06 01|28 22.14 03.           |
 red 1  |     |           |           |           |     |
 0      |00 16|08 24.04 02|20 18.10 01|12 06.22 03|
 |      |     |     |           |           |     |
 1      |00 16|08 02|24 04 10 01|20 18.06 03|
 |      |     |     |                 |     |
 2      |00 16|08 02|04 01.24 10 18 03|
 |      |     |     |     |           |
 3      |00.16|08.02|04.01|24.03.
red 2   |  |     |     |     |  |
0       |00|16.02|08.01|04.03|
|       |  |  |     |     |  |
1       |00|02|16.01|08.03|
|       |  |  |  |     |  |
2       |00|02|01|16.03|
|       |  |  |  |  |  |
3       |00|02|01|03|

This is repeated until each category has only has 1 member, these are the top 2^n elements

Finally, we could just match everyone against each other, to find the most promising, this can be done in (2^n-1)! matches, hopefully a lot less than (2^N-1)!
*/

//This is largely a C program, because I originally made it in C, I only import some basic C++ functions, to make this work with the rest of the program
#include<cstdint>
#include<cstdlib>
#include<cstdio>
#include<cstdbool>
#include"2n_elimination_bracket.hpp"

size_t matches=0;

void split(std::function<bool(size_t,size_t)> match,size_t* player_ID,size_t size,size_t* target_ID)
{
    for (size_t j = 0; j < size; j+=2)
    {

        if (match(player_ID[j],player_ID[j+1]))
        {
            target_ID[j/2]       =player_ID[j  ];
            target_ID[size/2+j/2]=player_ID[j+1];
        }
        else
        {
            target_ID[j/2]       =player_ID[j+1];
            target_ID[size/2+j/2]=player_ID[j  ];
        }
    }
    for (size_t j = 0; j < size; ++j)
    {
        player_ID[j]=target_ID[j];
    }
}

//Here we do not copy the data over, we merely use a workspace to re-arange it
void reduce(std::function<bool(size_t,size_t)> match,size_t* player_ID,size_t size,size_t categories,size_t* tmp_ID)
{
    for (size_t i = 0; i < categories; ++i)
    {
        split(match,&player_ID[size*i],size,&tmp_ID[size*i]);
    }

    for (size_t j = 0; j < size/2; ++j)
    {
        player_ID[j]=tmp_ID[j];
    }

    for (size_t i = 0; i+1 < categories; ++i)
    {
        //To fit with traditional 2^n eliminations bracket, mix the brackets
        //i.e. 0 1 2 3 4 5 6 7 -> 0 4 1 5 2 6 3 7
        for (size_t j = 0; j < size/2; ++j)
        {
            player_ID[size*i+j*2  +size/2]=tmp_ID[j  +size*i + size/2];
            player_ID[size*i+j*2+1+size/2]=tmp_ID[j  +size*i + size];
        }
    }
}

void twon_backet(std::function<bool(size_t,size_t)> match,size_t* player_ID,size_t N, size_t n)
{
    size_t two_n = 1<<n;
    size_t two_N = 1<<N;

    size_t *workspace = (size_t*)malloc(two_N*sizeof(size_t));

    printf("Initial              :");
    for (size_t i = 0; i < two_N; ++i)
    {
        if (i!=0)
            printf(" ");
        printf("%s%lu",player_ID[i]<10?"0":"",player_ID[i]);
    }
    printf("\n");

    //Splitting
    for (size_t i = 0; i < n; ++i)
    {
        printf("Splitting   (level %lu):",i);
        for (size_t j = 0; j < ((size_t)1<<i); ++j)
        {
            size_t splitting = two_N/(1<<i);
            split(match,&player_ID[splitting*j] ,two_N>>i, &workspace[splitting*j]);
        }
        for (size_t j = 0; j < two_N; ++j)
        {
            if (j!=0)
            {
                if (j%(two_N/(2<<i))==0)
                    printf("|");
                else
                    printf(" ");
            }
            printf("%s%lu",player_ID[j]<10?"0":"",player_ID[j]);
        }
        printf("\n");
    }

    //Reduction
    for (size_t reduction = 0; reduction < N-n; ++reduction)
    {
        size_t category_width = 1<<(N-n-reduction);

        for (size_t reduction_level = 0; reduction_level<two_n; ++reduction_level)
        {
            printf("Reduction %lu (level %lu):",reduction,reduction_level);

            reduce(match,&player_ID[reduction_level*(category_width>>1)] ,two_N>>(n+reduction),two_n-reduction_level, &workspace[reduction_level*(category_width>>1)]);

            //Loop through all categories for printing
            size_t id=0;
            for (size_t c = 0; c < two_n; ++c)
            {
                if (c!=0)
                    printf("|");
                for (size_t j = 0; j < (c<=reduction_level ? category_width>>1 : category_width); ++j)
                {
                    if (j!=0)
                        printf(" ");
                    printf("%s%lu",player_ID[id]<10?"0":"",player_ID[id]);
                    ++id;
                }
            }
            printf("\n");
        }
    }
    printf("%lu\n",matches);

    free(workspace);
}
