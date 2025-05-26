#ifndef DUNGEON_H
#define DUNGEON_H

#define MAX_FILE_NAME_SIZE 106

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
    struct Room* connections[4];
    int visited;
    union 
    {
        Monster* monster;
        Item* item;
    };

    struct Room* next;
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


Room* create_room(int id);
void connect_rooms(Room* a, Room* b);
void connect_bidirectional(Room* a, Room* b);
Gamestate* generate_dungeon(int roomcount);
void assign_content_to_rooms(Gamestate* game);
void room_discription(Player* player);
Room* choose_next_room(Room* current);
void use_item(Player* player, Item* item);
void fight(Player* player, Monster* monster);
int enter_room(Player* player);
void gameplay(Gamestate* game);
void ask_save(Gamestate* game);


void free_monster(Monster* m);
void free_items(Item* i);
void free_dungeon(Room* head);
void free_player(Player* p);
void free_gamestate(Gamestate* game);


void save_game(Gamestate* game, const char* filename);
Gamestate* load_game(const char* filename);

#endif