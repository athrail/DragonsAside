#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <array>
#include <cstdint>
#include <map>

#include "tile.hpp"

bool Tile::has_road_connection(const RoadConnections &con) const {
  return !!(m_road_connections & static_cast<uint8_t>(con));
}

void Tile::render(SDL_Renderer *r) const {
  const std::array<RoadConnections, 4> connections{
      RoadConnections::Up, RoadConnections::Right, RoadConnections::Down,
      RoadConnections::Left};
  const std::map<RoadConnections, SDL_FRect> connection_rects{
      {RoadConnections::Up, SDL_FRect{m_rect.x + m_rect.w * 0.25f, m_rect.y,
                                      m_rect.w * 0.5f, m_rect.h * 0.75f}},
      {RoadConnections::Right,
       SDL_FRect{m_rect.x + m_rect.w * 0.25f, m_rect.y + m_rect.h * 0.25f,
                 m_rect.w * 0.75f, m_rect.h * 0.5f}},
      {RoadConnections::Down,
       SDL_FRect{m_rect.x + m_rect.w * 0.25f, m_rect.y + m_rect.h * 0.25f,
                 m_rect.w * 0.5f, m_rect.h * 0.75f}},
      {RoadConnections::Left, SDL_FRect{m_rect.x, m_rect.y + m_rect.h * 0.25f,
                                        m_rect.w * 0.75f, m_rect.h * 0.5f}}};

  switch (m_type) {
  case TileType::Road: {
    SDL_SetRenderDrawColor(r, 0x1F, 0x5F, 0x26, 0xFF);
    SDL_RenderFillRect(r, &m_rect);
    SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
    for (auto &con : connections) {
      if (!!(m_road_connections & static_cast<uint8_t>(con))) {
        SDL_RenderFillRect(r, &connection_rects.at(con));
      }
    }
    break;
  }
  case TileType::Dragon: {
    SDL_SetRenderDrawColor(r, 0xFF, 0x0, 0x7F, 0xFF);
    SDL_RenderFillRect(r, &m_rect);
    break;
  }
  case TileType::Equipment: {
    SDL_SetRenderDrawColor(r, 0x0, 0xAA, 0x7F, 0xFF);
    SDL_RenderFillRect(r, &m_rect);
    break;
  }
  default:
    break;
  }

  // Border
  SDL_SetRenderDrawColor(r, 0x18, 0x18, 0x18, 0xFF);
  SDL_RenderRect(r, &m_rect);
}

void Tile::rotate() {
  const std::array<RoadConnections, 4> connections{
      RoadConnections::Up, RoadConnections::Right, RoadConnections::Down,
      RoadConnections::Left};
  uint8_t new_connections{0};
  for (auto &con : connections) {
    if (!!(m_road_connections & static_cast<uint8_t>(con))) {
      switch(con) {
      case RoadConnections::Up:
        new_connections |= static_cast<uint8_t>(RoadConnections::Right);
        break;
      case RoadConnections::Right:
        new_connections |= static_cast<uint8_t>(RoadConnections::Down);
        break;
      case RoadConnections::Down:
        new_connections |= static_cast<uint8_t>(RoadConnections::Left);
        break;
      case RoadConnections::Left:
        new_connections |= static_cast<uint8_t>(RoadConnections::Up);
        break;
      }
    }
  }
  m_road_connections = new_connections;
}
