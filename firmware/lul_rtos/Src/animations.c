#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "animations.h"
#include "neopixel.h"
#include "shared.h"

uint32_t frame_counter;
uint8_t pixel_map[NEOPIXEL_COUNT] = {2, 1, 0, 3, 4, 5, 8, 7, 6, 9, 10, 11, 14, 13, 12};
uint8_t red_buf[NEOPIXEL_COUNT];
uint8_t green_buf[NEOPIXEL_COUNT];
uint8_t blue_buf[NEOPIXEL_COUNT];
led_animation neo_anime[NEOPIXEL_COUNT];

void set_pixel_index(uint8_t which, uint8_t r, uint8_t g, uint8_t b)
{
  red_buf[pixel_map[which]] = r;
  green_buf[pixel_map[which]] = g;
  blue_buf[pixel_map[which]] = b;
}

void randcolor(uint8_t* red, uint8_t* blue, uint8_t* green)
{
  while(1) 
  {
    *red = rand() % 256;
    *blue = rand() % (256 - *red);
    *green = 256 - *red - *blue;
    if(*red <= 50 || *green <= 50 || *blue <= 50)
      return;
  }
}

// this runs every single frame
void led_animation_handler(void)
{
  frame_counter++;
  for (int hhhhhh = 0; hhhhhh < NEOPIXEL_COUNT; ++hhhhhh)
  {
    uint32_t current_frame = frame_counter - neo_anime[hhhhhh].animation_start;
    if(neo_anime[hhhhhh].animation_type == ANIMATION_NO_ANIMATION)
      continue;
    else if(neo_anime[hhhhhh].animation_type == ANIMATION_FULLY_ON)
      set_pixel_index(neo_anime[hhhhhh].index, neo_anime[hhhhhh].target_color[0], neo_anime[hhhhhh].target_color[1], neo_anime[hhhhhh].target_color[2]);
    else if(neo_anime[hhhhhh].animation_type == ANIMATION_CROSS_FADE)
    {
      if(current_frame <= neo_anime[hhhhhh].animation_duration)
      {
        for (int i = 0; i < THREE; ++i)
        {
          neo_anime[hhhhhh].current_color[i] += neo_anime[hhhhhh].step[i];
          if(neo_anime[hhhhhh].current_color[i] > 255)
            neo_anime[hhhhhh].current_color[i] = 255;
          if(neo_anime[hhhhhh].current_color[i] < 0)
            neo_anime[hhhhhh].current_color[i] = 0;
        }
      }
      else
      {
         for (int i = 0; i < THREE; ++i)
          neo_anime[hhhhhh].current_color[i] = neo_anime[hhhhhh].target_color[i];
      }
      set_pixel_index(neo_anime[hhhhhh].index, neo_anime[hhhhhh].current_color[0], neo_anime[hhhhhh].current_color[1], neo_anime[hhhhhh].current_color[2]);
    }
    else if(neo_anime[hhhhhh].animation_type == ANIMATION_MY_RAND)
    {
      if(current_frame % (ANIME_FPS * 5) == 0)
        for (int i = 0; i < THREE; ++i)
          neo_anime[hhhhhh].action[i] = rand() % 3;
      for (int i = 0; i < THREE; ++i)
      {
        if(neo_anime[hhhhhh].action[i] == ANIME_INCREASE && neo_anime[hhhhhh].target_color[i] < 255)
          neo_anime[hhhhhh].target_color[i]++;
        else if(neo_anime[hhhhhh].action[i] == ANIME_DECREASE && neo_anime[hhhhhh].target_color[i] > 0)
          neo_anime[hhhhhh].target_color[i]--;
      }
      set_pixel_index(neo_anime[hhhhhh].index, neo_anime[hhhhhh].target_color[0], neo_anime[hhhhhh].target_color[1], neo_anime[hhhhhh].target_color[2]);
    }
  }
  taskENTER_CRITICAL();
  neopixel_show(red_buf, green_buf, blue_buf);
  taskEXIT_CRITICAL();
}

void animation_test(void)
{
  ;
}

void led_animation_init(led_animation* anime_struct, uint8_t index)
{
  for (int i = 0; i < THREE; ++i)
  {
    anime_struct->current_color[i] = 0;
    anime_struct->step[i] = 0;
    anime_struct->target_color[i] = 0;
    anime_struct->action[i] = rand() % 3;
  }
  anime_struct->animation_start = 0;
  anime_struct->animation_duration = 0;
  anime_struct->animation_type = ANIMATION_NO_ANIMATION;
  anime_struct->index = index;
}

void anime_init(void)
{
  uint8_t colors[THREE] = {64, 64, 64};
  // randcolor(&colors[0], &colors[1], &colors[2]);
  for (int i = 0; i < NEOPIXEL_COUNT; ++i)
    led_animation_init(&neo_anime[i], i);
  led_start_animation(&neo_anime[0], colors, ANIMATION_MY_RAND, 0);
}

void led_start_animation(led_animation* anime_struct, uint8_t dest_color[THREE], uint8_t anime_type, uint8_t durations_frames)
{
  for (int i = 0; i < THREE; ++i)
    anime_struct->step[i] = (dest_color[i] - anime_struct->current_color[i]) / (double)durations_frames;
  memcpy(anime_struct->target_color, dest_color, THREE);
  anime_struct->animation_start = frame_counter;
  anime_struct->animation_type = anime_type;
  anime_struct->animation_duration = durations_frames;
}
