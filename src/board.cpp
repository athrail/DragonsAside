#include <algorithm>
#include <random>

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
    auto &tile = get_tile(get_random() % m_board_width,
                          1 + (get_random() % (m_board_height - 2)));
    tile.m_type = TileType::Equipment;
  }

  randomize_draw_pile();
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

  if (m_selected_tile) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0xFF, 0xFF, 0xFF, 0x20);
    SDL_RenderFillRect(r, &m_selected_tile->m_rect);
  }

  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(r, 0xFF, 0x0, 0x0, 0xFF);
  SDL_RenderRect(r, &m_entry_arrow);
  SDL_RenderLines(r, m_entry_arrow_points, 3);
}
