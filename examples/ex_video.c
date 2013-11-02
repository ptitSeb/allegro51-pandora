#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_video.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include <stdio.h>

#include "common.c"

static ALLEGRO_DISPLAY *screen;
static ALLEGRO_FONT *font;
static char const *filename;
static float zoom = 0;

static void video_display(ALLEGRO_VIDEO *video)
{
   /* Videos often do not use square pixels - this returns the aspect
    * ratio of the pixels.
    */
   float aspect_ratio = al_get_video_aspect_ratio(video);
   /* Get the currently visible frame of the video, based on clock
    * time.
    */
   ALLEGRO_BITMAP *frame = al_get_video_frame(video);
   int w, h, x, y;
   ALLEGRO_COLOR tc = al_map_rgba_f(0, 0, 0, 0.5);
   ALLEGRO_COLOR bc = al_map_rgba_f(0.5, 0.5, 0.5, 0.5);
   double p;

   if (!frame)
      return;

   if (zoom == 0 && aspect_ratio > 0.0f) {
      /* Always make the video fit into the window. */
      h = al_get_display_height(screen);
      w = (int)(h * aspect_ratio);
      if (w > al_get_display_width(screen)) {
         w = al_get_display_width(screen);
         h = (int)(w / aspect_ratio);
      }
   }
   else {
      w = al_get_video_width(video);
      h = al_get_video_height(video);
   }
   x = (al_get_display_width(screen) - w) / 2;
   y = (al_get_display_height(screen) - h) / 2;

   /* Display the frame. */
   al_draw_scaled_bitmap(frame, 0, 0,
                         al_get_bitmap_width(frame),
                         al_get_bitmap_height(frame), x, y, w, h, 0);

   /* Show some video information. */
   al_draw_filled_rounded_rectangle(4, 4,
      al_get_display_width(screen) - 4, 4 + 14 * 3, 8, 8, bc);
   p = al_get_video_position(video, 0);
   al_draw_textf(font, tc, 8, 8 , 0, "%s", filename);
   al_draw_textf(font, tc, 8, 8 + 13, 0, "%3d:%02d (V: %+5.2f A: %+5.2f)",
      (int)(p / 60),
      ((int)p) % 60,
      al_get_video_position(video, 1) - p,
      al_get_video_position(video, 2) - p);
   al_draw_textf(font, tc, 8, 8 + 13 * 2, 0,
      "video rate %.02f (%dx%d, aspect %.1f) audio rate %.0f",
         al_get_video_fps(video),
         al_get_video_width(video),
         al_get_video_height(video),
         al_get_video_aspect_ratio(video),
         al_get_video_audio_rate(video));
   al_flip_display();
   al_clear_to_color(al_map_rgb(0, 0, 0));
}


int main(int argc, char *argv[])
{
   ALLEGRO_EVENT_QUEUE *queue;
   ALLEGRO_EVENT event;
   ALLEGRO_TIMER *timer;
   ALLEGRO_VIDEO *video;
   bool fullscreen = false;
   bool redraw = true;

   if (!al_init()) {
      abort_example("Could not init Allegro.\n");
   }

   open_log();

   if (argc < 2) {
      log_printf("This example needs to be run from the command line.\nUsage: %s <file>\n", argv[0]);
      goto done;
   }

   al_init_font_addon();
   al_install_keyboard();

   al_install_audio();
   al_reserve_samples(1);
   al_init_primitives_addon();
   
   /* In this example we use a fixed FPS timer. If the video is
    * displayed in a game this probably makes most sense. In a
    * dedicated video player you probably want to listen to
    * ALLEGRO_EVENT_VIDEO_FRAME events and only redraw whenever one
    * arrives - to reduce possible jitter and save CPU.
    */
   timer = al_create_timer(1.0 / 60);

   al_set_new_display_flags(ALLEGRO_RESIZABLE);
   al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
   screen = al_create_display(640, 480);
   if (!screen) {
      abort_example("Could not set video mode - exiting\n");
   }
   
   font = al_create_builtin_font();
   if (!font) {
      abort_example("No font.\n");
   }

   al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);

   filename = argv[1];
   video = al_open_video(filename);
   if (!video) {
      abort_example("Cannot read %s.\n", filename);
   }
   log_printf("video FPS: %f\n", al_get_video_fps(video));
   log_printf("video audio rate: %f\n", al_get_video_audio_rate(video));
   log_printf(
      "keys:\n"
      "Space: Play/Pause\n"
      "cursor right/left: seek 10 seconds\n"
      "cursor up/down: seek one minute\n"
      "F: toggle fullscreen\n"
      "1: disable scaling\n"
      "S: scale to window\n");

   queue = al_create_event_queue();
   al_register_event_source(queue, al_get_video_event_source(video));
   al_register_event_source(queue, al_get_display_event_source(screen));
   al_register_event_source(queue, al_get_timer_event_source(timer));
   al_register_event_source(queue, al_get_keyboard_event_source());

   al_start_video(video, al_get_default_mixer());
   al_start_timer(timer);
   for (;;) {
      double incr;

      if (redraw && al_event_queue_is_empty(queue)) {
         video_display(video);
         redraw = false;
      }

      al_wait_for_event(queue, &event);
      switch (event.type) {
         case ALLEGRO_EVENT_KEY_DOWN:
            switch (event.keyboard.keycode) {
               case ALLEGRO_KEY_SPACE:
                  al_pause_video(video, !al_is_video_paused(video));
                  break;
               case ALLEGRO_KEY_ESCAPE:
                  al_close_video(video);
                  goto done;
                  break;
               case ALLEGRO_KEY_LEFT:
                  incr = -10.0;
                  goto do_seek;
               case ALLEGRO_KEY_RIGHT:
                  incr = 10.0;
                  goto do_seek;
               case ALLEGRO_KEY_UP:
                  incr = 60.0;
                  goto do_seek;
               case ALLEGRO_KEY_DOWN:
                  incr = -60.0;
                  goto do_seek;

               do_seek:
                  al_seek_video(video, al_get_video_position(video, 0) + incr);
                  break;

               case ALLEGRO_KEY_F:
                  fullscreen = !fullscreen;
                  al_set_display_flag(screen, ALLEGRO_FULLSCREEN_WINDOW,
                     fullscreen);
                  break;
               
               case ALLEGRO_KEY_1:
                  zoom = 1;
                  break;

               case ALLEGRO_KEY_S:
                  zoom = 0;
                  break;
               default:
                  break;
            }
            break;
         
         case ALLEGRO_EVENT_DISPLAY_RESIZE:
            al_acknowledge_resize(screen);
            al_clear_to_color(al_map_rgb(0, 0, 0));
            break;

         case ALLEGRO_EVENT_TIMER:
            /*
            display_time += 1.0 / 60;
            if (display_time >= video_time) {
               video_time = display_time + video_refresh_timer(is);
            }*/

            redraw = true;
            break;

         case ALLEGRO_EVENT_DISPLAY_CLOSE:
            al_close_video(video);
            goto done;
            break;

         /*case ALLEGRO_EVENT_VIDEO_FRAME:
            al_get_video_frame((void *)event.user.data1);
            break;*/
         default:
            break;
      }
   }
done:
   al_destroy_display(screen);
   close_log(true);
   return 0;
}

/* vim: set sts=3 sw=3 et: */
