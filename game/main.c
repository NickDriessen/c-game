#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "dungeon.h" //for the structs and enmus
#include "cJSON.h" //for the save and load game


int main(int argc, char *argv[])
{

    srand(time(NULL)); //makes random numbers passable

    Gamestate* game = NULL; //starts the gamestate ar NULL (no game jet)

    int load = 0; //chechs if you user chose to load a file ar start a now game

    for (int i = 0; i < strlen(argv[1]); i++) 
    {
        if (argv[1][i] == '.')      //checks if there is a . in the sting for .json
        {
            load = 1;
            char fileName[MAX_FILE_NAME_SIZE];
            strcpy(fileName, argv[1]);
            game = load_game(fileName);
            printf("------------Welcome back to the dungeon------------\n");
        }
    }

    if(!load && argv[1][1] <= '9' && argv[1][1] >= '1') // checks if the first letter in the string is between a 1 and a 9 and if the game is not loaded 
    {
        int roomAmmount;
        sscanf(argv[1], "%d", &roomAmmount); // makes the string a int so it can be used
        if (roomAmmount >= 2 && roomAmmount <= 100)
        {
            game = generate_dungeon(roomAmmount);
            printf("--------------Welcome to the dungeon--------------\n");
        }
        else
            printf("ammount not right needs to be between 2 and 100");
    }


    gameplay(game); //the gameplay

    if (game->Player->currentRoom->type != CHEST && game->Player->HP > 0) //checks if the playes is dead or if the player won the game
        ask_save(game);


    free_gamestate(game); //frees the alocated memory
    return 0; //exit the game
}

// dungeon generation
Room* create_room(int id)
{
    Room* room = calloc(1, sizeof(Room)); //alocates the memory and sets it to 0
    room->id = id; // gives the room its room id
    room->type = EMPTY; // sets the room type to EMPTY

    return room;
}

void connect_rooms(Room* a, Room* b) //makes it so the rooms are connected
{
    for (int i = 0; i < 4; i++)
    {
        if (a->connections[i] == 0)
        {
            a->connections[i] = b;
            break;
        }
    }
}

void connect_bidirectional(Room* a, Room* b) //makes it so the room are connected from both sides
{
    connect_rooms(a, b);
    connect_rooms(b, a);
}

Gamestate* generate_dungeon(int roomcount) //the fungeon generation
{
    Gamestate* game = calloc(1, sizeof(Gamestate)); // alocates the memory for Gamestate ans sets it to 0
    game->rooms = calloc(1, sizeof(Room*) * roomcount); // allocates the memory for the selected amount of rooms and sets it to 0
    game->roomcount = roomcount;

    for (int i = 0; i < roomcount; i++) //creates the specified room amount
        game->rooms[i] = create_room(i);

    for (int i = 1; i < roomcount; i++) //connects random rooms
    {
        int other = rand() % i;
        connect_bidirectional(game->rooms[i], game->rooms[other]);
    }
    //creates the player
    game->Player = calloc(1, sizeof(Player));
    game->Player->HP = 20;
    game->Player->PP = 5;
    game->Player->currentRoom = game->rooms[0];

    assign_content_to_rooms(game); //sets the conten to the room (monster, loot, chest)

    return game;
}

Monster* create_monster()
{
    Monster* m = calloc(1, sizeof(Monster)); // alocates the memory for monster and sets it to 0
    m->type = rand() % 2; //chooses the random monster thats in the room

    if(m->type == GOBLIN)
    {
        m->HP = 12; // health points of goblin
        m->PP = 2; // power points of skeleton
    }
    if(m->type == SKELETON)
    {
        m->HP = 6; // health points of skeleton
        m->PP = 4; // power points of skeleton
    }

    return m;
}

Item* create_item()
{
    Item* i = calloc(1, sizeof(Item)); // alocates the memory for item and sets it to 0
    i->type = rand() % 2; //chooses the random item thats in the room

    if(i->type == HEALTH)
    {
        i->value = 5; // gives 5 healt points
    }
    if(i->type == POWERUP)
    {
        i->value = 2; // gives 2 power points
    }

    return i;
}

void assign_content_to_rooms(Gamestate* game)
{
    int treasureRoom = 1 + (rand() % (game->roomcount - 1)); //makes the number id of the treasureRoom

    for( int i = 0; i < game->roomcount; i++)
    {
        if(i == 0)
            continue;

        else if(i == treasureRoom) //places the treasureRoom
        {
            game->rooms[i]->type = CHEST;
            continue;
        }

        int random = rand() % 100;

        if(random < 30) //places the monsterRoom
        {
            game->rooms[i]->type = MONSTER;
            game->rooms[i]->monster = create_monster();
        }
        else if (random < 70) //places the itemRoom
        {
            game->rooms[i]->type = LOOT;
            game->rooms[i]->item = create_item();
        }
        else //places the emptyRoom
        {
            game->rooms[i]->type = EMPTY;
        }
    }
}

//the game play
void room_discription(Player* player)
{
    Room* room = player->currentRoom; //shows you the room the player is in
    printf("you are currently in room %d\n\n", room->id); //prints out the room the player is in

    if(room->visited) //shuld only print when the room is not visited before(does fully not work)
    {
        printf("This room has been visited before(by you).\n");
    }
    else
    {
        switch (room->type) //is seposed the tell you what room your in but it doesn't
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

    printf("This room is connected to rooms: "); //shows what rooms you can go to
    for(int i = 0; i < 4; i++)
        {
            if(room->connections[i] != NULL)
                printf("%d ", room->connections[i]->id);
        }
    printf("\n");
}

Room* choose_next_room(Room* current)
{
    char option[9];
    printf("choose a door(room id) or type exit: "); //asks what room you want to go to or exit the game
    scanf("%9s", &option);

    if (strcmp(option,"exit")==0) //if exit wil send you to the ask save game
    {
        return NULL;
    }
    
    int i_option = atoi(option);

    for(int i = 0; i < 4; i++) //cheks if chosen door is possible
    {
        if(current->connections[i] != NULL && current->connections[i]->id == i_option)
        {
            return current->connections[i];
        }
    }

    printf("That is not a option, try again.\n"); //if not possible, asks again
    return choose_next_room(current);
}

void use_item(Player* player, Item* item)
{
    switch (item->type) //gives and uses the found item
    {
    case HEALTH:
        player->HP += item->value;
        printf("\nYou found a potion and gained %d HP! (now %d)\n", item->value, player->HP);
        break;
    case POWERUP:
        player->PP += item->value;
        printf("\nYou found a PPup and your power gets a +%d (now %d)\n", item->value, player->PP);
        break;
    default:
        printf("No item.\n");
        break;
    }
}

void fight(Player* player, Monster* monster)
{
    if (monster->type == GOBLIN)
    {
        printf("\nA goblin attacks!\n");
    }
    else
    {
        printf("\nA seleton attacks!\n");
    }

    printf("Combat starts!\n\n"); //starts the monster fight
    getchar();

    while (player->HP > 0 && monster->HP > 0) //combat with monster till one is dead
    {
        int round = rand() %16; //gives a random number tussen de 0 en 16

        for(int i = 4; i >= 0; i--)
        {
            int bit = (round >> i) & 1; //makes random number a bit

            if(bit == 0) //uses bit so the monster or the player fights
            {
                player->HP -= monster->PP;
                if (player->HP < 0)
                    player->HP = 0;

                printf("The monster attacks!\n You take %d demage, remaining hp = %d\n", monster->PP, player->HP);
                getchar();
            }
            else
            {
                monster->HP -= player->PP;
                if (monster->HP < 0)
                    monster->HP = 0;
                
                printf("You attack the monster!\n It takes %d damage! monster hp = %d\n", player->PP, monster->HP);
                getchar();
            }

            if(player->HP == 0 || monster->HP == 0) // if one dies the fights end
                break;
        }

        printf("\n");
    }

    if (player->HP <= 0) // if the player dies
    {
        printf("You where defeated by the monster.\n");
    }
    else // if the monster dies
    {
        printf("The monster is defeated!\n");
    }
}

int enter_room(Player* player)
{
    Room* room = player->currentRoom; //checks what room the player enterd

    if(!room->visited) //checks if you viseted the room before
    {
        switch (room->type)
        {
        case MONSTER: //case monster the fight starts
            fight(player, room->monster);
            free(room->monster);
            room->monster = NULL;
            break;
        case LOOT: //case loot you get and use a item
            use_item(player, room->item);
            free(room->item);
            room->item = NULL;
            break;
        case CHEST: //case chest you win the game
            printf("You found the chest you win the game!\n");
            getchar();
            return 1;
            break;
        default:
            break;
        }
        room->visited = 1; //makes it so the room is visited
    }

    return 0;
}

void gameplay(Gamestate* game)
{
    int finished = 0;
    int player_exit = 0;
    
    while (!finished && game->Player->HP > 0 && !player_exit) //checks if the player is not dead or the chest if found or if you didn't exit the game
    {
        room_discription(game->Player); //describes the room

        Room* next = choose_next_room(game->Player->currentRoom); //makes it so you can shoose the next room
        if (next == NULL) //makes it so you can exit the room
        {
            player_exit = 1;
            continue;
        }
        game->Player->currentRoom = next;

        finished = enter_room(game->Player);
    }
    if (finished) //for if you win the game
    {
        printf("Congratulations!\n");
    } 
    else if (player_exit) //fot if you exit the game
    {
        printf("You have chosen to exit the dungeon. Farewell, adventurer!\n");
    } 
    else if (game->Player->HP <= 0) //for if you die
    {
        printf("Your journey has ended in demise.\n");
    } 
    else //just in case should never happen
    {
        printf("The game has ended for an unknown reason.\n");
    }
}

//freeing memory
void free_monster(Monster* m) //frees the monster memory
{
    if(m != NULL)
    {
        free(m);
    }
}

void free_items(Item* i) //frees the item memory
{
    if (i != NULL)
    {
        free(i);
    }
}

void free_player(Player* p) //frees the player memory
{
    if(p != NULL)
    {
        free(p);
    }
}

void free_gamestate(Gamestate* game) //makes it so the others get freed
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

//saving & loading the game.
void ask_save(Gamestate* game)
{
    char choice[10];    

    printf("Do you want to save progress?(yes/no)\ninput: "); //asks you is you want to save your game
    scanf("%9s", choice);

    for (int i = 0; i < strlen(choice); i++)
    {
        choice[i] = tolower(choice[i]); //makes it all lower case
    }

    if(strcmp(choice, "yes")==0) //if yes you wil start the saving game progress
    {
        char savefile[MAX_FILE_NAME_SIZE];

        printf("Give the name of a savefile (max 100 char and dont add .json)\ninput: "); //asks for the file name
        scanf("%100s", savefile);

        strcat(savefile, ".json"); //adds the .json for you

        save_game(game, savefile); //saves the game in a .json file

        printf("game saved on: %s", savefile); //tells you the full file name
    }
    else if (strcmp(choice, "no")==0) // if no the game wil just close
    {
        printf("Game not saved.");
    }
    else //for a mis type
    {
        printf("wrong input try again.");
        ask_save(game);
    }
}

void save_game(Gamestate* game, const char* filename) //the part I get the least. Made it with ai and i dont understant it one bit but i dont have time to learn it or chainge it i wil try to explaine it when asked. (but keep your hope low)
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
        Room* room = calloc(1, sizeof(Room));
        room->id = cJSON_GetObjectItem(r, "id")->valueint;
        room->visited = cJSON_GetObjectItem(r, "visited")->valueint;

        int type_int = cJSON_GetObjectItem(r, "type")->valueint;
        room->type = (Roomtype)type_int;

        if(room->type == MONSTER)
        {
            cJSON* m = cJSON_GetObjectItem(r, "monster");
            if(m)
            {
                Monster* mon = calloc(1, sizeof(Monster));
                int mtype = cJSON_GetObjectItem(m, "type")->valueint;
                mon->type = (Monstertype)mtype;
                mon->HP = cJSON_GetObjectItem(m, "HP")->valueint;
                mon->PP = cJSON_GetObjectItem(m, "PP")->valueint;
                room->monster = mon;
            }
            else
                room->monster = NULL;
        }
        else if (room->type == LOOT)
        {
            cJSON* i = cJSON_GetObjectItem(r, "item");
            if (i)
            {
                Item* item = calloc(1,sizeof(Item));
                int itype = cJSON_GetObjectItem(i, "type")->valueint;
                item->type = (Itemtype)itype;
                item->value = cJSON_GetObjectItem(i, "value")->valueint;
                room->item = item;
            }
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
        Room* current_room_obj = game->rooms[cJSON_GetObjectItem(r, "id")->valueint];
        cJSON* conns = cJSON_GetObjectItem(r, "connections");
        for (int j = 0; j < cJSON_GetArraySize(conns); j++)
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

    int currentRoomId = cJSON_GetObjectItem(player_json, "currentRoom")->valueint;
    p->currentRoom = game->rooms[currentRoomId];
    game->Player = p;

    cJSON_Delete(root);
    return game;
}