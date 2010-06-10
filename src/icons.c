/* SF-Lib - Icons.c
 *
 * Version 0.10 (5 May 2003)
 */

/* OS-Lib header files. */

#include "oslib/wimp.h"
#include "oslib/os.h"

/* SF-Lib header files. */

#include "icons.h"
#include "string.h"
#include "debug.h"
#include "string.h"

/* ANSII C header files. */

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/* ================================================================================================================== */

char *icon_text (wimp_w w, wimp_i i, char *buffer)
{
  wimp_icon_state icon;


  icon.w = w;
  icon.i = i;
  wimp_get_icon_state (&icon);

  if (icon.icon.flags & wimp_ICON_INDIRECTED)
  {
    ctrl_strcpy (buffer, icon.icon.data.indirected_text.text);
    return (buffer);
  }
  else
  {
    ctrl_strcpy (buffer, icon.icon.data.text);
    return (buffer);
  }
}

/* ------------------------------------------------------------------------------------------------------------------ */

char *indirected_icon_text (wimp_w w, wimp_i i)
{
  wimp_icon_state icon;


  icon.w = w;
  icon.i = i;
  wimp_get_icon_state (&icon);

  if (icon.icon.flags & wimp_ICON_INDIRECTED)
  {
    return (icon.icon.data.indirected_text.text);
  }
  else
  {
    return (NULL);
  }
}


/* ------------------------------------------------------------------------------------------------------------------ */

char *indirected_icon_validation (wimp_w w, wimp_i i)
{
  wimp_icon_state icon;


  icon.w = w;
  icon.i = i;
  wimp_get_icon_state (&icon);

  if ((icon.icon.flags & (wimp_ICON_INDIRECTED | wimp_ICON_TEXT)) == (wimp_ICON_INDIRECTED | wimp_ICON_TEXT))
  {
    return (icon.icon.data.indirected_text.validation);
  }
  else
  {
    return (NULL);
  }
}

/* ================================================================================================================== */

/* Returns 1 if the icon was not validated. */

int get_validation_command (char *buffer, wimp_w w, wimp_i i, char command)
{
  char *val, *copy, *part;

  *buffer = '\0';

  val = indirected_icon_validation (w, i);

  if (val != NULL)
  {
    copy = (char *) malloc (strlen (val) + 1);

    if (copy != NULL)
    {
      ctrl_strcpy (copy, val);

      part = strtok (copy, ";");

      while (part != NULL)
      {
        if (toupper (*part) == toupper (command))
        {
          ctrl_strcpy (buffer, part + 1);
        }

        part = strtok (NULL, ";");
      }

      free (copy);
    }

    return (0);
  }
  else
  {
    return (1);
  }
}

/* ================================================================================================================== */

void set_icon_selected (wimp_w w, wimp_i i, int selected)
{
  int flags;


  flags = selected ? wimp_ICON_SELECTED : 0;
  wimp_set_icon_state (w, i, flags, wimp_ICON_SELECTED);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void set_icon_shaded (wimp_w w, wimp_i i, int shaded)
{
  int flags;


  flags = shaded ? wimp_ICON_SHADED : 0;
  wimp_set_icon_state (w, i, flags, wimp_ICON_SHADED);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void set_icon_deleted (wimp_w w, wimp_i i, int deleted)
{
  int flags;


  flags = deleted ? wimp_ICON_DELETED : 0;
  wimp_set_icon_state (w, i, flags, wimp_ICON_DELETED);
}

/* ================================================================================================================== */

int read_icon_selected (wimp_w w, wimp_i i)
{
  wimp_icon_state icon;


  icon.w = w;
  icon.i = i;
  wimp_get_icon_state (&icon);

  return ((icon.icon.flags & wimp_ICON_SELECTED) != 0);
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_icon_shaded (wimp_w w, wimp_i i)
{
  wimp_icon_state icon;


  icon.w = w;
  icon.i = i;
  wimp_get_icon_state (&icon);

  return ((icon.icon.flags & wimp_ICON_SHADED) != 0);
}

/* ================================================================================================================== */

void set_radio_icon_group_selected (wimp_w window, int selected, int icons, ...)
{
  int     i;
  va_list ap;


  va_start (ap, icons);

  for (i=0; i<icons; i++)
  {
    set_icon_selected (window, va_arg (ap, wimp_i), selected == i);
  }

  va_end (ap);
}

/* ------------------------------------------------------------------------------------------------------------------ */

int read_radio_icon_group_selected (wimp_w window, int icons, ...)
{
  int     i, selected;
  va_list ap;

  selected = -1;

  va_start (ap, icons);

  for (i=0; i<icons; i++)
  {
    if (read_icon_selected (window, va_arg (ap, wimp_i)))
    {
      selected = i;
    }
  }

  va_end (ap);

  return (selected);
}

/* ================================================================================================================== */

void set_icons_shaded (wimp_w window, int shaded, int icons, ...)
{
  int     i;
  va_list ap;


  va_start (ap, icons);

  for (i=0; i<icons; i++)
  {
    set_icon_shaded (window, va_arg (ap, wimp_i), shaded);
  }

  va_end (ap);
}

/* ================================================================================================================== */

void set_icons_shaded_when_radio_off (wimp_w window, wimp_i icon, int icons, ...)
{
  int     i, state;
  va_list ap;


  va_start (ap, icons);

  state = !read_icon_selected (window, icon);

  for (i=0; i<icons; i++)
  {
    set_icon_shaded (window, va_arg (ap, wimp_i), state);
  }

  va_end (ap);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void set_icons_shaded_when_radio_on (wimp_w window, wimp_i icon, int icons, ...)
{
  int     i, state;
  va_list ap;


  va_start (ap, icons);

  state = read_icon_selected (window, icon);

  for (i=0; i<icons; i++)
  {
    set_icon_shaded (window, va_arg (ap, wimp_i), state);
  }

  va_end (ap);
}

/* ================================================================================================================== */

void put_caret_at_end (wimp_w window, wimp_i icon)
{
  int             index = 0;
  wimp_icon_state icon_state;


  if (icon != wimp_ICON_WINDOW)
  {
    icon_state.w = window;
    icon_state.i = icon;
    wimp_get_icon_state (&icon_state);

    index = ctrl_strlen (icon_state.icon.data.indirected_text.text);
  }

  wimp_set_caret_position (window, icon, NULL, NULL, -1, index);
}

/* ------------------------------------------------------------------------------------------------------------------ */

void replace_caret_in_window (wimp_w window)
{
  int              i, old_icon, new_icon, type;
  wimp_caret       caret;
  wimp_window_info *info, *new;


  wimp_get_caret_position (&caret);

  if (caret.w == window && caret.i != wimp_ICON_WINDOW)
  {
    if (read_icon_shaded (caret.w, caret.i))
    {
      /* If the icon where the caret is located is now shaded, it needs to be moved.  To do this, we need to know
       * where the other writable icons are.  This is done by getting the window definition and scanning through
       * the icons looking for a writable one to place the caret into.  The numerically nearest icon to the
       * original is used.
       */

      /* First, claim some memory to get the definition into.  This is big enough for the header only info
       * (to find out how many icons there are) and may also do for the full definition.
       */

      info = (wimp_window_info *) malloc (4096);

      if (info != NULL)
      {
        /* We got some memory, so get the window definitin header and check the number of icons. */

        info->w = window;
        wimp_get_window_info_header_only (info);

        if ((88 + 32 * info->icon_count) > sizeof (info))
        {
          /* If there are too many icons to fit out block, expand it.  If that fails, the original block is freed
           * and the pointer set to NULL.
           */

          new = (wimp_window_info *) realloc (info, (88 + 32 * info->icon_count));

          if (new == NULL)
          {
            free (info);
          }

          info = new;
        }

        if (info != NULL)
        {
          /* we've got enough memory to take the full window definition, so get that. */

          info->w = window;
          wimp_get_window_info (info);

          old_icon = caret.i;
          new_icon = -1;

          /* For each icon, see if it's writable and not shaded.  If so, check if it's closer to the original
           * icon than the current 'new_icon'; if so, update the new icon.
           */

          for (i=0; i<info->icon_count; i++)
          {
            type = ((info->icons[i].flags & wimp_ICON_BUTTON_TYPE) >> wimp_ICON_BUTTON_TYPE_SHIFT);

            if ((type == wimp_BUTTON_WRITE_CLICK_DRAG || type == wimp_BUTTON_WRITABLE) &&
                ((info->icons[i].flags & wimp_ICON_SHADED) == 0))
            {

              if ((abs (old_icon - i) < abs (old_icon - new_icon)) || new_icon == -1)
              {
                new_icon = i;
              }
            }
          }

          /* If we found a suitable new icon, put the caret at the end; if not, dump it into the work area. */

          if (new_icon != -1)
          {
            put_caret_at_end (caret.w, new_icon);
          }
          else
          {
            put_caret_at_end (caret.w, wimp_ICON_WINDOW);
          }

          free (info);
        }
        else
        {
          /* If the block expansion failed, fall back to dumping the caret into the workspace. */

          put_caret_at_end (caret.w, wimp_ICON_WINDOW);
        }
      }
      else
      {
        /* If the initial memory allocation failed, fall back to dumping the caret into the workspace. */

        put_caret_at_end (caret.w, wimp_ICON_WINDOW);
      }
    }
    else
    {
      put_caret_at_end (caret.w, caret.i);
    }
  }
}

/* ================================================================================================================== */

void redraw_icons_in_window (wimp_w w, int icons, ...)
{
  int     i;
  va_list ap;


  va_start (ap, icons);

  for (i=0; i<icons; i++)
  {
    wimp_set_icon_state (w, va_arg (ap, wimp_i), 0, 0);
  }

  va_end (ap);
}

/* ================================================================================================================== */

void insert_text_into_icon (wimp_w w, wimp_i i, int index, char *text, int n)
{
  wimp_icon_state icon_state;
  int             len, copy, j;


  icon_state.w = w;
  icon_state.i = i;
  wimp_get_icon_state (&icon_state);

  /* Calculate the length of the current icon text (including terminator), the amount of free space and hence the
   * amount of the new text to be copied in.
   */

  len = ctrl_strlen (icon_state.icon.data.indirected_text.text) + 1;
  copy = icon_state.icon.data.indirected_text.size - len;
  if (copy > n)
  {
    copy = n;
  }

  /* Move up the text following the insertion point. */

  if (index < len)
  {
    memmove (icon_state.icon.data.indirected_text.text+index+copy, icon_state.icon.data.indirected_text.text+index,
             len - index);
  }

  /* Copy in the new text, until all the characters have been copied or a null is reached.  Set the caret to be
   * after the copied text and redraw the icon.
   */

  for (j = 0; text[j] != '\0' && j < copy; j++)
  {
    icon_state.icon.data.indirected_text.text[index + j] = text[j];
  }

  wimp_set_caret_position (w, i, NULL, NULL, -1, index+copy);
  wimp_set_icon_state (w, i, 0, 0);
}
