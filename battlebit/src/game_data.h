
#include <stdbool.h>

#define typename(x) _Generic((x),        /* Get the name of a type */             \
                                                                                  \
        _Bool: "_Bool",                  unsigned char: "unsigned char",          \
         char: "char",                     signed char: "signed char",            \
    short int: "short int",         unsigned short int: "unsigned short int",     \
          int: "int",                     unsigned int: "unsigned int",           \
     long int: "long int",           unsigned long int: "unsigned long int",      \
long long int: "long long int", unsigned long long int: "unsigned long long int", \
        float: "float",                         double: "double",                 \
  long double: "long double",                   char *: "pointer to char",        \
       void *: "pointer to void",                int *: "pointer to int",         \
      default: "other")


typedef struct ships_data {
    int carrier;
    int battleship;
    int destroyer;
    int submarine;
    int patrol_boat;
} ships_data;

int get_char_ship_type_len(char ship);
bool is_ship_horizontal(char ship);
void init_ships_data(struct ships_data * ship_data);
void mark_ship_seen(char ship, struct ships_data * data);

bool has_seen_char_ship(char ship, struct ships_data *data);
bool is_char_valid_ship(char ship);
bool is_char_valid_coordinate(char coordinate);
bool is_valid_coordinate(int x, int y);

