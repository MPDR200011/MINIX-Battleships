#include "player.h"

uint n_ships = 5;

Player player_init() {
  Player player;
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      player.player_board[i][j] = OCEAN;
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      player.oponent_board[i][j] = OPONENT_UNKNOWN;
    }
  }

  ship* battleships_tmp = malloc(sizeof(ship) * n_ships);

  for (uint i = 0; i < n_ships; i++) {
    int cycle = i % 5;

    switch (cycle) {
      case 0: {  // destroyer size 2
        bool* hit = malloc(sizeof(bool) * 2);
        memset(hit, 0, 2);
        ship tmp = {.exists = 0,
                    .st = destroyer,
                    .vertical = 0,
                    .x = -1,
                    .y = -1,
                    .hits = hit,
                    .size = 2};
        battleships_tmp[i] = tmp;
        break;
      }
      case 1: {  // submarine size 3
        bool* hit = malloc(sizeof(bool) * 3);
        memset(hit, 0, 3);
        ship tmp = {.exists = 0,
                    .st = submarine,
                    .vertical = 0,
                    .x = -1,
                    .y = -1,
                    .hits = hit,
                    .size = 3};
        battleships_tmp[i] = tmp;
        break;
      }
      case 2: {  // cruiser size 3
        bool* hit = malloc(sizeof(bool) * 3);
        memset(hit, 0, 3);
        ship tmp = {.exists = 0,
                    .st = cruiser,
                    .vertical = 0,
                    .x = -1,
                    .y = -1,
                    .hits = hit,
                    .size = 3};
        battleships_tmp[i] = tmp;
        break;
      }
      case 3: {  // battleship size 4
        bool* hit = malloc(sizeof(bool) * 4);
        memset(hit, 0, 4);
        ship tmp = {.exists = 0,
                    .st = battleship,
                    .vertical = 0,
                    .x = -1,
                    .y = -1,
                    .hits = hit,
                    .size = 4};
        battleships_tmp[i] = tmp;
        break;
      }
      case 4: {  // carrier size 5
        bool* hit = malloc(sizeof(bool) * 5);
        memset(hit, 0, 5);
        ship tmp = {.exists = 0,
                    .st = carrier,
                    .vertical = 0,
                    .x = -1,
                    .y = -1,
                    .hits = hit,
                    .size = 5};
        battleships_tmp[i] = tmp;
        break;
      }
    }
  }

  player.battleships = battleships_tmp;
  player.battleships_size = n_ships;

  return player;
}

int player_setPlayerShip(Player* self, ship s) {
  ship* ship = 0;
  for (uint i = 0; i < n_ships; i++) {
    if (self->battleships[i].st == s.st && !self->battleships[i].exists) {
      ship = &self->battleships[i];
      break;
    }
  }

  if (ship == 0) {
    return -1;
  }

  if (s.vertical) {
    if (s.y + s.size <= 10) {
      for (uint i = 0; i < s.size; i++) {
        if (self->player_board[s.y + i][s.x] == PLAYER_SHIP) {
          return -1;
        }
      }
      for (uint i = 0; i < s.size; i++) {
        self->player_board[s.y + i][s.x] = PLAYER_SHIP;
      }
    } else {
      return -1;
    }
  } else {
    if (s.x + s.size <= 10) {
      for (uint i = 0; i < s.size; i++) {
        if (self->player_board[s.y][s.x + i] == PLAYER_SHIP) {
          return -1;
        }
      }
      for (uint i = 0; i < s.size; i++) {
        self->player_board[s.y][s.x + i] = PLAYER_SHIP;
      }
    } else {
      return -1;
    }
  }

  ship->exists = true;
  ship->x = s.x;
  ship->y = s.y;
  ship->vertical = s.vertical;

  return 0;
}

void player_removeShip(Player* self, shiptype type) {
  ship* s = 0;
  for (uint i = 0; i < n_ships; i++) {
    if (self->battleships[i].st == type && self->battleships[i].exists) {
      s = &self->battleships[i];
      break;
    }
  }

  if (s == 0) {
    return;
  }

  if (s->vertical) {
    for (uint i = 0; i < s->size; i++) {
      self->player_board[s->y + i][s->x] = OCEAN;
    }
  } else {
    for (uint i = 0; i < s->size; i++) {
      self->player_board[s->y][s->x + i] = OCEAN;
    }
  }

  s->exists = false;
  s->x = -1;
  s->y = -1;
  s->vertical = false;
}

bool player_shootPlayer(Player* self, uint32_t x, uint32_t y) {
  if (self->player_board[y][x] == PLAYER_SHIP) {
    self->player_board[y][x] = PLAYER_SHIP_HIT;
    return true;
  } else {
    self->player_board[y][x] = OPONENT_TRY;
    return false;
  }
}

bool player_checkIfFleetDestroyed(Player* self) {
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      if (self->player_board[i][j] == PLAYER_SHIP) {
        return false;
      }
    }
  }
  return true;
}

uint32_t player_getOponentCellColor(Player* self, uint32_t x, uint32_t y) {
  return self->oponent_board[y][x];
}

