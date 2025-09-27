#include <algorithm>
#include <cassert>
#include <cstdio>
#include <queue>
#include <random>
#include <string.h>
#include <vector>

#include "board.hpp"
#include "tile.hpp"

extern int get_random();
extern std::mt19937 eng;

void Board::init(int res_x, int res_y) {
  if (res_x > res_y) {
    const float tile_size = static_cast<float>(res_y - 100) / m_board_height;
    m_tile_height = m_tile_width = tile_size;
    m_position = SDL_FPoint{
        (res_x * 0.5f) - ((m_board_width * m_tile_width) * 0.5f), 50};
  } else {
    const float tile_size = static_cast<float>(res_x - 100) / m_board_width;
    m_tile_height = m_tile_width = tile_size;
    m_position = SDL_FPoint{
        50,
        (res_y * 0.5f) - ((m_board_height * m_tile_height) * 0.5f),
    };
  }
  m_board_rect =
      SDL_FRect{m_position.x, m_position.y, m_board_width * m_tile_width,
                m_board_height * m_tile_height};
  for (int x{0}; x < m_board_width; ++x) {
    for (int y{0}; y < m_board_height; ++y) {
      get_tile(x, y).m_rect = SDL_FRect{m_position.x + (x * m_tile_width),
                                        m_position.y + (y * m_tile_height),
                                        m_tile_width, m_tile_height};
    }
  }
}

void Board::new_game() {
  m_valid_moves.clear();

  for (int x{0}; x < m_board_width; ++x) {
    for (int y{0}; y < m_board_height; ++y) {
      auto &tile = get_tile(x, y);
      tile.m_type = TileType::None;
      tile.m_selected = false;
    }
  }

  constexpr size_t equipment_count = 3;
  for (size_t i{0}; i < equipment_count; ++i)
    get_tile(get_random(), 1 + get_random()).m_type = TileType::Equipment;

  randomize_draw_pile();

  m_valid_moves.push_back({0, 7});
  m_valid_moves.push_back({5, 0});

  recalculate_reachable_tiles();
  recalculate_end_tiles();
}

void Board::randomize_draw_pile() {
  m_draw_pile.clear();

  // Tiles:
  // 21 roads (1 dead end, 7 straights, 4 turns, 7 T turns, 2 crossroads)
  // 16 dragons
  // 3 knights equipment
  m_draw_pile.push_back(
      Tile{.m_type = TileType::Road,
           .m_road_connections = static_cast<uint8_t>(RoadConnections::Up)});

  for (size_t i{0}; i < 7; ++i) {
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Down)});

    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Down) |
                              static_cast<uint8_t>(RoadConnections::Left)});
  }

  for (size_t i{0}; i < 4; ++i) {
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Left)});
  }

  m_draw_pile.push_back(
      Tile{.m_type = TileType::Road, .m_road_connections = 15});
  m_draw_pile.push_back(
      Tile{.m_type = TileType::Road, .m_road_connections = 15});

  constexpr size_t dragons_count = 16;
  for (size_t i{0}; i < dragons_count; ++i)
    m_draw_pile.push_back(Tile{.m_type = TileType::Dragon});

  std::ranges::shuffle(m_draw_pile, eng);
}

Tile &Board::get_tile(uint8_t x, uint8_t y) {
  assert(x < m_board_width);
  assert(y < m_board_height);
  return m_tiles.at((y * m_board_width) + x);
}

void Board::set_selected(uint8_t x, uint8_t y) {
  if (m_selected_tile)
    m_selected_tile->m_selected = false;
  m_selected_tile = &get_tile(x, y);
  m_selected_tile->m_selected = true;
}

void Board::unselect() {
  if (m_selected_tile) {
    m_selected_tile->m_selected = false;
    m_selected_tile = nullptr;
  }
}

void Board::render(SDL_Renderer *r) {
  for (auto &tile : m_tiles) {
    tile.render(r);
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  if (m_selected_tile) {
    SDL_SetRenderDrawColor(r, 0xFF, 0xFF, 0xFF, 0x20);
    SDL_RenderFillRect(r, &m_selected_tile->m_rect);
  }

  // SDL_SetRenderDrawColor(r, 0x0, 0xFF, 0x0, 0x20);
  // for (auto &point : m_valid_moves) {
  //   SDL_RenderFillRect(r, &get_tile(point.x, point.y).m_rect);
  // }

  SDL_SetRenderDrawColor(r, 0x80, 0xFF, 0xff, 0x20);
  for (int i{0}; i < m_reachable_tiles_count; ++i) {
    auto point = m_reachable_tiles.at(i);
    SDL_RenderFillRect(r, &get_tile(point.x, point.y).m_rect);
  }

  SDL_SetRenderDrawColor(r, 0xFF, 0x20, 0x20, 0x40);
  for (int i{0}; i < m_end_tiles_count; ++i) {
    auto point = m_end_tiles.at(i);
    SDL_RenderFillRect(r, &get_tile(point.x, point.y).m_rect);
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(r, 0xFF, 0x0, 0x0, 0xFF);
}

void Board::add_valid_moves_from_tile(const int x, const int y) {
  const auto &tile = get_tile(x, y);
  const std::array<RoadConnections, 4> connections = {
      RoadConnections::Up, RoadConnections::Down, RoadConnections::Left,
      RoadConnections::Right};

  for (auto &con : connections) {
    switch (con) {
    case RoadConnections::Up: {
      if (y == 0)
        break;
      if (!tile.has_road_connection(con))
        break;
      const auto &upper_tile = get_tile(x, y - 1);
      if ((upper_tile.m_type == TileType::Road) ||
          (upper_tile.m_type == TileType::Dragon))
        break;
      m_valid_moves.push_back(SDL_Point{x, y - 1});
      break;
    }
    case RoadConnections::Right: {
      if (x == m_board_width - 1)
        break;
      if (!tile.has_road_connection(con))
        break;
      const auto &right_tile = get_tile(x + 1, y);
      if ((right_tile.m_type == TileType::Road) ||
          (right_tile.m_type == TileType::Dragon))
        break;
      m_valid_moves.push_back(SDL_Point{x + 1, y});
      break;
    }
    case RoadConnections::Down: {
      if (y == m_board_height - 1)
        break;
      if (!tile.has_road_connection(con))
        break;
      const auto &right_tile = get_tile(x, y + 1);
      if ((right_tile.m_type == TileType::Road) ||
          (right_tile.m_type == TileType::Dragon))
        break;
      m_valid_moves.push_back(SDL_Point{x, y + 1});
      break;
    }
    case RoadConnections::Left: {
      if (x == 0)
        break;
      if (!tile.has_road_connection(con))
        break;
      const auto &right_tile = get_tile(x - 1, y);
      if ((right_tile.m_type == TileType::Road) ||
          (right_tile.m_type == TileType::Dragon))
        break;
      m_valid_moves.push_back(SDL_Point{x - 1, y});
      break;
    }
    }
  }
}

void Board::update_valid_moves() {
  std::erase_if(m_valid_moves, [this](const SDL_Point &p) -> bool {
    bool valid_move{false};

    if ((p.x == 0) && (p.y == m_board_height - 1))
      return false;

    if ((p.y == 0) && (p.x == m_board_width - 1))
      return false;

    auto &type = get_tile(p.x, p.y).m_type;
    if ((type != TileType::None) && (type != TileType::Equipment)) {
      return true;
    }

    if (p.x > 0) {
      valid_move =
          valid_move ||
          get_tile(p.x - 1, p.y).has_road_connection(RoadConnections::Right);
    }

    if (p.x < m_board_width - 1) {
      valid_move =
          valid_move ||
          get_tile(p.x + 1, p.y).has_road_connection(RoadConnections::Left);
    }

    if (p.y > 0) {
      valid_move =
          valid_move ||
          get_tile(p.x, p.y - 1).has_road_connection(RoadConnections::Down);
    }

    if (p.y < m_board_height - 1) {
      valid_move =
          valid_move ||
          get_tile(p.x, p.y + 1).has_road_connection(RoadConnections::Up);
    }

    return !valid_move;
  });
}

void Board::recalculate_reachable_tiles() {
  memset(m_reachable_tiles.data(), 0, m_board_size * sizeof(SDL_Point));
  m_reachable_tiles_count = 0;

  std::queue<SDL_Point> tiles_to_check;
  std::array<bool, m_board_size> visited{false};
  tiles_to_check.emplace(m_start_tile);

  while (!tiles_to_check.empty()) {
    auto coords = tiles_to_check.front();
    auto &tile = get_tile(coords.x, coords.y);
    auto index = (coords.y * m_board_width) + coords.x;

    if (!visited.at(index)) {
      visited.at(index) = true;

      if (tile.m_type == TileType::Road) {
        if (tile.has_road_connection(RoadConnections::Up) && (coords.y > 0))
          tiles_to_check.emplace(coords.x, coords.y - 1);
        if (tile.has_road_connection(RoadConnections::Down) &&
            (coords.y < m_board_height - 1))
          tiles_to_check.emplace(coords.x, coords.y + 1);
        if (tile.has_road_connection(RoadConnections::Left) && (coords.x > 0))
          tiles_to_check.emplace(coords.x - 1, coords.y);
        if (tile.has_road_connection(RoadConnections::Right) &&
            (coords.x < m_board_width - 1))
          tiles_to_check.emplace(coords.x + 1, coords.y);
      } else if ((tile.m_type == TileType::None) ||
                 (tile.m_type == TileType::Equipment)) {
        if (coords.y > 0)
          tiles_to_check.emplace(coords.x, coords.y - 1);
        if (coords.y < m_board_height - 1)
          tiles_to_check.emplace(coords.x, coords.y + 1);
        if (coords.x > 0)
          tiles_to_check.emplace(coords.x - 1, coords.y);
        if (coords.x < m_board_width - 1)
          tiles_to_check.emplace(coords.x + 1, coords.y);
      }

      if (tile.m_type != TileType::Dragon) {
        m_reachable_tiles.at(m_reachable_tiles_count).x = coords.x;
        m_reachable_tiles.at(m_reachable_tiles_count).y = coords.y;
        m_reachable_tiles_count++;
      }
    }
    tiles_to_check.pop();
  }
}

void Board::recalculate_end_tiles() {
  memset(m_end_tiles.data(), 0, m_board_size * sizeof(SDL_Point));
  m_end_tiles_count = 0;

  std::queue<SDL_Point> tiles_to_check;
  std::array<bool, m_board_size> visited{false};
  tiles_to_check.emplace(m_finish_tile);

  while (!tiles_to_check.empty()) {
    auto coords = tiles_to_check.front();
    auto &tile = get_tile(coords.x, coords.y);
    auto index = (coords.y * m_board_width) + coords.x;

    if (!visited.at(index)) {
      visited.at(index) = true;

      if ((tile.m_type == TileType::None) || (tile.m_type == TileType::Equipment)) {
        m_end_tiles.at(m_end_tiles_count).x = coords.x;
        m_end_tiles.at(m_end_tiles_count).y = coords.y;
        m_end_tiles_count++;
      } else if (tile.m_type == TileType::Road) {
        if (tile.has_road_connection(RoadConnections::Up) && (coords.y > 0))
          tiles_to_check.emplace(coords.x, coords.y - 1);
        if (tile.has_road_connection(RoadConnections::Down) &&
            (coords.y < m_board_height - 1))
          tiles_to_check.emplace(coords.x, coords.y + 1);
        if (tile.has_road_connection(RoadConnections::Left) && (coords.x > 0))
          tiles_to_check.emplace(coords.x - 1, coords.y);
        if (tile.has_road_connection(RoadConnections::Right) &&
            (coords.x < m_board_width - 1))
          tiles_to_check.emplace(coords.x + 1, coords.y);
      }
    }
    tiles_to_check.pop();
  }
}

bool Board::can_reach_end() {
  for (int i{m_reachable_tiles_count - 1}; i >= 0; --i) {
    auto tile = m_reachable_tiles.at(i);
    for (int j{0}; j < m_end_tiles_count; ++j) {
      auto end_tile = m_end_tiles.at(j);
      if ((tile.x == end_tile.x) && (tile.y == end_tile.y))
        return true;
    }
  }

  return false;
}
