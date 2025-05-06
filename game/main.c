#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAX_ROOMS 100
#define MAX_ROOM_CONECT 4

typedef enum { EMPTY, MONSTER, LOOT, CHEST }Roomtype;
typedef enum { GOBLIN, SKELETON }Monstertype;
typedef enum { POWERUP, HEALTH }Itemtype;


typedef struct Monster
{
    Monstertype type;
    int HP;
    int PP;
}Monster;

typedef struct Item
{
    Itemtype type;
    int value;
}Item;

typedef struct Room
{
    int id;
    Roomtype type;
    struct Room* connections[MAX_ROOM_CONECT];
    int visited;
    union 
    {
        Monster* monster;
        Item* item;
    };
    
}Room;

typedef struct Player
{
    int HP;
    int PP;
    Room* currentRoom;
}Player;

typedef struct Gamestate
{
    Player* Player;
    Room** rooms;
    int roomcount;
}Gamestate;


//geregation

Room* create_room(int id)
{
    Room* room = malloc(sizeof(Room));
    room->id = id;
    room->type = EMPTY;
    room->visited = 0;
    for (int i = 0; i < MAX_ROOM_CONECT; i++)
        room->connections[i] = NULL;

    return room;
}


void connect_rooms(Room* a, Room* b)
{
    for (int i = 0; i < MAX_ROOM_CONECT; i++)
    {
        if (a->connections[i] == NULL)
        {
            a->connections[i] = b;
            break;
        }
    }
}

void connect_bidirectional(Room* a, Room* b)
{
    connect_rooms(a, b);
    connect_rooms(b, a);
}

Gamestate* generate_dungeon(int roomcount)
{
    Gamestate* game = malloc(sizeof(Gamestate));
    game->rooms = malloc(sizeof(Room*) * roomcount);
    game->roomcount = roomcount;

    for (int i = 0; i < roomcount; i++)
        game->rooms[i] = create_room(i);

    for (int i = 0; i < roomcount; i++)
    {
        int other = rand()% i;
        connect_bidirectional(game->rooms[i], game->rooms[other]);
    }

    game->Player = malloc(sizeof(Player));
    game->Player->HP = 20;
    game->Player->PP = 5;
    game->Player->currentRoom = game->rooms;

    return game;
}

int main(int argv, char* argc[])
{
    srand(time(NULL));
    int numrooms = 10;

    if (argc >= 2)
    {
        numrooms = atoi(argc[1]);
        if (numrooms < 2 || numrooms > MAX_ROOMS)
        {
            printf("Room ammount need to be between 2 and %d\n", MAX_ROOMS);    
            return 1;
        }
    }

    Gamestate* game = generate_dungeon(numrooms);
    printf("amount of rooms = %d", numrooms);
    printf("Starting room = %d", game->Player->currentRoom->id);

    return 0;
}