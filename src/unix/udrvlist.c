/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Dynamic driver lists shared by Unixy system drivers.
 *
 *      By Peter Wang.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro5/allegro.h"
#include "allegro5/internal/aintern.h"
#include "allegro5/internal/aintern_bitmap.h"
#include "allegro5/internal/aintern_system.h"

#if defined ALLEGRO_WITH_XWINDOWS
#ifdef ALLEGRO_RASPBERRYPI
#include "allegro5/internal/aintern_raspberrypi.h"
#elif defined ALLEGRO_PANDORA
#include "allegro5/internal/aintern_pandora.h"
#elif defined ALLEGRO_ODROID
#include "allegro5/internal/aintern_odroid.h"
#else
#include "allegro5/platform/aintxglx.h"
#endif
#endif



/* This is a function each platform must define to register all available
 * system drivers.
 */
void _al_register_system_interfaces(void)
{
   ALLEGRO_SYSTEM_INTERFACE **add;
#if defined ALLEGRO_WITH_XWINDOWS && !defined ALLEGRO_RASPBERRYPI && !defined ALLEGRO_PANDORA && !defined ALLEGRO_ODROID
   add = _al_vector_alloc_back(&_al_system_interfaces);
   *add = _al_system_xglx_driver();
#elif defined ALLEGRO_RASPBERRYPI
   add = _al_vector_alloc_back(&_al_system_interfaces);
   *add = _al_system_raspberrypi_driver();
#elif defined ALLEGRO_PANDORA
   add = _al_vector_alloc_back(&_al_system_interfaces);
   *add = _al_system_pandora_driver();
#elif defined ALLEGRO_ODROID
   add = _al_vector_alloc_back(&_al_system_interfaces);
   *add = _al_system_odroid_driver();
#endif
}

