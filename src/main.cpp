#include <SDL3/SDL.h>
#include <SDL3/SDL_blendmode.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <random>
#include <vector>

#include "board.hpp"
#include "tile.hpp"

std::random_device rd;
std::mt19937 eng{std::mt19937(rd())};
std::uniform_int_distribution<> distr{std::uniform_int_distribution<>(1, 15)};

int get_random() { return distr(eng); }

struct State {
  SDL_Renderer *renderer;
  SDL_Window *window;
  TTF_Font *font;
  Board board{};
  bool m_running{true};
  Tile m_next_tile;
  uint8_t m_eq_count{0};
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
          auto tile_type = st.board.m_selected_tile->m_type;
          if ((tile_type == TileType::None) ||
              (tile_type == TileType::Equipment)) {
            if (st.m_next_tile.m_type != TileType::None) {
              SDL_FRect drawn_tile_rect = st.m_next_tile.m_rect;
              st.m_next_tile.m_rect = st.board.m_selected_tile->m_rect;
              *st.board.m_selected_tile = st.m_next_tile;
              st.m_next_tile.m_type = TileType::None;

              if (st.board.m_draw_pile.empty())
                return;

              st.m_next_tile = st.board.m_draw_pile.back();
              st.m_next_tile.m_rect = drawn_tile_rect;
              st.board.m_draw_pile.pop_back();
            }
          }
        }
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
        st.m_next_tile.rotate();
      }
    }
  }

  while (st.m_next_tile.m_type == TileType::Dragon) {
    auto &random_tile =
        st.board.get_tile(get_random() % st.board.m_board_width,
                          1 + (get_random() % (st.board.m_board_height - 2)));
    if (random_tile.m_type == TileType::None) {
      SDL_FRect drawn_tile_rect = st.m_next_tile.m_rect;
      st.m_next_tile.m_rect = random_tile.m_rect;
      random_tile = st.m_next_tile;
      st.m_next_tile.m_type = TileType::None;

      if (st.board.m_draw_pile.empty())
        return;

      st.m_next_tile = st.board.m_draw_pile.back();
      st.m_next_tile.m_rect = drawn_tile_rect;
      st.board.m_draw_pile.pop_back();
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
  SDL_FRect drawn_tile_rect{10.0f, 60.0f, state.board.m_tile_width,
                            state.board.m_tile_height};
  state.m_next_tile.m_rect = drawn_tile_rect;

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

    SDL_RenderPresent(state.renderer);
  }

  return 0;
}
