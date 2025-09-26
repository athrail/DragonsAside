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
  SDL_Point m_start_tile{ .x = 0, .y = m_board_height - 1};
  SDL_Point m_finish_tile{ .x = m_board_width - 1, .y = 0};

public:
  void init(int res_x, int res_y);
  void randomize_draw_pile();
  Tile &get_tile(uint8_t x, uint8_t y);
  void set_selected(uint8_t x, uint8_t y);
  void unselect();
  void render(SDL_Renderer *r);
  void add_valid_moves_from_tile(const int x, const int y);
  void update_valid_moves();
  void new_game();
  void recalculate_reachable_tiles();
  void recalculate_end_tiles();
  bool can_reach_end();

private:
  std::array<Tile, m_board_size> m_tiles;
  std::array<SDL_Point, m_board_size> m_reachable_tiles;
  int m_reachable_tiles_count{0};
  std::array<SDL_Point, m_board_size> m_end_tiles;
  int m_end_tiles_count{0};
  SDL_FRect m_exit_arrow{};
};

#endif // _BOARD_HPP
