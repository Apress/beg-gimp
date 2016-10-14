/*
 * AutocropAll plug-in version 1.00
 * by Akkana Peck, adapted from Zealous Crop by Adam D. Moss.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libgimp/gimp.h>

#define PLUG_IN_PROC "plug-in-autocropall"


/* Declare local functions. */
static void        query         (void);
static void        run           (const gchar      *name,
                                  gint              nparams,
                                  const GimpParam  *param,
                                  gint             *nreturn_vals,
                                  GimpParam       **return_vals);

static inline gint colours_equal (const guchar     *col1,
                                  const guchar     *col2,
                                  gint              bytes);
static void        do_acrop      (GimpDrawable     *drawable,
                                  gint32            image_id);


GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static inline gint
colours_equal (const guchar *col1,
               const guchar *col2,
               gint          bytes)
{
  gint b;

  for (b = 0; b < bytes; b++)
    {
      if (col1[b] != col2[b])
        return FALSE;
    }

  return TRUE;
}

static void
query (void)
{
  static GimpParamDef args[] =
  {
    { GIMP_PDB_INT32,    "run-mode", "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE,    "image",    "Input image"                  },
    { GIMP_PDB_DRAWABLE, "drawable", "Input drawable"               }
  };

  gimp_install_procedure (PLUG_IN_PROC,
                          "Automagically crops unused space from"
                          "the edges of all layers of an image",
                          "",
                          "Akkana Peck",
                          "Akkana Peck",
                          "2005",
                          "Autocrop All Layers",
                          "RGB*, GRAY*, INDEXED*",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (args), 0,
                          args, NULL);

  gimp_plugin_menu_register (PLUG_IN_PROC, "<Image>/Image/Crop");
}

static void
run (const gchar      *name,
     gint              n_params,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[1];
  GimpDrawable      *drawable;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
  gint32             image_id;

  *nreturn_vals = 1;
  *return_vals  = values;

  run_mode = param[0].data.d_int32;

  if (run_mode == GIMP_RUN_NONINTERACTIVE)
    {
      if (n_params != 3)
        {
          status = GIMP_PDB_CALLING_ERROR;
        }
    }

  if (status == GIMP_PDB_SUCCESS)
    {
      /*  Get the specified drawable  */
      drawable = gimp_drawable_get(param[2].data.d_drawable);
      image_id = param[1].data.d_image;

      /*  Make sure that the drawable is gray or RGB or indexed  */
      if (gimp_drawable_is_rgb (drawable->drawable_id) ||
          gimp_drawable_is_gray (drawable->drawable_id) ||
          gimp_drawable_is_indexed (drawable->drawable_id))
        {
          gimp_progress_init ("Autocropping...");

          gimp_tile_cache_ntiles (1 +
                                  (drawable->width > drawable->height ?
                                  (drawable->width / gimp_tile_width()) :
                                  (drawable->height / gimp_tile_height())));

          do_acrop(drawable, image_id);

          if (run_mode != GIMP_RUN_NONINTERACTIVE)
            gimp_displays_flush ();

          gimp_drawable_detach (drawable);
        }
      else
        {
          status = GIMP_PDB_EXECUTION_ERROR;
        }
    }

  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

static void
do_acrop (GimpDrawable *drawable,
          gint32        image_id)
{
  GimpPixelRgn  srcPR;
  gint          iwidth, iheight;
  gint          x, y, l;
  guchar       *buffer;
  gint          xmin, xmax, ymin, ymax;
  gint         *layers = NULL;
  gint          numlayers;

  iwidth = gimp_image_width(image_id);
  iheight = gimp_image_height(image_id);

  /* Set each edge to the opposite side -- i.e. xmin (the left edge
   * of the final cropped image) starts at the right edge.
   */
  xmin = iwidth - 1;
  xmax = 1;
  ymin = iheight - 1;
  ymax = 1;

  buffer = g_malloc ((iwidth > iheight ? iwidth : iheight) * drawable->bpp);

  layers = gimp_image_get_layers (image_id, &numlayers);

  for (l=0; l<numlayers; ++l)
    {
      drawable = gimp_drawable_get(layers[l]);
      gint dwidth  = drawable->width;
      gint dheight = drawable->height;
      gint layerOffsetX, layerOffsetY;
      gint bytes = drawable->bpp;
      gint start;

      /* Relate the layer coordinates to the image coordinates */
      gimp_drawable_offsets (layers[l], &layerOffsetX, &layerOffsetY);

      /*  initialize the pixel region to this layer */
      gimp_pixel_rgn_init (&srcPR, drawable, 0, 0, dwidth, dheight,
                           FALSE, FALSE);

      /* Update ymin. If the layer offset is greater than ymin,
       * then the region to be cropped already contains the whole layer
       * and we don't have to look any farther.
       */
      start = 0;
      if (layerOffsetY < 0)
          start = -layerOffsetY;
      for (y = start; y < dheight && layerOffsetY + y < ymin; y++)
        {
          gimp_pixel_rgn_get_row (&srcPR, buffer, 0, y, dwidth);

          for (x = 0; x < dwidth * bytes; x += bytes)
            {
              if (!colours_equal (buffer, &buffer[x], bytes))
                {
                  /* convert this layer coordinate back to image coord */
                  ymin = y + layerOffsetY;
                  break;
                }
            }
        }

      /* Update ymax. If the layer's offset plus height is less than
       * ymax, then the region to be cropped already contains the
       * whole layer and we don't have to look any farther.
       */
      start = dheight - 1;
      if (layerOffsetY + dheight > iheight)
          start = iheight - layerOffsetY - 1;
      for (y = start; y > 0 && layerOffsetY + y > ymax; y--)
        {
          gimp_pixel_rgn_get_row (&srcPR, buffer, 0, y, dwidth);

          for (x = 0; x < dwidth * bytes; x += bytes)
            {
              if (!colours_equal (buffer, &buffer[x], bytes))
                {
                  /* convert this layer coordinate back to image coord */
                  ymax = y + layerOffsetY + 1;
                  break;
                }
            }
        }

      /* Update xmin. If the layer offset is greater than ymin,
       * then the region to be cropped already contains the whole layer
       * and we don't have to look any farther.
       */
      start = 0;
      if (layerOffsetX < 0)
          start = -layerOffsetX;
      for (x = start; x < dwidth && layerOffsetX + x < xmin; x++)
        {
          gimp_pixel_rgn_get_col (&srcPR, buffer, x, 0, dheight);

          for (y = 0; y < dheight * bytes; y += bytes)
            {
              if (!colours_equal (buffer, &buffer[y], bytes))
                {
                  /* convert this layer coordinate back to image coord */
                  xmin = x + layerOffsetX;
                  break;
                }
            }
        }

      /* Update xmax. If the layer's offset plus width is less than
       * xmax, then the region to be cropped already contains the
       * whole layer and we don't have to look any farther.
       */
      start = dwidth - 1;
      if (layerOffsetX + dwidth > iwidth)
          start = iwidth - layerOffsetX - 1;
      for (x = start; x > 0 && layerOffsetX + x > xmax; x--)
        {
          gimp_pixel_rgn_get_col (&srcPR, buffer, x, 0, dheight);

          for (y = 0; y < dheight * bytes; y += bytes)
            {
              if (!colours_equal (buffer, &buffer[y], bytes))
                {
                  /* convert this layer coordinate back to image coord */
                  xmax = x + layerOffsetX + 1;
                  break;
                }
            }
        }

      gimp_progress_update ((gdouble)l / numlayers);
    }

  if (xmin == 0 && xmax == iwidth &&
      ymin == 0 && ymax == iheight)
    {
      g_message ("Nothing to crop.");
      return;
    }

  gimp_image_undo_group_start (image_id);

  gimp_image_crop(image_id,
                  xmax - xmin, ymax - ymin,
                  xmin, ymin);

  g_free (layers);
  g_free (buffer);

  gimp_progress_update (1.00);
  gimp_image_undo_group_end (image_id);
}
