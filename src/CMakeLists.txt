# src/CMakeLists.txt
# A few class libraries
add_library(Game Game.cpp)
add_library(Player Player.cpp)
add_library(playMatch playMatch.cpp)
add_library(2n_elimination_bracket 2n_elimination_bracket.cpp)
add_library(SDL_functions SDL_functions.cpp)

# Define the executable
add_executable(RoyalGameUr main.cpp)

# Everyone get your headers from here
target_include_directories(SDL_functions PRIVATE ../include)
target_include_directories(RoyalGameUr PRIVATE ../include)
target_include_directories(Game PRIVATE ../include)
target_include_directories(Player PRIVATE ../include)
target_include_directories(playMatch PRIVATE ../include)
target_include_directories(2n_elimination_bracket PRIVATE ../include)

# Link libraries to main program
target_link_libraries(RoyalGameUr Game)
target_link_libraries(RoyalGameUr SDL_functions)
target_link_libraries(RoyalGameUr Player)
target_link_libraries(RoyalGameUr playMatch)
target_link_libraries(Player Game)

