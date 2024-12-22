#pragma once

//This IS really a C library I made for an earlier project, the only change I have made is replace a C-style function pointer with a C++ std::function, because that is the only way I can get it to work with this C++ project

//I KNOW it is UGGLY and EVIL to mix C++ and C, but I see no point in rewriting all this working C library into C++

#include<functional>


void twon_backet(std::function<bool(size_t,size_t)> match,size_t* player_ID,size_t N, size_t n);
