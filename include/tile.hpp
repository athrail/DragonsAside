#ifndef _TILE_HPP
#define _TILE_HPP

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <cstdint>

enum struct TileType { None = 0, Equipment, Dragon, Road };

enum struct RoadConnections : uint8_t {
  Up = 1 << 0,
  Right = 1 << 1,
  Down = 1 << 2,
  Left = 1 << 3
};

struct Tile {
  TileType m_type{TileType::None};
  uint8_t m_road_connections{0};
  bool m_selected{false};
  SDL_FRect m_rect{};

  bool has_road_connection(const RoadConnections &con) const;
  void render(SDL_Renderer *r) const;
  void rotate();
};

#endif // _TILE_HPP
