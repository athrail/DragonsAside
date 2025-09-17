#ifndef _BOARD_HPP
#define _BOARD_HPP

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <array>
#include <cstdint>
#include <vector>

#include "tile.hpp"

class Board {
public:
  static constexpr uint8_t m_board_width = 6;
  static constexpr uint8_t m_board_height = 8;
  static constexpr uint16_t m_board_size = m_board_width * m_board_height;
  float m_tile_width{};
  float m_tile_height{};
  std::vector<Tile> m_draw_pile;
  Tile *m_selected_tile{nullptr};
  SDL_FPoint m_position{};
  SDL_FRect m_board_rect{};
  std::vector<SDL_Point> m_valid_moves;

private:
  std::array<Tile, m_board_size> m_tiles;
  SDL_FRect m_entry_arrow{};
  SDL_FPoint m_entry_arrow_points[3];
  SDL_FRect m_exit_arrow{};

public:
  void init(int res_x, int res_y);
  void randomize_draw_pile();
  Tile &get_tile(uint8_t x, uint8_t y);
  void set_selected(uint8_t x, uint8_t y);
  void unselect();
  void render(SDL_Renderer *r);
  void add_valid_moves_from_tile(const int x, const int y);
};

#endif // _BOARD_HPP
