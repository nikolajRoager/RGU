//Graphical version, the game is displayed in a seperate window, the AI's reasoning is displayed on the terminal
#include<iostream>
#include<curses.h>
#include<string>
#include<random>
#include<fstream>
#include<deque>
#include<sys/time.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_mixer.h>

#include"Game.hpp"
#include"Player.hpp"
#include"playMatch.hpp"

#include"SDL_functions.hpp"

#define tile_center_start_x 33
#define tile_center_start_y 33

#define tile_size 64

#define BOT_DELAY 1000

int main(int argc, char* argv[])
{
    bool wait_for_bot = false;

    //Up to 2 bots allowed
    bool player_bot[]={false,false};

    std::string brainpath0="null";
    std::string brainpath1="null";

    std::ifstream brainstream0;
    std::ifstream brainstream1;

    bool save_replay=false;

    //Replay another game, this automatically loads the required brains, and sets everything to be bot-controlled, loads the rolls and verifies that the moves made by the bots match the moves made in the replay
    bool play_replay=false;

    std::ofstream replay_out;
    std::ifstream replay_in;

    bool graphics=true;

    uint32_t start_matches=1;

    for (int i = 1; i < argc; ++i)
    {
        if (0 == std::string(argv[i]).compare("-bf0") &&i+1<argc)
        {
            brainpath0 = argv[i+1];
            brainstream0=std::ifstream(brainpath0);
            if (!brainstream0.is_open())
            {
                std::cout<<"Could not open "<<argv[i+1]<<std::endl;
                if (brainstream1.is_open())
                    brainstream1.close();
                return 0;
            }
            player_bot[0]=true;
            i+=1;
            play_replay=false;
        }
        else if (0 == std::string(argv[i]).compare("-bf1") &&i+1<argc)
        {
            brainpath1 = argv[i+1];
            brainstream1=std::ifstream(brainpath1);
            if (!brainstream1.is_open())
            {
                std::cout<<"Could not open "<<argv[i+1]<<std::endl;
                if (brainstream1.is_open())
                    brainstream1.close();
                return 0;
            }
            player_bot[1]=true;
            i+=1;
            play_replay=false;
        }
        else if (std::string(argv[i]).compare("-m")==0 && i+1<argc)
        {
            graphics =false;
            player_bot[0]=true;
            player_bot[1]=true;
            start_matches=atoi(argv[i+1]);
            save_replay=false;
            play_replay=false;
            i+=1;

        }
        else if (std::string(argv[i]).compare("-save")==0 && i+1<argc)
        {
            save_replay=true;
            play_replay=false;
            replay_out=std::ofstream(argv[i+1]);
            i+=1;
            start_matches=1;
        }
        else if (std::string(argv[i]).compare("-replay")==0 && i+1<argc)
        {
            save_replay=false;
            play_replay=true;
            replay_in=std::ifstream(argv[i+1]);
            i+=1;
            start_matches=1;
        }
        else if (std::string(argv[i]).compare("-ng")==0)
        {
            graphics =false;
            player_bot[0]=true;
            player_bot[1]=true;
        }
        else if (std::string(argv[i]).compare("-wait")==0)
        {
            wait_for_bot =true;
        }
    }

    if (save_replay)
    {
        replay_out<<brainpath0<<std::endl;
        replay_out<<brainpath1<<std::endl;
    }
    if (play_replay)
    {
        replay_in>>brainpath0;
        replay_in>>brainpath1;

        if (brainpath0.compare("null"))
        {
            brainstream0=std::ifstream(brainpath0);
            if (!brainstream0.is_open())
            {
                std::cout<<"Could not open "<<brainpath0<<std::endl;
                if (brainstream0.is_open())
                    brainstream0.close();
                return 0;
            }
            player_bot[0]=true;
        }
        else
            player_bot[0]=false;

        if (brainpath1.compare("null"))
        {
            brainstream1=std::ifstream(brainpath1);
            if (!brainstream1.is_open())
            {
                std::cout<<"Could not open "<<brainpath1<<std::endl;
                if (brainstream1.is_open())
                    brainstream1.close();
                return 0;
            }
            player_bot[1]=true;
        }
        else
            player_bot[1]=false;
    }

    Player Bot[]={Player(true,brainstream0),Player(true,brainstream1)};

    if (brainstream0.is_open())
        brainstream0.close();
    if (brainstream1.is_open())
        brainstream1.close();

    //Start up SDL graphics

    std::cout<<"Init SDL"<<std::endl;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        std::cout<<"Could not initialize SDL2"<<std::endl;
        return 1;
    }


    std::cout<<"Init images"<<std::endl;
    if (!IMG_Init(IMG_INIT_PNG))
    {
        std::cout<<"Could not initialize PNG support"<< SDL_GetError()<<std::endl;
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {//If it did not work, return error
        std::cout<<"Could not initialize AUDIO support"<< SDL_GetError()<<std::endl;
        SDL_Quit();
        IMG_Quit();
        return 1;
    }

    uint32_t win_w = 1920;
    uint32_t win_h = 1000;

    //For seeing the size of the screen
    SDL_DisplayMode DM;

    std::cout<<"Update screen width height"<<std::endl;
    SDL_GetCurrentDisplayMode(0, &DM);
    win_w = DM.w;
    win_h = DM.h;

    std::cout << "Width " << win_w << " Height " << win_h << std::endl;

    uint FLAGS = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;

    SDL_Window* window = SDL_CreateWindow("I can write whatever I want here, and nobody can stop me", 0, 0, win_w, win_h, FLAGS);


    if (window == nullptr)
    {
        IMG_Quit();
        SDL_Quit();
        std::cout<<SDL_GetError()<<std::endl;
        return 1;
    }

    int tw, th;//this width and height I first used just w and h and got confused when the later setup of the framebuffer gave me waaay to high resolution
    SDL_GetWindowSize(window, &tw, &th);
    win_h = th;
    win_w = tw;

    //The window renderer
    SDL_Renderer* renderer = NULL;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if( renderer == nullptr)
    {
        std::cout<<"Could not init renderer: "<< SDL_GetError() <<std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Rect background_tile_size;
    SDL_Texture* background_tile = loadTexture("textures/background_tile.png",renderer,background_tile_size);

    SDL_Rect background_destination;
    SDL_Rect board_size;
    SDL_Texture* board_texture= loadTexture("textures/board.png",renderer,board_size);
    SDL_Rect board_destination;


    SDL_Rect menu_destination;
    SDL_Rect menu_size;
    SDL_Texture* menu_texture= loadTexture("textures/menu_tablet.png",renderer,menu_size);
    SDL_Texture* menu_shadow= loadTexture("textures/menu_tablet_shadow.png",renderer,menu_size);



    SDL_Rect roll_marker_size;
    SDL_Texture* roll_marker0_texture= loadTexture("textures/roll_marker_0.png",renderer,roll_marker_size);
    SDL_Texture* roll_marker1_texture= loadTexture("textures/roll_marker_1.png",renderer,roll_marker_size);

    SDL_Rect roll_marker_destination;


    SDL_Rect die_size;
    std::array<SDL_Texture*,6> die_texture= {loadTexture("textures/die0_0.png",renderer,die_size),loadTexture("textures/die0_1.png",renderer,die_size),loadTexture("textures/die0_2.png",renderer,die_size),loadTexture("textures/die1_0.png",renderer,die_size),loadTexture("textures/die1_1.png",renderer,die_size),loadTexture("textures/die1_2.png",renderer,die_size)};

    SDL_Texture* die_shadow=loadTexture("textures/die_shadow.png",renderer,die_size);

    SDL_Rect die_destination;


    //the board needs to fill 80% of the screens width, or one third of its height
    float scale_factor=std::min(0.8*win_w/board_size.w,0.5*win_h/board_size.h);


    die_destination.w=die_size.w*scale_factor;
    die_destination.h=die_size.h*scale_factor;

    roll_marker_destination.w=scale_factor*roll_marker_size.w;
    roll_marker_destination.h=scale_factor*roll_marker_size.h;
    roll_marker_destination.x=0;
    roll_marker_destination.y=(win_h-roll_marker_destination.h)/2;



    SDL_Rect piece_tile_size;

    //These got to be the same size
    SDL_Texture* piece0_texture = loadTexture("textures/piece0.png",renderer,piece_tile_size);
    SDL_Texture* piece1_texture = loadTexture("textures/piece1.png",renderer,piece_tile_size);

    SDL_Texture* piece_shadow_texture = loadTexture("textures/piece_shadow.png",renderer,piece_tile_size);


    SDL_Texture* select0_texture = loadTexture("textures/select0.png",renderer,piece_tile_size);
    SDL_Texture* select1_texture = loadTexture("textures/select1.png",renderer,piece_tile_size);


    piece_tile_size.x=0;
    piece_tile_size.y=0;

    SDL_Rect piece_destination_size;

    piece_destination_size.w=piece_tile_size.w*scale_factor;
    piece_destination_size.h=piece_tile_size.h*scale_factor;


    board_destination.w = board_size.w*scale_factor;
    board_destination.h = board_size.h*scale_factor;

    //Should be centered, but do make room for the width and height
    board_destination.y=(win_h-board_destination.h)/2;
    board_destination.x=(win_w-board_destination.w)/2;

    //It is either 1/2 height or 1/2 width
    float aspect_ratio = float(menu_size.h)/menu_size.w;

    if (win_w>win_h)
    {
        menu_destination.w=win_w/2.0;
        menu_destination.h=menu_destination.w*aspect_ratio;
    }
    else
    {
        menu_destination.h=win_h/2.0;
        menu_destination.w=menu_destination.h/aspect_ratio;
    }

    menu_destination.y=(win_h-menu_destination.h)/2;
    menu_destination.x=(win_w-menu_destination.w)/2;


    background_destination.w=background_tile_size.w*scale_factor;
    background_destination.h=background_tile_size.h*scale_factor;


    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x00 );


    //Load sounds
    Mix_Chunk* rattle_sound = Mix_LoadWAV("dice-shaking-in-hand.wav");
    int dice_channel=0;

    Mix_Chunk* roll_sound = Mix_LoadWAV("dice-roll_end.wav");

    Mix_Chunk* piece_sound = Mix_LoadWAV("gamepiece_short.wav");

    Mix_Chunk* menu_slide = Mix_LoadWAV("stone-sliding.wav");

    //For animating motion, get the x,y position in pixels on the board
    auto get_piece_pos = [&piece_tile_size](int location,bool player) -> std::pair<int,int>
    {
        int x=0;
        int y=0;

        if (location>0)
        {
            if (location>19)
            {
                x=(6*tile_size-(tile_size+piece_tile_size.w))*0.5+1.2*(location-16)*piece_tile_size.w;
                y= player ? -(tile_size+piece_tile_size.w)*0.5-0.8*piece_tile_size.w : 2*tile_size+(tile_size+piece_tile_size.w)*0.5+0.8*piece_tile_size.w;

            }
            else if (location>15)
            {
                x=6*tile_size-(tile_size+piece_tile_size.w)*0.5+1.2*(location-16)*piece_tile_size.w;
                y= player ? -(tile_size+piece_tile_size.w)*0.5 : 2*tile_size+(tile_size+piece_tile_size.w)*0.5;

            }
            else if (location==15)
            {
                x=21*tile_size-location*tile_size-(tile_size+piece_tile_size.w)*0.5;
                y=player ? 0 :2*tile_size;
            }
            else if (location<=4)
            {
                x=4*tile_size-location*tile_size;
                y=player ? 0*tile_size :2*tile_size;
            }
            else if (location>=13)
            {
                x=20*tile_size-location*tile_size;
                y=player ? 0 :2*tile_size;
            }
            else
            {
                x=location*tile_size-5*tile_size;
                y=1*tile_size;

            }
        }
        else if (location==0)
        {
            x=3*tile_size+(tile_size+piece_tile_size.w)*0.5;
            y=player ? 0*tile_size :2*tile_size;

        }
        else if (location<0)
        {
            y= player ? -(tile_size+piece_tile_size.w)*0.5 : 2*tile_size+(tile_size+piece_tile_size.w)*0.5;
            x= 3*tile_size +(1+location)*piece_tile_size.w+(tile_size+piece_tile_size.w)*0.5;
        }

        return std::make_pair(x,y);
    };




    auto display_piece = [&renderer,&board_destination,&piece1_texture,&piece0_texture, &piece_destination_size,&scale_factor](std::pair<int,int> pos,bool player) -> void
   {

        piece_destination_size.y=board_destination.y-piece_destination_size.h/2+(tile_center_start_y+pos.second)*scale_factor;
        piece_destination_size.x=board_destination.x-piece_destination_size.w/2+(tile_center_start_x+pos.first)*scale_factor;

        SDL_RenderCopy(renderer, player? piece1_texture : piece0_texture, NULL, &piece_destination_size);
    };

    bool quit = false;
    SDL_Event e;

    //Set up game
    int32_t game_start_state = 0b0000000000000000000000000000;
    int32_t game_start_unstarted = 0b01110111;
    Game G(game_start_state,game_start_unstarted);
    bool player = G.whoseTurn();

    std::default_random_engine rng(time(NULL));

    //For display, get the 4 dice, there are 6 pictures, 3 for 0 roll 3 for 1 roll
    std::uniform_int_distribution<int> SingleDice(0,5);
    std::uniform_real_distribution<float> dice_offset(-1.0,1.0);//The dice can be slightly offset or rotated

    //Each time the dice are rolled, the offset and angle of the dice cjamge
    std::array<float,4> offset_x={};
    std::array<float,4> offset_y={};
    std::array<float,4> angle={};

    std::array<int8_t,4> current_dice={};

    auto getRoll=[&current_dice,&SingleDice,&rng,&offset_x,&offset_y,&angle,&dice_offset,&die_size]()->int8_t
    {
        int8_t roll=0;

        for (int i = 0; i < 4; ++i)
        {
            int8_t r = SingleDice(rng);
            current_dice[i]=r;
            angle[i] = dice_offset(rng)*90-45;
            offset_x[i] = dice_offset(rng)*die_size.w*0.25;
            offset_y[i] = dice_offset(rng)*die_size.h*0.25;

            roll+=r>=3 ? 1 : 0;
        }

        return roll;
    };



    uint32_t animation_start=0;
    uint32_t menu_animation_end=0;
    uint32_t menu_animation_start=0;

    //All the states of the game shown to the screen
    enum gameState {WAIT_ROLL,HOLD_ROLL,TAKE_PIECE,MOVE_PIECE,WIN,HALT} state=WAIT_ROLL;

    int hide_piece_id=-8;//Hide this piece when displaying the board
    int move_id=0;
    int piece_target_id=-8;//Where is this piece supposed to go after the movement

    std::vector< std::pair<int8_t/*What pieces are involved in this move*/,Game> > LegalMoves;

    int mouse_x=0;
    int mouse_y=0;
    int p_mouse_x=0;
    int p_mouse_y=0;

    int mouseTravel=0;//How far has the mouse moved since we reset this counter


    bool mouseDown=false;


    int roll=0;

    std::deque<piece_animation> Animations;

    uint32_t millis = SDL_GetTicks();

    bool paused=false;

    uint32_t matches=start_matches;
    uint32_t player0Wins=0;

    uint32_t turn=0;

    while(!quit && matches!=0 )
    {

        millis = SDL_GetTicks();


        int tw, th;//this width and height I first used just w and h and got confused when the later setup of the framebuffer gave me waaay to high resolution
        SDL_GetWindowSize(window, &tw, &th);
        win_h = th;
        win_w = tw;


        //the board needs to fill 80% of the screens width, or one third of its height
        scale_factor=std::min(0.8*win_w/board_size.w,0.5*win_h/board_size.h);

        board_destination.w = board_size.w*scale_factor;
        board_destination.h = board_size.h*scale_factor;

        background_destination.w=background_tile_size.w*scale_factor+tile_center_start_y*scale_factor;
        background_destination.h=background_tile_size.h*scale_factor+tile_center_start_x*scale_factor;

        piece_destination_size.w=piece_tile_size.w*scale_factor;
        piece_destination_size.h=piece_tile_size.h*scale_factor;

        die_destination.w=die_size.w*scale_factor;
        die_destination.h=die_size.h*scale_factor;

        //Should be centered, but do make room for the width and height
        board_destination.y=(win_h-board_destination.h)/2;
        board_destination.x=(win_w-board_destination.w)/2;



        if (win_w>win_h)
        {
            menu_destination.h=win_h/2;
            menu_destination.w=menu_destination.h/aspect_ratio;
        }
        else
        {
            menu_destination.w=win_w;
            menu_destination.h=menu_destination.w*aspect_ratio;

        }

        menu_destination.y=(win_h-menu_destination.h)/2;
        menu_destination.x=(win_w-menu_destination.w)/2;



        die_destination.x=die_destination.w;
        die_destination.y=(win_h-board_destination.h)/2 +2.75*die_destination.h;//Start, 2 die above edge, where each die has 0.5 die betwixt themself


        roll_marker_destination.w=scale_factor*roll_marker_size.w;
        roll_marker_destination.h=scale_factor*roll_marker_size.h;
        roll_marker_destination.x=board_destination.x/2-roll_marker_destination.w/2;
        roll_marker_destination.y=(win_h-roll_marker_destination.h)/2;


        bool justClicked=false;
        bool justUnClicked=false;


        //Loop throuh all events
        while(SDL_PollEvent(&e) != 0)
            switch(e.type)
            {
            default: break;

            case SDL_QUIT:
                quit = true;
                break;

            case SDL_MOUSEBUTTONDOWN:
                justClicked=true;
                mouseDown=true;
                break;

            case SDL_MOUSEBUTTONUP:
                justUnClicked=true;
                mouseDown=false;
                break;

            case SDL_MOUSEMOTION:
                //Get mouse position
                p_mouse_x=mouse_x;
                p_mouse_y=mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                mouseTravel+=std::abs(mouse_x-p_mouse_x);
                mouseTravel+=std::abs(mouse_y-p_mouse_y);
                break;

            case  SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {

                    if ( (millis>menu_animation_end) )
                    {
                        menu_animation_start=millis;
                        menu_animation_end=millis+400;
                        if (!graphics)
                            menu_animation_end=menu_animation_start;
                        paused=!paused;
                        if (graphics)
                            dice_channel=Mix_PlayChannel(-1, menu_slide, 0);

                    }
                }


            }


        if (!paused && millis>menu_animation_end)
        {
            if (state==WAIT_ROLL && Animations.size()==0/*Let all shuffling end first*/)
            {
                if (!player_bot[player])
                {
                    //Pick up the dice by clicking the right place
                    if (justClicked && mouse_x>roll_marker_destination.x && mouse_y>roll_marker_destination.y && mouse_x<roll_marker_destination.w+roll_marker_destination.x && mouse_y<roll_marker_destination.h+roll_marker_destination.y)
                    {
                        state=HOLD_ROLL;

                        dice_channel=Mix_PlayChannel(-1, rattle_sound , 0);
                        mouseTravel=0;
                        animation_start=millis;
                    }
                }
                else
                {
                    //Pick up the dice by clicking the right place
                    if ((millis>animation_start+BOT_DELAY && (!wait_for_bot || justClicked)) || !graphics)
                    {
                        state=HOLD_ROLL;
                        dice_channel=Mix_PlayChannel(-1, rattle_sound , 0);
                        mouseTravel=0;
                        animation_start=millis;
                    }

                }

            }
            else if (state==HOLD_ROLL)
            {
                if (!player_bot[player])
                {
                    //Shaking doesn't matter we do one final roll regardless
                    if (!mouseDown && mouse_x>roll_marker_destination.x && mouse_y>roll_marker_destination.y && mouse_x<roll_marker_destination.w+roll_marker_destination.x && mouse_y<roll_marker_destination.h+roll_marker_destination.y)
                    {
                        if (play_replay)
                        {
                            std::string input;
                            replay_in>>input;//"roll" player "roll" (the text is just for my sake when reading the file, so ignore it)

                            if (input.compare("roll"))
                            {
                                std::cout<<"Error, command \""<<input<<"\" is not roll"<<std::endl;
                                state=HALT;
                                continue;
                            }

                            replay_in>>input;
                            replay_in>>input;
                            replay_in>>roll;
                        }
                        else
                        {
                            roll=getRoll();
                            if (save_replay)
                            {
                                replay_out<<"roll "<<player<<" roll "<<roll<<std::endl;
                            }
                        }

                        Mix_HaltChannel(dice_channel);
                        dice_channel= Mix_PlayChannel(-1, roll_sound , 0);


                        LegalMoves=G.getLegalMoves(player,roll);

                        animation_start=millis;
                        if (LegalMoves.size()==0)
                        {
                            state=WAIT_ROLL;
                            animation_start=millis;
                            player=!player;
                            G.flipTurn();


                            if (play_replay)
                            {
                                std::string input;

                                //TEMP, check that the Game matches what we load
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                            }
                            if (save_replay)
                            {
                                replay_out<<"turn "<<turn<<" game "<<G.getPlayerData()<<" "<<(int)G.getFinished()<<" "<<(int)G.getUnstarted()<<std::endl;
                            }
                            ++turn;

                        }
                        else
                        {
                            state=TAKE_PIECE;
                            animation_start=millis;
                        }
                    }
                    else if (mouseTravel>tile_size*scale_factor)
                    {
                        if (millis<animation_start+200 && graphics)//If we move that distance in less than 0.2 s, we rattle the dice
                        {
                            //Rattle-rattle

                            if (!Mix_Playing(dice_channel))
                            {
                                roll=getRoll();
                                dice_channel= Mix_PlayChannel(-1, rattle_sound , 0);
                            }


                        }
                        mouseTravel=0;
                        animation_start=millis;
                    }

                }
                else
                {
                    //Pick up the dice by clicking the right place
                    if (millis>animation_start+BOT_DELAY || !graphics)
                    {
                        if (play_replay)
                        {
                            std::string input;
                            replay_in>>input;

                            if (input.compare("roll"))
                            {
                                std::cout<<"Error, command \""<<input<<"\" is not roll"<<std::endl;
                                state=HALT;
                                continue;
                            }

                            //"roll" player "roll" (the text is just for my sake when reading the file, so ignore it)
                            replay_in>>input;
                            replay_in>>input;
                            replay_in>>roll;
                        }
                        else
                        {
                            roll=getRoll();
                            if (save_replay)
                            {
                                replay_out<<"roll "<<player<<" roll "<<roll<<std::endl;
                            }
                        }
                        Mix_HaltChannel(dice_channel);
                        if (graphics)
                            dice_channel= Mix_PlayChannel(-1, roll_sound , 0);

                        LegalMoves=G.getLegalMoves(player,roll);

                        animation_start=millis;
                        if (LegalMoves.size()==0)
                        {
                            state=WAIT_ROLL;
                            animation_start=millis;
                            player=!player;
                            G.flipTurn();

                            if (play_replay)
                            {
                                std::string input;

                                //TEMP, check that the Game matches what we load
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                            }
                            if (save_replay)
                            {
                                replay_out<<"turn "<<turn<<" game "<<G.getPlayerData()<<" "<<(int)G.getFinished()<<" "<<(int)G.getUnstarted()<<std::endl;
                            }

                            ++turn;
                        }
                        else
                        {
                            state=TAKE_PIECE;
                            animation_start=millis;
                        }


                    }

                }

            }
            else if (state==TAKE_PIECE)
            {
                if (!player_bot[player])
                {
                    if (justClicked)
                    {
                        animation_start=millis;
                        for (int i = 0; i <15; ++i)
                        {
                            std::pair<int,int> pos=get_piece_pos (i,player );

                            pos.second=board_destination.y+(tile_center_start_y+pos.second)*scale_factor;
                            pos.first=board_destination.x+(tile_center_start_x+pos.first)*scale_factor;

                            int dx = pos.first-mouse_x;
                            int dy = pos.second-mouse_y;
                            if(dx*dx+dy*dy<(piece_destination_size.w*piece_destination_size.w)/4 && ((i==0 && G.getPlayerUnstarted(player)>0) || (i>0 && G.getPosOccupant(player,i-1))))
                            {
                                for (uint8_t j = 0; j< LegalMoves.size(); ++j)
                                    if(LegalMoves[j].first+1==i)
                                    {
                                        hide_piece_id=LegalMoves[j].first+1;

                                        move_id=j;
                                        state=MOVE_PIECE;
                                        piece_target_id=LegalMoves[j].first+1+roll;

                                        if (save_replay)
                                        {
                                            replay_out<<"move "<<player<<" from "<<hide_piece_id<<" to "<<piece_target_id<<std::endl;
                                        }

                                        if (play_replay)
                                        {
                                            std::string input;

                                            //TEMP, check that the Game matches what we load
                                            replay_in>>input;
                                            replay_in>>input;
                                            replay_in>>input;
                                            replay_in>>input;
                                            replay_in>>input;
                                            replay_in>>input;
                                        }



                                        break;
                                    }
                            }
                        }
                    }
                }
                else
                {
                    //Move as the bot desired
                    if (millis>animation_start+BOT_DELAY || !graphics)
                    {
                        move_id = Bot[player].playMove(LegalMoves,player,roll);

                        hide_piece_id= LegalMoves[move_id].first+1;

                        if (save_replay)
                        {
                            replay_out<<"move "<<player<<" from "<<hide_piece_id<<" to "<<piece_target_id<<std::endl;
                        }


                        piece_target_id= LegalMoves[move_id].first+1+roll;

                        {
                            std::pair<int,int> Start = get_piece_pos(hide_piece_id,player);
                            std::pair<int,int> End = get_piece_pos(piece_target_id,player);

                            int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                            if (!graphics)
                                time=0;
                            Animations.push_back(piece_animation(piece_target_id,player,Start.first,Start.second,End.first,End.second,millis,millis+time));
                            state=MOVE_PIECE;

                            animation_start=millis+time;//absorb wait into this
                            if (!graphics)
                                menu_animation_end=menu_animation_start;
                        }
                    }
                }



            }
            else if (state==MOVE_PIECE)
            {
                if (!player_bot[player])
                {
                    if (justUnClicked)
                    {
                        animation_start=millis;
                        std::pair<int,int> pos=get_piece_pos (piece_target_id,player);

                        pos.second=board_destination.y+(tile_center_start_y+pos.second)*scale_factor;
                        pos.first=board_destination.x+(tile_center_start_x+pos.first)*scale_factor;

                        int dx = pos.first-mouse_x;
                        int dy = pos.second-mouse_y;

                        bool found = dx*dx+dy*dy<(piece_destination_size.w*piece_destination_size.w);

                        if (!found)
                        {
                            int dx = pos.first-mouse_x-0.1*die_destination.w*scale_factor;
                            int dy = pos.second-mouse_y-0.1*die_destination.h*scale_factor;
                            //For the sake of moving this, having the shadow above should count as well
                            found = dx*dx+dy*dy<(piece_destination_size.w*piece_destination_size.w);

                        }

                        if(found )
                        {
                            //Took one of the start pieces?
                            if(hide_piece_id==0)
                            {
                                //Move another one into position
                                if (G.getPlayerUnstarted(player)>0)
                                {
                                    std::pair<int,int> Start = get_piece_pos(1-G.getPlayerUnstarted(player),player);
                                    std::pair<int,int> End = get_piece_pos(0,player);
                                    int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                                    if (!graphics)
                                        time=0;
                                    Animations.push_back(piece_animation(0,player,Start.first,Start.second,End.first,End.second,millis,millis+time));

                                }


                            }
                            else if (piece_target_id==15)//Moved it to the end
                            {
                                //Move it wherever it is supposed to go
                                std::pair<int,int> Start = get_piece_pos(15,player);
                                std::pair<int,int> End = get_piece_pos(16+G.getPlayerFinished(player),player);
                                int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                                if (!graphics)
                                    time=0;
                                Animations.push_back(piece_animation(16+G.getPlayerFinished(player),player,Start.first,Start.second,End.first,End.second,millis,millis+time));

                            }
                            //Took an enemy piece
                            else if  (G.getPosOccupant(!player,piece_target_id-1) && piece_target_id>4 && piece_target_id<13)
                            {
                                    std::pair<int,int> Start = get_piece_pos(piece_target_id,!player);
                                    std::pair<int,int> End = get_piece_pos(-G.getPlayerUnstarted(!player),!player);
                                    int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                                    if (!graphics)
                                        time=0;
                                    Animations.push_back(piece_animation(-G.getPlayerUnstarted(!player),!player,Start.first,Start.second,End.first,End.second,millis,millis+time));
                            }


                            G=LegalMoves[move_id].second;

                            if (save_replay)
                            {
                                replay_out<<"turn "<<turn<<" game "<<G.getPlayerData()<<" "<<(int)G.getFinished()<<" "<<(int)G.getUnstarted()<<std::endl;
                            }
                            if (play_replay)
                            {
                                std::string input;

                                //TEMP, check that the Game matches what we load
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                                replay_in>>input;
                            }

                            ++turn;

                            hide_piece_id=-8;
                            piece_target_id=-8;

                            if (graphics)
                                Mix_PlayChannel(-1, piece_sound , 0);

                            if (G.gameOver())
                            {
                                if (player)
                                    ++player0Wins;
                                state=WIN;
                                //Bring up pause menu, to allow restartign
                                menu_animation_start=millis;
                                menu_animation_end=millis+400;

                                if (!graphics)
                                    menu_animation_end=menu_animation_start;

                                paused=!paused;
                                if (graphics)
                                    dice_channel=Mix_PlayChannel(-1, menu_slide, 0);

                                if (save_replay)
                                    replay_out<<"end "<<player<<" win"<<std::endl;


                            }
                            else
                            {
                                state=WAIT_ROLL;
                                player = G.whoseTurn();
                            }
                        }
                        else
                        {
                            if (graphics)
                                Mix_PlayChannel(-1, piece_sound , 0);
                            hide_piece_id=-8;
                            state=TAKE_PIECE;
                        }
                    }
                }
                else if (millis>animation_start || !graphics)//The time to move is absorbed into the animation start
                {
                    //play animation of movement
                    if(hide_piece_id==0)
                    {
                        //Move another one into position
                        if (G.getPlayerUnstarted(player)>0)
                        {
                            std::pair<int,int> Start = get_piece_pos(1-G.getPlayerUnstarted(player),player);
                            std::pair<int,int> End = get_piece_pos(0,player);
                            int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                            if (!graphics)
                                time=0;
                            Animations.push_back(piece_animation(0,player,Start.first,Start.second,End.first,End.second,millis,millis+time));

                        }


                    }
                    else if (piece_target_id==15)//Moved it to the end
                    {
                        //Move it wherever it is supposed to go
                        std::pair<int,int> Start = get_piece_pos(15,player);
                        std::pair<int,int> End = get_piece_pos(16+G.getPlayerFinished(player),player);
                        int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                        if (!graphics)
                            time=0;
                        Animations.push_back(piece_animation(16+G.getPlayerFinished(player),player,Start.first,Start.second,End.first,End.second,millis,millis+time));

                    }
                    //Took an enemy piece
                    else if  (G.getPosOccupant(!player,piece_target_id-1) && piece_target_id>4 && piece_target_id<13)
                    {
                            std::pair<int,int> Start = get_piece_pos(piece_target_id,!player);
                            std::pair<int,int> End = get_piece_pos(-G.getPlayerUnstarted(!player),!player);
                            int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                            if (!graphics)
                                time=0;
                            Animations.push_back(piece_animation(-G.getPlayerUnstarted(!player),!player,Start.first,Start.second,End.first,End.second,millis,millis+time));
                    }




                    G=LegalMoves[move_id].second;

                    if (save_replay)
                    {
                        replay_out<<"turn "<<turn<<" game "<<G.getPlayerData()<<" "<<(int)G.getFinished()<<" "<<(int)G.getUnstarted()<<std::endl;
                    }
                    ++turn;

                    hide_piece_id=-8;
                    piece_target_id=-8;

                    if (graphics)
                        Mix_PlayChannel(-1, piece_sound , 0);

                    if (G.gameOver())
                    {
                        if (player)
                            ++player0Wins;
                        state=WIN;


                        if (save_replay)
                            replay_out<<"end "<<player<<" win"<<std::endl;
                        //Bring up pause menu, to allow restartign
                        menu_animation_start=millis;
                        menu_animation_end=millis+400;

                        if (!graphics)
                            menu_animation_end=menu_animation_start;


                        paused=!paused;
                        if (graphics)
                            dice_channel=Mix_PlayChannel(-1, menu_slide, 0);

                    }
                    else
                    {
                        state=WAIT_ROLL;
                        player = G.whoseTurn();
                    }

                }
            }
        }//Menu access
        else if (paused && ((justClicked && millis>menu_animation_end) || (!graphics && matches>1)))
        {
            if ((mouse_y>menu_destination.y && mouse_y<menu_destination.y+menu_destination.h) || (!graphics && matches>1) )
                if (mouse_x>menu_destination.x || (!graphics && matches>1))
                {
                    --matches;

                    if (mouse_x<menu_destination.x+menu_destination.w/3 || (!graphics && matches>1))
                    {
                        menu_animation_start=millis;
                        menu_animation_end=millis+400;

                        if (!graphics)
                            menu_animation_end=menu_animation_start;


                        paused=false;

                        if (graphics)
                            dice_channel=Mix_PlayChannel(-1, menu_slide, 0);

                        state=WAIT_ROLL;
                        player = false;

                        for (int p = 0; p <2; ++p)
                        {
                            bool Player = (p==1);
                            int unstarted = G.getPlayerUnstarted(Player);
                            for (int i = 0; i <14; ++i)
                            {
                                if (G.getPosOccupant(Player,i))
                                {
                                    //Animate all pieces moving back, starting now

                                    std::pair<int,int> Start= get_piece_pos(1+i,Player);
                                    std::pair<int,int> End = get_piece_pos(-unstarted ,Player);
                                    int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                                    if (!graphics)
                                        time=0;
                                    Animations.push_back(piece_animation(-unstarted,Player,Start.first,Start.second,End.first,End.second,millis,millis+time));
                                    ++unstarted;
                                }
                            }
                            int finished = G.getPlayerFinished(Player);

                            for (int i = 0; i <finished ; ++i)
                            {
                                //Animate all pieces moving back, starting now

                                std::pair<int,int> Start= get_piece_pos(15+i,Player);
                                std::pair<int,int> End = get_piece_pos(-unstarted ,Player);
                                int time = 250*sqrt((End.first-Start.first)*(End.first-Start.first)+(End.second-Start.second)*(End.second-Start.second))/tile_size;//4 tile width per second
                                if (!graphics)
                                    time=0;
                                Animations.push_back(piece_animation(-unstarted,Player,Start.first,Start.second,End.first,End.second,millis,millis+time));
                                ++unstarted;
                            }
                        }

                        G=Game(game_start_state,game_start_unstarted);

                        turn=0;

                    }
                    else if (mouse_x<menu_destination.x+2*menu_destination.w/3 && graphics)
                    {
                        quit=true;
                    }
                    else  if (mouse_x<menu_destination.x+menu_destination.w && graphics)
                    {
                        menu_animation_start=millis;
                        menu_animation_end=millis+400;

                        if (!graphics)
                            menu_animation_end=menu_animation_start;


                        paused=false;

                    }


                }
            if (!graphics)
                paused =false;
        }


        //Make display ready, and start displaying
        SDL_RenderClear(renderer);

        SDL_Rect thisTileRect;
        thisTileRect.x=0;
        thisTileRect.y=0;
        thisTileRect.w=background_destination.w;
        thisTileRect.h=background_destination.h;


        for (;(uint32_t)thisTileRect.x<win_w; thisTileRect.x+=background_destination.w)
        {
            thisTileRect.y=0;
            for (;(uint32_t)thisTileRect.y<win_h; thisTileRect.y+=background_destination.h)
            {
                SDL_RenderCopy(renderer, background_tile, NULL, &thisTileRect);
            }
        }

        //Those things below the board and its shadow
        G.display_below(display_piece,get_piece_pos,hide_piece_id,player,Animations,millis);

        //The boards shadow

        //The board
        SDL_RenderCopy(renderer, board_texture, NULL, &board_destination);

        //Display the pieces already on the board
        G.display_board(display_piece,get_piece_pos,hide_piece_id,player,Animations,millis);


        //Marker if we need to roll
        if (state==WAIT_ROLL || state==HOLD_ROLL)
            SDL_RenderCopy(renderer, player ? roll_marker1_texture: roll_marker0_texture, NULL, &roll_marker_destination);

        die_destination.x=board_destination.x/2-die_destination.w/2;
        die_destination.y=(win_h-die_destination.h)/2-2.25*die_destination.h;//Start, 2 die above edge, where each die has 0.5 die betwixt themself

        //The 4 dice's shadow
        for (int i = 0; i < 4; ++i)
        {
            if (state==HOLD_ROLL)//Holding the dice in your hand
            {

                if (player_bot[player])
                {
                    die_destination.x=roll_marker_destination.x+roll_marker_destination.w/2-die_destination.w+die_destination.w*(i%2)+0.1*die_destination.w*scale_factor+sin((millis-animation_start)/25.0)*roll_marker_destination.w/8;
                    die_destination.y=roll_marker_destination.y+roll_marker_destination.h/2-die_destination.h+die_destination.h*(i<2? 0 : 1)+0.1*die_destination.h*scale_factor+sin((millis-animation_start)/50.0)*roll_marker_destination.h/4;
                }
                else
                {
                    die_destination.x=mouse_x-die_destination.w+die_destination.w*(i%2)+0.1*die_destination.w*scale_factor;
                    die_destination.y=mouse_y-die_destination.h+die_destination.h*(i<2? 0 : 1)+0.1*die_destination.h*scale_factor;
                }

            }
            else
            {
                die_destination.x=board_destination.x/2-die_destination.w/2+offset_x[i]*scale_factor+0.02*die_destination.w*scale_factor;
                die_destination.y=(win_h-die_destination.h)/2-2.25*die_destination.h+die_destination.h*1.5*i+ offset_y[i]*scale_factor+0.02*die_destination.h*scale_factor;
            }

            SDL_Point center;
            center.x=die_destination.w*0.5;
            center.y=die_destination.h*0.5;


            SDL_RenderCopyEx(renderer,die_shadow, NULL, &die_destination, state==HOLD_ROLL ? 0 :  angle[i], &center, SDL_FLIP_NONE);

        }


        //The 4 dice
        for (int i = 0; i < 4; ++i)
        {
            //Pretend the mouse is over the dice
            if (state==HOLD_ROLL)//Holding the dice in your hand
            {
                if (player_bot[player])
                {
                    die_destination.x=roll_marker_destination.x+roll_marker_destination.w/2-die_destination.w+die_destination.w*(i%2)+sin((millis-animation_start)/25.00)*roll_marker_destination.w/8;
                    die_destination.y=roll_marker_destination.y+roll_marker_destination.h/2-die_destination.h+die_destination.h*(i<2? 0 : 1)+sin((millis-animation_start)/50.0)*roll_marker_destination.h/4;
                }
                else
                {
                    die_destination.x=mouse_x-die_destination.w+die_destination.w*(i%2);
                    die_destination.y=mouse_y-die_destination.h+die_destination.h*(i<2? 0 : 1);
                }
            }
            else
            {
                die_destination.x=board_destination.x/2-die_destination.w/2+offset_x[i]*scale_factor;
                die_destination.y=(win_h-die_destination.h)/2-2.25*die_destination.h+die_destination.h*1.5*i+ offset_y[i]*scale_factor;
            }

            SDL_Point center;
            center.x=die_destination.w*0.5;
            center.y=die_destination.h*0.5;



            SDL_RenderCopyEx(renderer,die_texture[current_dice[i]], NULL, &die_destination, state==HOLD_ROLL ? (i<2? 0 : -60) :  angle[i], &center, SDL_FLIP_NONE);

            die_destination.y+=1.5*die_destination.h;
        }

        if (state==TAKE_PIECE)
        {


            for (uint8_t i = 0; i< LegalMoves.size(); ++i)
            {

                std::pair<int,int> pos = get_piece_pos(LegalMoves[i].first+1,player);




                piece_destination_size.x =board_destination.x+(tile_center_start_x+pos.first)*scale_factor-piece_destination_size.w/2;
                piece_destination_size.y =board_destination.y+(tile_center_start_y+pos.second)*scale_factor-piece_destination_size.h/2;


                SDL_Point center;
                center.x= piece_destination_size.w*0.5;
                center.y= piece_destination_size.h*0.5;

                SDL_RenderCopyEx(renderer,player ? select1_texture : select0_texture , NULL, &piece_destination_size, millis*18/100, &center, SDL_FLIP_NONE);
            }
        }
        else if (state==MOVE_PIECE && !player_bot[player])
        {
            std::pair<int,int> pos = get_piece_pos(piece_target_id,player);




            piece_destination_size.x =board_destination.x+(tile_center_start_x+pos.first)*scale_factor-piece_destination_size.w/2;
            piece_destination_size.y =board_destination.y+(tile_center_start_y+pos.second)*scale_factor-piece_destination_size.h/2;


            SDL_Point center;
            center.x= piece_destination_size.w*0.5;
            center.y= piece_destination_size.h*0.5;

            SDL_RenderCopyEx(renderer,player ? select1_texture : select0_texture , NULL, &piece_destination_size, millis*18/100, &center, SDL_FLIP_NONE);

            piece_destination_size.x=mouse_x-piece_destination_size.w/2+0.1*die_destination.w*scale_factor;
            piece_destination_size.y=mouse_y-piece_destination_size.h/2+0.1*die_destination.h*scale_factor;

            SDL_RenderCopy(renderer, piece_shadow_texture , NULL, &piece_destination_size);

            piece_destination_size.x=mouse_x-piece_destination_size.w/2;
            piece_destination_size.y=mouse_y-piece_destination_size.h/2;

            SDL_RenderCopy(renderer, player? piece1_texture : piece0_texture, NULL, &piece_destination_size);

        }
        else if (state==WIN)
        {//No more moving, bring up pause menu to restart

        }

        for  (piece_animation& A : Animations)
        {
            if (millis>=A.start && millis<=A.end)
            {
                float fac = float(millis-A.start)/(A.end-A.start);


                piece_destination_size.x =board_destination.x+(tile_center_start_x+A.x0*(1-fac)+A.x1*fac)*scale_factor-piece_destination_size.w/2+0.1*die_destination.w*scale_factor;
                piece_destination_size.y =board_destination.y+(tile_center_start_y+A.y0*(1-fac)+A.y1*fac)*scale_factor-piece_destination_size.h/2+0.1*die_destination.h*scale_factor;

                SDL_RenderCopy(renderer, piece_shadow_texture , NULL, &piece_destination_size);

                piece_destination_size.x-=0.1*die_destination.w*scale_factor;
                piece_destination_size.y-=0.1*die_destination.h*scale_factor;

                SDL_RenderCopy(renderer, A.player ? piece1_texture : piece0_texture , NULL, &piece_destination_size);
            }
            else if (millis>A.end && A.start<=A.end)//Mark this as already sounded, then sound it
            {
                A.start=millis;

                if (graphics)
                    Mix_PlayChannel(-1, piece_sound , 0);
            }

        }


        if (Animations.size()>0 && millis>Animations[0].end)
        {
            Animations.pop_front();
        }


        if ( (millis>=menu_animation_start && millis<menu_animation_end) || paused )
        {
            if (millis<menu_animation_end)
            {
                float fac = (millis-menu_animation_start)/float(menu_animation_end-menu_animation_start);

                if (!paused)
                    fac=1-fac;
                fac = 1-(1-fac)*(1-fac)*(1-fac);
                menu_destination.y=(1-fac)*win_h*1.5+(fac)*menu_destination.y;
            }
            menu_destination.x+=menu_destination.w/10;
            menu_destination.y+=menu_destination.w/10;
            SDL_RenderCopy(renderer,  menu_shadow, NULL, &menu_destination);
            menu_destination.x-=menu_destination.w/10;
            menu_destination.y-=menu_destination.w/10;

        SDL_RenderCopy(renderer,  menu_texture , NULL, &menu_destination);

        }


        //Flush changes
        SDL_RenderPresent(renderer);
    }

    std::cout<<player0Wins<<" / "<<start_matches<<std::endl;

    //Close SDL
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(background_tile);
    SDL_DestroyTexture(board_texture);
    SDL_DestroyTexture(piece0_texture);
    SDL_DestroyTexture(piece1_texture);
    SDL_DestroyTexture(select0_texture);
    SDL_DestroyTexture(select1_texture);
    SDL_DestroyTexture(roll_marker0_texture);
    SDL_DestroyTexture(roll_marker1_texture);
    SDL_DestroyTexture(die_shadow);
    SDL_DestroyTexture(menu_texture);

    SDL_DestroyRenderer(renderer);
    Mix_FreeChunk(rattle_sound);
    Mix_FreeChunk(roll_sound);
    Mix_FreeChunk(piece_sound);
    Mix_FreeChunk(menu_slide);
    IMG_Quit();
    SDL_Quit();

    if (save_replay)
        replay_out.close();
    else if (play_replay)
        replay_in.close();


    return 0;
}
