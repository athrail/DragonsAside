#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <vector>

#include "tile.hpp"

bool Tile::has_road_connection(RoadConnections &con) {
  return !!(m_road_connections & static_cast<uint8_t>(con));
}

void Tile::render(SDL_Renderer *r) {
  const std::vector<RoadConnections> connections{
      RoadConnections::Up, RoadConnections::Right, RoadConnections::Down,
      RoadConnections::Left};

  switch (m_type) {
  case TileType::Road: {
    SDL_SetRenderDrawColor(r, 0x1F, 0x5F, 0x26, 0xFF);
    SDL_RenderFillRect(r, &m_rect);
    for (auto &con : connections) {
      if (!!(m_road_connections & static_cast<uint8_t>(con))) {
        switch (con) {
        case RoadConnections::Up: {
          auto road = SDL_FRect{m_rect.x + m_rect.w * 0.25f, m_rect.y,
                                m_rect.w * 0.5f, m_rect.h * 0.75f};
          SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
          SDL_RenderFillRect(r, &road);
          break;
        }
        case RoadConnections::Right: {
          auto road = SDL_FRect{m_rect.x + m_rect.w * 0.25f,
                                m_rect.y + m_rect.h * 0.25f, m_rect.w * 0.75f,
                                m_rect.h * 0.5f};
          SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
          SDL_RenderFillRect(r, &road);
          break;
        }
        case RoadConnections::Down: {
          auto road = SDL_FRect{m_rect.x + m_rect.w * 0.25f,
                                m_rect.y + m_rect.h * 0.25f, m_rect.w * 0.5f,
                                m_rect.h * 0.75f};
          SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
          SDL_RenderFillRect(r, &road);
          break;
        }
        case RoadConnections::Left: {
          auto road = SDL_FRect{m_rect.x, m_rect.y + m_rect.h * 0.25f,
                                m_rect.w * 0.75f, m_rect.h * 0.5f};
          SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
          SDL_RenderFillRect(r, &road);
          break;
        }
        default:
          break;
        }
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
