#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "dungeon.h"
#include "cJSON.h"

#define MAX_FILE_NAME_SIZE 100

int main()
{

    srand(time(NULL));

    Gamestate* game = NULL;
    char choice[10];

    printf("-----------Welkom to the game!-----------\n");
    printf("Do you want to load game or start a new save?(Load/New)\ninput: ");
    scanf("%9s", choice);

    for (int i = 0; i < strlen(choice); i++)
    {
        choice[i] = tolower(choice[i]);
    }
    
    if(strcmp(choice, "load")==0)
    {
        char fileName[MAX_FILE_NAME_SIZE];
        printf("give the name of your save file (max 100 char and dont add .json).\n input: ");
        scanf("%100s", fileName);

        strcat(fileName, ".json");

        game = load_game(fileName);
        if(!game)
        {
            printf("File \"%s\" not found and/or loading failed.\n", fileName);
            main();
        }

    }
    else if(strcmp(choice, "new")==0)
    {
        int roomAmmount = 0;
        printf("How manny room do you want to explore (between 2 and 100)?\ninput: ");
        scanf("%d", &roomAmmount);
        if (roomAmmount > 2 && roomAmmount < 100)
        {
            game = generate_dungeon(roomAmmount);
        }
        else
        {
            printf("invaled room amount try again.\n");
            main();
        }
    }
    else
    {
        printf("\"%s\" is not a valed choise", choice);
        main();
    }

    gameplay(game);

    Player* player;    
    if (player->HP <= 0 || game->Player->currentRoom->type != CHEST)
        ask_save(game);

    free_gamestate(game);
    return 0;
}


void ask_save(Gamestate* game)
{
    char choice[10];    

    printf("Do you want to save progress?(yes/no)\ninput: ");
    scanf("%9s", choice);

    for (int i = 0; i < strlen(choice); i++)
    {
        choice[i] = tolower(choice[i]);
    }

    if(strcmp(choice, "yes")==0)
    {
        char savefile[MAX_FILE_NAME_SIZE];

        printf("Give the name of a savefile (max 100 char and dont add .json)");
        scanf("%100s", savefile);

        strcat(savefile, ".json");

        save_game(game, savefile);

        printf("game saved on: %s", savefile);
    }
    else if (strcmp(choice, "no")==0)
    {
        printf("Game not saved.");
    }
    else
    {
        printf("wrong input try again.");
        ask_save(game);
    }
    
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


    int other = 0;
    for (int i = 1; i < roomcount+1; i++)
    {
        other = rand() % i;
        connect_bidirectional(game->rooms[i-1], game->rooms[other]);
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
            game->rooms[i]->monster = create_monster();
        }
        else if (random < 60)
        {
            game->rooms[i]->type = LOOT;
            game->rooms[i]->item = create_item();
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
        int round = rand() %17;
        printf("Attack order = ");

        for(int i = 4; i >= 0; i--)
        {
            int bit = (round >> i) & 1;
            printf("%d", bit);

            if(bit == 0)
            {
                player->HP -= monster->PP;
                if (player->HP < 0)
                    player->HP = 0;

                printf("The monster attacks!\n You take %d demage, remaining hp = %d\n", monster->PP, player->HP);
            }
            else
            {
                monster->HP -= player->PP;
                if (monster->HP < 0)
                    monster->HP = 0;
                
                printf("You attack the monster!\n It takes %d damage! monster hp = %d\n", player->PP, monster->HP);
            }

            if(player->HP == 0 || monster->HP == 0)
                break;
        }

        printf("\n");
    }

    if (player->HP <= 0)
    {
        printf("You where defeated by the monster.\n");
    }
    else
    {
        printf("The monster is defeated!\n");
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


//freeing memory
void free_monster(Monster* m)
{
    if(m != NULL)
    {
        free(m);
    }
}

void free_items(Item* i)
{
    if (i != NULL)
    {
        free(i);
    }
}

void free_dungeon(Room* head)
{
    Room* current = head;
    while(current != NULL)
    {
        free_monster(current->monster);
        free_items(current->item);

        if(current->connections != NULL)
        {
            free(current->connections);
        }

        Room* temp = current;
        current = current->next;
        free(temp);
    }
}

void free_player(Player* p)
{
    if(p != NULL)
    {
        free(p);
    }
}

void free_gamestate(Gamestate* game)
{
    if (!game)
        return;
    
    for(int i = 0; i < game->roomcount; i++)
    {
        Room* room = game->rooms[i];
        if (!room)
            continue;

        if(room->type == MONSTER && room->monster)
        {
            free(room->monster);
            room->monster = NULL;
        }

        if(room->type == LOOT && room->item)
        {
            free(room->item);
            room->item = NULL;
        }

        free(room);
    }

    free(game->rooms);

    if (game->Player)
    {
        free(game->Player);
    }

    free(game);
}

void save_game(Gamestate* game, const char* filename)
{
    cJSON* root = cJSON_CreateObject();
    cJSON* rooms = cJSON_CreateArray();

    for (int i = 0; i < game->roomcount; i++)
    {
        Room* r = game->rooms[i];
        cJSON* room = cJSON_CreateObject();
        cJSON_AddNumberToObject(room, "id", r->id);
        cJSON_AddNumberToObject(room, "type", r->type);
        cJSON_AddNumberToObject(room, "visited", r->visited);

        cJSON* connections = cJSON_CreateArray();
        for (int j = 0; j < 4; j++)
        {
            if (r->connections[j])
            {
                cJSON_AddItemToArray(connections, cJSON_CreateNumber(r->connections[j]->id));
            }
            else
            {
                cJSON_AddItemToArray(connections, cJSON_CreateNull());
            }
        }
        cJSON_AddItemToObject(room, "connections", connections);

        if(r->type == MONSTER && r->monster != NULL)
        {
            cJSON* mon = cJSON_CreateObject();
            cJSON_AddNumberToObject(mon, "type", r->monster->type);
            cJSON_AddNumberToObject(mon,"HP", r->monster->HP);
            cJSON_AddNumberToObject(mon,"PP", r->monster->PP);
            cJSON_AddItemToObject(room, "monster", mon);
        }
        else if(r->type == LOOT && r->item != NULL)
        {
            cJSON* it = cJSON_CreateObject();
            cJSON_AddNumberToObject(it, "type", r->item->type);
            cJSON_AddNumberToObject(it, "value", r->item->value);
            cJSON_AddItemToObject(room, "item", it);
        }

        cJSON_AddItemToArray(rooms, room);
    }

    cJSON* player = cJSON_CreateObject();
    cJSON_AddNumberToObject(player, "HP", game->Player->HP);
    cJSON_AddNumberToObject(player, "PP", game->Player->PP);
    cJSON_AddNumberToObject(player, "currentRoom", game->Player->currentRoom->id);

    cJSON_AddItemToObject(root, "rooms", rooms);
    cJSON_AddItemToObject(root, "player", player);
    cJSON_AddNumberToObject(root, "roomcount", game->roomcount);

    char* json_data = cJSON_Print(root);
    FILE* file = fopen(filename, "w");
    if(file)
    {
        fputs(json_data, file);
        fclose(file);
        printf("Game saved to %s\n", filename);
    }
    else
    {
        printf("failed to open te save file.\n");
    }

    free(json_data);
    cJSON_Delete(root);
}

Gamestate* load_game(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("Kon %s niet openen.\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON* root = cJSON_Parse(data);
    free(data);
    if (!root)
    {
        printf("JSON parse error.\n");
        return NULL;
    }

    Gamestate* game = calloc(1, sizeof(Gamestate));

    cJSON* rooms_json = cJSON_GetObjectItem(root, "rooms");
    int roomcount = cJSON_GetArraySize(rooms_json);
    game->roomcount = roomcount;
    game->rooms = calloc(roomcount, sizeof(Room*));

    for(int i = 0; i < roomcount; i++)
    {
        cJSON* r = cJSON_GetArrayItem(rooms_json, i);
        Room* room = calloc(1, sizeof(room));
        room->id = cJSON_GetObjectItem(r, "id")->valueint;
        room->visited = cJSON_GetObjectItem(r, "visited")->valueint;

        const char* type_str = cJSON_GetObjectItem(r, "type")->valuestring;
        if(strcmp(type_str, "MONSTER")==0)
        {
            room->type = MONSTER;
            cJSON* m = cJSON_GetObjectItem(r, "monster");
            if(m)
            {
                Monster* mon = calloc(1, sizeof(Monster));
                const char* mtype = cJSON_GetObjectItem(m, "type")->valuestring;
                mon->type = strcmp(mtype, "GOBLIN") == 0 ? GOBLIN : SKELETON;
                mon->HP = cJSON_GetObjectItem(m, "HP")->valueint;
                mon->PP = cJSON_GetObjectItem(m, "PP")->valueint;
                room->monster = mon;
            }
        }
        else if (strcmp(type_str, "LOOT")==0)
        {
            room->type = LOOT;
            cJSON* i = cJSON_GetObjectItem(r, "item");
            if (i)
            {
                Item* item = calloc(1,sizeof(Item));
                const char* itype = cJSON_GetObjectItem(i, "type")->valuestring;
                item->type = strcmp(itype, "HEALTH")==0 ? HEALTH : POWERUP;
                item->value = cJSON_GetObjectItem(i, "value")->valueint;
                room->item = item;
            }
        }
        else if(strcmp(type_str, "CHEST")==0)
        {
            room->type = CHEST;
        }
        else
        {
            room->type = EMPTY;
        }

        game->rooms[room->id] = room;
    }


    for(int i = 0; i < roomcount; i++)
    {
        cJSON* r = cJSON_GetArrayItem(rooms_json, i);
        cJSON* conns = cJSON_GetObjectItem(r, "connections");
        for (int j = 0; j < 4; j++)
        {
            cJSON* conn = cJSON_GetArrayItem(conns, j);
            if (conn && !cJSON_IsNull(conn))
            {
                int other_id = conn->valueint;
                game->rooms[i]->connections[j] = game->rooms[other_id];
            }
        }
    }


    cJSON* player_json = cJSON_GetObjectItem(root, "player");
    Player* p = calloc(1, sizeof(Player));
    p->HP = cJSON_GetObjectItem(player_json, "HP")->valueint;
    p->PP = cJSON_GetObjectItem(player_json, "PP")->valueint;
    int currentRoomId = cJSON_GetObjectItem(player_json, "currentRoomId")->valueint;
    p->currentRoom = game->rooms[currentRoomId];
    game->Player = p;

    cJSON_Delete(root);
    return game;
}