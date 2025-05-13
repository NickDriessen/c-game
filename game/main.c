#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAX_ROOMS 100

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

Room* create_room(int id)
{
    Room* room = calloc(1, sizeof(Room));
    room->id = id;
    room->type = EMPTY;

    return room;
}


void connect_rooms(Room* a, Room* b)
{
    for (int i = 0; i < 4; i++)
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
    Gamestate* game = calloc(1, sizeof(Gamestate));
    game->rooms = calloc(1, sizeof(Room*) * roomcount);
    game->roomcount = roomcount;

    for (int i = 0; i < roomcount; i++)
        game->rooms[i] = create_room(i);

    for (int i = 0; i < roomcount; i++)
    {
        int other = rand()% i;
        connect_bidirectional(game->rooms[i], game->rooms[other]);
    }

    game->Player = calloc(1, sizeof(Player));
    game->Player->HP = 20;
    game->Player->PP = 5;
    game->Player->currentRoom = game->rooms[0];

    assign_content_to_rooms(game);

    return game;
}

Monster* create_monster()
{
    Monster* m = calloc(1, sizeof(Monster));
    m->type = rand() % 2;

    if(m->type == GOBLIN)
    {
        m->HP = 10;
        m->PP = 4;
    }
    if(m->type == SKELETON)
    {
        m->HP = 5;
        m->PP = 7;
    }

    return m;
}

Item* create_item()
{
    Item* i = calloc(1, sizeof(Item));
    i->type = rand() % 2;

    if(i->type == HEALTH)
    {
        i->value = 5;
    }
    if(i->type == POWERUP)
    {
        i->value = 2;
    }

    return i;
}

void assign_content_to_rooms(Gamestate* game)
{
    int treasureRoom = rand() % game->roomcount;

    for( int i = 0; i < game->roomcount; i++)
    {
        if(i == 0)
            continue;

        else if(i == treasureRoom)
        {
            game->rooms[i]->type = CHEST;
            continue;
        }

        int random = rand() % 100;

        if(random < 30)
        {
            game->rooms[i]->type = MONSTER;
            game->rooms[i]->monster = create_monster;
        }
        else if (random < 60)
        {
            game->rooms[i]->type = LOOT;
            game->rooms[i]->item = create_item;
        }
        else
        {
            game->rooms[i]->type = EMPTY;
        }
    }
}

void room_discription(Player* player)
{
    Room* room = player->currentRoom;
    printf("you are currently in room %d\n", room->id);

    if(room->visited)
    {
        printf("This room has been visited before(by you).\n");
    }
    else
    {
        switch (room->type)
        {
        case MONSTER:
            printf("A wild Monster apeared!\n");
            break;
        case LOOT:
            printf("You found a item!\n");
            break;
        case CHEST:
            printf("There be treasure(you found the chest!)\n");
            break;
        case EMPTY:
            printf("The room is empty.\n");
            break;
        default:
            printf("you somehow enterd a ERROR ROOM");
            break;
        }
    }

    printf("This room is connected to rooms: ");
    for(int i = 0; i < 4; i++)
        {
            if(room->connections[i] != NULL)
                printf("%d ", room->connections[i]->id);
        }
    printf("\n");
}

Room* choose_next_room(Room* current)
{
    int option;
    printf("choose a door(room id): ");
    scanf("%d", &option);
    
    for(int i = 0; i < 4; i++)
    {
        if(current->connections[i] != NULL && current->connections[i]->id == option)
        {
            return current->connections[i];
        }
    }

    printf("That is not a option, try again.\n");
    return choose_next_room(current);
}

void use_item(Player* player, Item* item)
{
    switch (item->type)
    {
    case HEALTH:
        player->HP += item->value;
        printf("You gained %d HP! (now %d)\n", item->value, player->HP);
        break;
    case POWERUP:
        player->PP += item->value;
        printf("You used a PPUP power gets a +%d (now %d)\n", item->value, player->PP);
        break;
    default:
        printf("No item.\n");
        break;
    }
}

void fight(Player* player, Monster* monster)
{
    printf("Combat starts!\n");

    while (player->HP > 0 && monster->HP > 0)
    {
        monster->HP -= player->PP;
        if (monster->HP > 0)
        {
            player->HP -= monster->PP;
        }
        printf("speler HP: %d, Monster HP: %d", player->HP, monster->HP);
    }

    if (player->HP <= 0)
    {
        printf("You lost the combat.\n");
    } 
    else
    {
        printf("you defeated the monster!");
    }
}

int enter_room(Player* player)
{
    Room* room = player->currentRoom;

    if(!room->visited)
    {
        switch (room->type)
        {
        case MONSTER:
            fight(player, room->monster);
            free(room->monster);
            room->monster = NULL;
            break;
        case LOOT:
            use_item(player, room->item);
            free(room->item);
            room->item = NULL;
            break;
        case CHEST:
            printf("You found the chest you win the game!\n");
            getchar();
            return 1;
            break;
        default:
            break;
        }
        room->visited = 1;
    }

    return 0;
}

void gameplay(Gamestate* game)
{
    int finished = 0;
    
    while (!finished && game->Player->HP > 0)
    {
        room_discription(game->Player);

        Room* next = choose_next_room(game->Player->currentRoom);
        game->Player->currentRoom = next;

        finished = enter_room(game->Player);
    }
    if(game->Player->HP <= 0)
    {
        printf("You died.\n");
    }
}