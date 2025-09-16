#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <queue>
#include <random>
#include <vector>

enum struct TileType { None = 0, Equipment, Dragon, Road };

enum struct RoadConnections : uint8_t {
  Up = 1 << 0,
  Right = 1 << 1,
  Down = 1 << 2,
  Left = 1 << 3
};

std::random_device rd;
std::mt19937 eng{std::mt19937(rd())};
std::uniform_int_distribution<> distr{std::uniform_int_distribution<>(1, 15)};

int get_random() { return distr(eng); }

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
};

struct Board {
  static constexpr uint8_t m_board_width = 6;
  static constexpr uint8_t m_board_height = 8;
  static constexpr uint16_t m_board_size = m_board_width * m_board_height;

  float m_tile_width{};
  float m_tile_height{};
  std::array<Tile, m_board_size> m_tiles;
  std::vector<Tile> m_draw_pile;
  SDL_FPoint m_position{};
  SDL_FRect m_board_rect{};
  SDL_FRect m_entry_arrow{};
  SDL_FPoint m_entry_arrow_points[3];
  SDL_FRect m_exit_arrow{};
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

    m_entry_arrow = SDL_FRect{
        m_position.x - 50.0f,
        m_position.y + m_board_rect.h - (m_tile_height * 0.5f) - 7.5f,
        30,
        15,
    };
    m_entry_arrow_points[0] = SDL_FPoint{
        m_entry_arrow.x + m_entry_arrow.w - 5.0f, m_entry_arrow.y - 5.0f};
    m_entry_arrow_points[1] = SDL_FPoint{
        m_entry_arrow.x + m_entry_arrow.w + 5.0f, m_entry_arrow.y + 7.5f};
    m_entry_arrow_points[2] =
        SDL_FPoint{m_entry_arrow.x + m_entry_arrow.w - 5.0f,
                   m_entry_arrow.y + m_entry_arrow.h + 5.0f};

    constexpr size_t equipment_count = 3;
    for (size_t i{0}; i < equipment_count; ++i) {
      auto& tile = get_tile(get_random() % m_board_width, 1 + (get_random() % (m_board_height - 2)));
      tile.m_type = TileType::Equipment;
    }

    randomize_draw_pile();
  }

  void randomize_draw_pile() {
    m_draw_pile.clear();
    // Tiles:
    // 21 roads
    // 16 dragons
    // 3 knights equipment
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road,
             .m_road_connections = static_cast<uint8_t>(RoadConnections::Up)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Right)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Down)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Left)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Right)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Down) |
                              static_cast<uint8_t>(RoadConnections::Right)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Left) |
                              static_cast<uint8_t>(RoadConnections::Right)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Left)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Right) |
                              static_cast<uint8_t>(RoadConnections::Left)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Down) |
                              static_cast<uint8_t>(RoadConnections::Left)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Right) |
                              static_cast<uint8_t>(RoadConnections::Up)});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road,
             .m_road_connections = static_cast<uint8_t>(RoadConnections::Down) |
                                   static_cast<uint8_t>(RoadConnections::Up)});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road,
             .m_road_connections = static_cast<uint8_t>(RoadConnections::Left) |
                                   static_cast<uint8_t>(RoadConnections::Up)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Up) |
                              static_cast<uint8_t>(RoadConnections::Down)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Right) |
                              static_cast<uint8_t>(RoadConnections::Down)});
    m_draw_pile.push_back(Tile{
        .m_type = TileType::Road,
        .m_road_connections = static_cast<uint8_t>(RoadConnections::Left) |
                              static_cast<uint8_t>(RoadConnections::Down)});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road, .m_road_connections = 15});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road, .m_road_connections = 15});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road, .m_road_connections = 15});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road, .m_road_connections = 15});
    m_draw_pile.push_back(
        Tile{.m_type = TileType::Road, .m_road_connections = 15});

    constexpr size_t dragons_count = 16;
    for (size_t i{0}; i < dragons_count; ++i)
      m_draw_pile.push_back(Tile{.m_type = TileType::Dragon});

    std::ranges::shuffle(m_draw_pile, eng);
  }

  void test_tiles() {
    for (size_t i{0}; i < 16; ++i) {
      auto &tile = get_tile(i % m_board_width, i / m_board_width);
      tile.m_type = TileType::Road;
      tile.m_road_connections = i;
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

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0xFF, 0x0, 0x0, 0xFF);
    SDL_RenderRect(r, &m_entry_arrow);
    SDL_RenderLines(r, m_entry_arrow_points, 3);
  }
};

struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;
  TTF_Font *font;
  Board board{};
  bool m_running{true};
  Tile m_next_tile;
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
        if (st.board.m_selected_tile &&
            st.board.m_selected_tile->m_type == TileType::None) {
          if (!st.board.m_draw_pile.empty()) {
            st.m_next_tile.m_rect = st.board.m_selected_tile->m_rect;
            *st.board.m_selected_tile = st.m_next_tile;
            st.m_next_tile = st.board.m_draw_pile.back();
            st.board.m_draw_pile.pop_back();
          }
        }
      }
    }
  }
}

void render_text(SDL_Renderer *r, const char *text, TTF_Font *font, float x,
                 float y, SDL_Color &c) {
  auto text_surface = TTF_RenderText_Solid(font, text, 0, c);
  auto text_texture = SDL_CreateTextureFromSurface(r, text_surface);

  SDL_FRect d;
  d.x = x;
  d.y = y;
  d.w = text_texture->w;
  d.h = text_texture->h;

  SDL_RenderTexture(r, text_texture, 0, &d);

  SDL_DestroySurface(text_surface);
  SDL_DestroyTexture(text_texture);
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

  if (!TTF_Init()) {
    SDL_Log("Couldn't initialize SDL_ttf");
    return 1;
  }

  state.font =
      TTF_OpenFont(std::filesystem::path{"./NotoSans.ttf"}.c_str(), 16.0f);
  if (!state.font) {
    SDL_Log("Couldn't load font :(");
    return 1;
  }

  if (!SDL_CreateWindowAndRenderer(title, res_x, res_y, 0, &state.window,
                                   &state.renderer)) {
    SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    return 1;
  }

  state.board.init(res_x, res_y);
  state.m_next_tile = state.board.m_draw_pile.back();
  state.board.m_draw_pile.pop_back();

  const char *text = "Dragons Aside";
  SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};

  while (state.m_running) {
    update(state);

    SDL_SetRenderDrawColorFloat(state.renderer, 0, 0, 0,
                                SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(state.renderer);

    state.board.render(state.renderer);
    render_text(state.renderer, text, state.font, 10.0f, 10.0f, white);
    render_text(state.renderer, "Drawn tile:", state.font, 10.0f, 30.0f, white);
    state.m_next_tile.render(state.renderer);

    // SDL_RenderTexture(state.renderer, text_texture, NULL, &d);
    SDL_RenderPresent(state.renderer);
  }

  return 0;
}
