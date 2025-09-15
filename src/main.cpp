#include "SDL3/SDL_blendmode.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <vector>

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

  bool has_road_connection(RoadConnections &con) {
    return !!(m_road_connections & static_cast<uint8_t>(con));
  }

  void render(SDL_Renderer *r) {
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
                                  m_rect.w * 0.5f, m_rect.h * 0.5f};
            SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
            SDL_RenderFillRect(r, &road);
            break;
          }
          case RoadConnections::Right: {
            auto road = SDL_FRect{m_rect.x + m_rect.w * 0.5f,
                                  m_rect.y + m_rect.h * 0.25f, m_rect.w * 0.5f,
                                  m_rect.h * 0.5f};
            SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
            SDL_RenderFillRect(r, &road);
            break;
          }
          case RoadConnections::Down: {
            auto road = SDL_FRect{m_rect.x + m_rect.w * 0.25f,
                                  m_rect.y + m_rect.h * 0.5f, m_rect.w * 0.5f,
                                  m_rect.h * 0.5f};
            SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
            SDL_RenderFillRect(r, &road);
            break;
          }
          case RoadConnections::Left: {
            auto road = SDL_FRect{m_rect.x, m_rect.y + m_rect.h * 0.25f,
                                  m_rect.w * 0.5f, m_rect.h * 0.5f};
            SDL_SetRenderDrawColor(r, 0xf3, 0xd9, 0xab, 0xFF);
            SDL_RenderFillRect(r, &road);
            break;
          }
          default:
            break;
          }
        }
      }
    }
    default:
      break;
    }

    // Border
    SDL_SetRenderDrawColor(r, 0x18, 0x18, 0x18, 0xFF);
    SDL_RenderRect(r, &m_rect);
  }
};

struct Board {
  static constexpr uint8_t m_board_width = 6;
  static constexpr uint8_t m_board_height = 8;
  static constexpr uint16_t m_board_size = m_board_width * m_board_height;

  float m_tile_width{};
  float m_tile_height{};
  std::array<Tile, m_board_size> m_tiles;
  SDL_FPoint m_position{};
  SDL_FRect m_board_rect{};
  Tile *m_selected_tile{nullptr};

  void init(int res_x, int res_y) {
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
                                m_position.y + (y * m_tile_height),
                                m_tile_width, m_tile_height};
      }
    }
  }

  Tile &get_tile(uint8_t x, uint8_t y) {
    return m_tiles.at((y * m_board_width) + x);
  }

  void set_selected(uint8_t x, uint8_t y) {
    if (m_selected_tile)
      m_selected_tile->m_selected = false;
    m_selected_tile = &get_tile(x, y);
    m_selected_tile->m_selected = true;
  }

  void unselect() {
    if (m_selected_tile) {
      m_selected_tile->m_selected = false;
      m_selected_tile = nullptr;
    }
  }

  void render(SDL_Renderer *r) {
    for (auto &tile : m_tiles) {
      tile.render(r);
    }

    if (m_selected_tile) {
      SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
      SDL_SetRenderDrawColor(r, 0xFF, 0xFF, 0xFF, 0x20);
      SDL_RenderFillRect(r, &m_selected_tile->m_rect);
    }
  }
};

struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;

  Board board{};

  bool m_running{true};
};

void update(State &st) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      st.m_running = false;
    } else if (event.type == SDL_EVENT_KEY_DOWN) {
      if (event.key.key == SDLK_ESCAPE)
        st.m_running = false;
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
      const auto p = SDL_FPoint{event.motion.x, event.motion.y};
      if (SDL_PointInRectFloat(&p, &st.board.m_board_rect)) {
        const auto tile_x =
            (p.x - st.board.m_position.x) / st.board.m_tile_width;
        const auto tile_y =
            (p.y - st.board.m_position.y) / st.board.m_tile_height;
        st.board.set_selected(tile_x, tile_y);
      } else {
        st.board.unselect();
      }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      if (event.button.button == SDL_BUTTON_LEFT) {
        if (st.board.m_selected_tile) {
          st.board.m_selected_tile->m_type = TileType::Road;
          st.board.m_selected_tile->m_road_connections |=
              static_cast<uint8_t>(RoadConnections::Up);
          st.board.m_selected_tile->m_road_connections |=
              static_cast<uint8_t>(RoadConnections::Down);
        }
      }
    }
  }
}

int main() {
  State state{};

  const size_t res_x = 1920;
  const size_t res_y = 1080;
  const char *title = "Dragons Aside";

  SDL_SetAppMetadata(title, "0.1", "org.athrail.dragonsaside");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer(title, res_x, res_y, 0, &state.window,
                                   &state.renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return 1;
  }

  state.board.init(res_x, res_y);

  while (state.m_running) {
    update(state);

    SDL_SetRenderDrawColorFloat(state.renderer, 0, 0, 0,
                                SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state.renderer);

    state.board.render(state.renderer);

    SDL_RenderPresent(state.renderer);
  }

  return 0;
}
