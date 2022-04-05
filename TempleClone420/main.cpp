#include <splashkit.h>

// const float GRAVITY = 0.08f;
// const float GRAVITY = 0.3f;
// const int MAX_SPEED = 15;
// const int JUMP_SPEED = -12;

const float GRAVITY = 0.2f;
const int MAX_SPEED = 10;
const int JUMP_SPEED = -7;

const int MAX_JUMPS = 2;

const int FOREGROUND_SCROLL_SPEED = -4;
const int BACKGROUND_SCROLL_SPEED = -1;

const int NUM_OF_POLES = 3;

typedef struct player_data {
  sprite sprite_data;
  int jumps;
  int score;
  bool is_dead;
} player_data;

struct scene_data {
  sprite background;
  sprite foreground;
};

typedef struct game_data {
  player_data player;
  scene_data scene;
  sprite poles[NUM_OF_POLES];
  int frame_count;
} game_data;

scene_data get_new_scene() {
  scene_data result;

  result.background = create_sprite(bitmap_named("Background"));
  sprite_set_x(result.background, 0);
  sprite_set_y(result.background, 0);
  sprite_set_dx(result.background, BACKGROUND_SCROLL_SPEED);

  result.foreground = create_sprite(bitmap_named("Foreground"), animation_script_named("ForegroundAnimations"));
  sprite_set_x(result.foreground, 0);
  sprite_set_y(result.foreground, screen_height() - sprite_height(result.foreground));
  sprite_set_dx(result.foreground, FOREGROUND_SCROLL_SPEED);
  sprite_start_animation(result.foreground, "Fire");

  return result;
}

player_data get_new_player() {
  player_data result;

  result.sprite_data = create_sprite(bitmap_named("Player"), animation_script_named("PlayerAnimations"));

  sprite_set_x(result.sprite_data, screen_width()/3 - sprite_width(result.sprite_data));
  sprite_set_y(result.sprite_data, screen_height()/2);
  sprite_start_animation(result.sprite_data, "Fly");

  result.jumps = 0;
  result.score = 0;

  return result;
}

sprite get_random_pole() {
  sprite result;

  result = create_sprite(bitmap_named("UpPole"));
  sprite_set_x(result, screen_width() + rnd(screen_width() * 2));
  sprite_set_y(result, screen_height() - sprite_height(result)/2 - rnd(bitmap_height(bitmap_named("Foreground"))));
  sprite_set_dx(result, FOREGROUND_SCROLL_SPEED);

  return result;
}

game_data set_up_game() {
  game_data result;

  result.player = get_new_player();
  result.scene = get_new_scene();

  for (int i = 0; i < NUM_OF_POLES; i++) {
    result.poles[i] = get_random_pole();
  }

  result.frame_count = 0;

  return result;
}

void update_velocity(player_data &player, scene_data scene) {
  sprite_set_dy(player.sprite_data, sprite_dy(player.sprite_data) + GRAVITY);

  if (sprite_dy(player.sprite_data) > MAX_SPEED) {
    sprite_set_dy(player.sprite_data, MAX_SPEED);
  } else if (sprite_dy(player.sprite_data) < -MAX_SPEED) {
    sprite_set_dy(player.sprite_data, -MAX_SPEED);
  }

  if (sprite_collision(player.sprite_data, scene.foreground) && sprite_dy(player.sprite_data) > 0) {
    sprite_set_dy(player.sprite_data, 0);
    player.jumps = MAX_JUMPS;
  }

}

void handle_input(player_data &player) {
  if (key_typed(SPACE_KEY) && player.jumps > 0) {
    sprite_set_dy(player.sprite_data, JUMP_SPEED);
    player.jumps -= 1;
  }
}

void update_scene(scene_data &scene) {
  update_sprite(scene.background);
  update_sprite(scene.foreground);

  if(sprite_x(scene.background) <= -(sprite_width(scene.background)/2)) {
    sprite_set_x(scene.background, 0);
  }

  if(sprite_x(scene.foreground) <= -(sprite_width(scene.foreground)/2)) {
    sprite_set_x(scene.foreground, 0);
  }
}

void reset_pole_data(sprite &pole) {
  free_sprite(pole);
  pole = get_random_pole();
}

void check_pole_overlap(sprite &pole, sprite poles[]) {
  for (int i = 0; i < NUM_OF_POLES; i++) {
    if (sprite_collision(pole, poles[i]) && pole != poles[i]) {
      reset_pole_data(pole);
    }
  }
}

void check_for_collisions(game_data &game) {
  for (int i = 0; i < NUM_OF_POLES; ++i) {
    if (sprite_collision(game.player.sprite_data, game.poles[i])) {
      game.player.is_dead = true;
      return;
    }
  }
}

void reset_player(player_data &player) {
  free_sprite(player.sprite_data);
  player = get_new_player();
}

void reset_game(game_data &game) {
  reset_player(game.player);
  for (int i = 0; i < NUM_OF_POLES; ++i) {
    reset_pole_data(game.poles[i]);
  }
}

void update_game(game_data &game) {
  if (!game.player.is_dead) {
    check_for_collisions(game);
    update_velocity(game.player, game.scene);
    handle_input(game.player);
    update_sprite(game.player.sprite_data);
    update_scene(game.scene);

    if(game.frame_count % 15 == 0) {
      game.player.score++;
    }

    for (int i = 0; i < NUM_OF_POLES; i++) {
      update_sprite(game.poles[i]);
      check_pole_overlap(game.poles[i], game.poles);
      if (sprite_x(game.poles[i]) < (0 - sprite_width(game.poles[i]))) {
        reset_pole_data(game.poles[i]);
      }
    }
  } else {
    reset_game(game);
  }
  
}

int main() {
  // initialisation
  open_window("Infinite Runner", 768, 432);
  load_resource_bundle("CaveEscape", "cave_escape.txt");

  game_data game = set_up_game();

  do {
    // update
    process_events();
    update_game(game);

    game.frame_count++;

    // pre-draw
    clear_screen(COLOR_WHITE);

    // draw
    draw_sprite(game.scene.background);
    draw_sprite(game.player.sprite_data);
    for (int i = 0; i < NUM_OF_POLES; i++) {
      draw_sprite(game.poles[i]);
    }
    draw_sprite(game.scene.foreground);

    char str[15];
    sprintf(str, "%d", game.player.score);

    draw_text(str, COLOR_WHITE, "GameFont", 21, 10, 10);

    // post-draw
    refresh_screen();

  } while (!window_close_requested("Infinite Runner"));

  // cleanup
  free_resource_bundle("CaveEscape");
  close_window("Infinite Runner");

  return 0;
}
