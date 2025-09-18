#include <algorithm>
#include <cassert>
#include <random>
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
      auto &tile = get_tile(x, y);
      tile.m_rect = SDL_FRect{m_position.x + (x * m_tile_width),
                              m_position.y + (y * m_tile_height), m_tile_width,
                              m_tile_height};
    }
  }

  m_entry_arrow = SDL_FRect{
      m_position.x - 50.0f,
      m_position.y + m_board_rect.h - (m_tile_height * 0.5f) - 7.5f,
      30,
      15,
  };
  m_entry_arrow_points[0] = SDL_FPoint{m_entry_arrow.x + m_entry_arrow.w - 5.0f,
                                       m_entry_arrow.y - 5.0f};
  m_entry_arrow_points[1] = SDL_FPoint{m_entry_arrow.x + m_entry_arrow.w + 5.0f,
                                       m_entry_arrow.y + 7.5f};
  m_entry_arrow_points[2] =
      SDL_FPoint{m_entry_arrow.x + m_entry_arrow.w - 5.0f,
                 m_entry_arrow.y + m_entry_arrow.h + 5.0f};

  constexpr size_t equipment_count = 3;
  for (size_t i{0}; i < equipment_count; ++i) {
    auto &tile = get_tile(get_random(), 1 + get_random());
    tile.m_type = TileType::Equipment;
  }

  randomize_draw_pile();

  m_valid_moves.push_back({0, 7});
  m_valid_moves.push_back({5, 0});
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

  SDL_SetRenderDrawColor(r, 0x0, 0xFF, 0x0, 0x20);
  for (auto &point : m_valid_moves) {
    SDL_RenderFillRect(r, &get_tile(point.x, point.y).m_rect);
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(r, 0xFF, 0x0, 0x0, 0xFF);
  SDL_RenderRect(r, &m_entry_arrow);
  SDL_RenderLines(r, m_entry_arrow_points, 3);
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
    printf("Checking %d,%d if valid\n", p.x, p.y);

    if ((p.x == 0) && (p.y == m_board_height - 1))
      return false;

    if ((p.y == 0) && (p.x == m_board_width - 1))
      return false;

    auto &type = get_tile(p.x, p.y).m_type;
    if ((type != TileType::None) && (type != TileType::Equipment)) {
      printf("Tile already occupied\n");
      return true;
    }

    if (p.x > 0) {
      valid_move =
          valid_move ||
          get_tile(p.x - 1, p.y).has_road_connection(RoadConnections::Right);
      printf("Checking connection on left: %b\n", valid_move);
    }

    if (p.x < m_board_width - 1) {
      valid_move =
          valid_move ||
          get_tile(p.x + 1, p.y).has_road_connection(RoadConnections::Left);
      printf("Checking connection on right: %b\n", valid_move);
    }

    if (p.y > 0) {
      valid_move =
          valid_move ||
          get_tile(p.x, p.y - 1).has_road_connection(RoadConnections::Down);
      printf("Checking connection on up: %b\n", valid_move);
    }

    if (p.y < m_board_height - 1) {
      valid_move =
          valid_move ||
          get_tile(p.x, p.y + 1).has_road_connection(RoadConnections::Up);
      printf("Checking connection on down: %b\n", valid_move);
    }

    printf("Move is valid\n");
    return !valid_move;
  });
}
