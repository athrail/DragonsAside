#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <array>
#include <cstdint>
#include <cstdio>

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

  bool has_road_connection(RoadConnections &con) {
    return !!(m_road_connections & static_cast<uint8_t>(con));
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

    for (int x{0}; x < m_board_width; ++x) {
      for (int y{0}; y < m_board_height; ++y) {
        if (get_tile(x, y).m_selected)
          SDL_SetRenderDrawColor(r, 0x0, 0xFF, 0x0, 0xFF);
        else
          SDL_SetRenderDrawColor(r, 0x18, 0x18, 0x18, 0xFF);
        const auto rect = SDL_FRect{m_position.x + (x * m_tile_width),
                                    m_position.y + (y * m_tile_height),
                                    m_tile_width, m_tile_height};
        SDL_RenderRect(r, &rect);
      }
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
    }
  }
}

int main() {
  State state{};

  const size_t res_x = 1280;
  const size_t res_y = 720;
  const char *title = "Dragons Aside";

  if (res_x > res_y) {
    const float tile_size =
        static_cast<float>(res_y) / state.board.m_board_height;
    state.board.m_tile_height = state.board.m_tile_width = tile_size;
    state.board.m_position = SDL_FPoint{
        (res_x * 0.5f) -
            ((state.board.m_board_width * state.board.m_tile_width) * 0.5f),
        0};
  } else {
    const float tile_size =
        static_cast<float>(res_x) / state.board.m_board_width;
    state.board.m_tile_height = state.board.m_tile_width = tile_size;
    state.board.m_position = SDL_FPoint{
        0,
        (res_y * 0.5f) -
            ((state.board.m_board_height * state.board.m_tile_height) * 0.5f),
    };
  }
  state.board.m_board_rect =
      SDL_FRect{state.board.m_position.x, state.board.m_position.y,
                state.board.m_board_width * state.board.m_tile_width,
                state.board.m_board_height * state.board.m_tile_height};

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
