/*
  loaders.c

  Functions responsible for loading multimedia.

  begin                : Thu May 4 2000
  copyright            : (C) 2000 by Sam Hart
                       : (C) 2003 by Jesse Andrews
  email                : tuxtype-dev@tux4kids.net

  Modified for use in tuxmath by David Bruce - 2006.
  email                : <davidstuartbruce@gmail.com>
                         <tuxmath-devel@lists.sourceforge.net>

  Modified to support SVG by Boleslaw Kulbabinski - 2009
  email                : <bkulbabinski@gmail.com>

  Part of "Tux4Kids" Project
  http://www.tux4kids.com/

  Copyright: See COPYING file that comes with this distribution.
*/

#include "loaders.h"
#include "globals.h"
#include "tuxmath.h"
#include "setup.h"       // for cleanup_on_error()
#include "SDL_extras.h"

#ifdef HAVE_RSVG
#include<librsvg/rsvg.h>
#include<librsvg/rsvg-cairo.h>
#endif

#include <png.h>
#include "pixels.h"

/* local functions */
int             check_file(const char* file);

#ifdef HAVE_RSVG
SDL_Surface*    load_svg(const char* file_name, int width, int height, const char* layer_name);
sprite*         load_svg_sprite(const char* file_name, int width, int height);
SDL_Surface*    render_svg_from_handle(RsvgHandle* file_handle, int width, int height, const char* layer_name);
void            get_svg_dimensions(const char* file_name, int* width, int* height);
#endif

SDL_Surface*    load_image(const char* file_name, int mode, int w, int h, bool proportional);
void            fit_in_rectangle(int* width, int* height, int max_width, int max_height);
SDL_Surface*    set_format(SDL_Surface* img, int mode);
sprite*         load_sprite(const char* name, int mode, int w, int h, bool proportional);

static int do_png_save(FILE * fi, const char *const fname, SDL_Surface * surf);
static void savePNG(SDL_Surface* surf,char* fn);


/* check to see if file exists, if so return true */
// int checkFile( const char *file ) {
//         static struct stat fileStats;
//         fileStats.st_mode = 0;
//         stat( file, &fileStats );
//         return (S_IFREG & fileStats.st_mode);
// }


/* Returns 1 if valid file, 2 if valid dir, 0 if neither: */
int check_file(const char* file)
{
  FILE* fp = NULL;
  DIR* dp = NULL;

  if (!file)
  {
    DEBUGMSG(debug_loaders, "check_file(): invalid char* argument!\n");
    return 0;
  }

  DEBUGMSG(debug_loaders, "check_file(): checking: %s\n", file);

  dp = opendir(file);
  if (dp)
  {
    DEBUGMSG(debug_loaders, "check_file(): Opened successfully as DIR\n");
    closedir(dp);
    return 2;
  }

  fp = fopen(file, "r");
  if (fp)
  {
    DEBUGMSG(debug_loaders, "check_file(): Opened successfully as FILE\n");
    fclose(fp);
    return 1;
  }

  DEBUGMSG(debug_loaders, "check_file(): Unable to open '%s' as either FILE or DIR\n", file);
  return 0;
}


#ifdef HAVE_RSVG

/* Load a layer of SVG file and resize it to given dimensions.
   If width or height is negative no resizing is applied.
   If layer = NULL then the whole image is loaded.
   layer_name must be preceded with a '#' symbol.
   Return NULL on failure.
   (partly based on TuxPaint's SVG loading function) */
SDL_Surface* load_svg(const char* file_name, int width, int height, const char* layer_name)
{
  SDL_Surface* dest;
  RsvgHandle* file_handle;

  DEBUGMSG(debug_loaders, "load_svg(): loading %s\n", file_name);

  rsvg_init();

  file_handle = rsvg_handle_new_from_file(file_name, NULL);
  if(NULL == file_handle)
  {
    DEBUGMSG(debug_loaders, "load_svg(): file %s not found\n", file_name);
    rsvg_term();
    return NULL;
  }

  dest = render_svg_from_handle(file_handle, width, height, layer_name);

  g_object_unref(file_handle);
  rsvg_term();

  return dest;
}

sprite* load_svg_sprite(const char* file_name, int width, int height)
{
  RsvgHandle* file_handle;
  sprite* new_sprite;
  char lay_name[20];
  int i;

  DEBUGMSG(debug_loaders, "load_svg_sprite(): loading sprite from %s\n", file_name);

  rsvg_init();

  file_handle = rsvg_handle_new_from_file(file_name, NULL);
  if(NULL == file_handle)
  {
    DEBUGMSG(debug_loaders, "load_svg_sprite(): file %s not found\n", file_name);
    rsvg_term();
    return NULL;
  }

  new_sprite = malloc(sizeof(sprite));
  new_sprite->default_img = render_svg_from_handle(file_handle, width, height, "#default");

  /* get number of frames from description */
  sscanf(rsvg_handle_get_desc(file_handle), "%d", &new_sprite->num_frames);
  DEBUGMSG(debug_loaders, "load_svg_sprite(): loading %d frames\n", new_sprite->num_frames);

  for(i = 0; i < new_sprite->num_frames; i++)
  {
    sprintf(lay_name, "#frame%d", i);
    new_sprite->frame[i] = render_svg_from_handle(file_handle, width, height, lay_name);
  }

  g_object_unref(file_handle);
  rsvg_term();

  return new_sprite;
}

/* render a layer of SVG file and resize it to given dimensions.
   If width or height is negative no resizing is applied. */
SDL_Surface* render_svg_from_handle(RsvgHandle* file_handle, int width, int height, const char* layer_name)
{
  RsvgDimensionData dimensions;
  cairo_surface_t* temp_surf;
  cairo_t* context;
  SDL_Surface* dest;
  float scale_x, scale_y;
  Uint32 Rmask, Gmask, Bmask, Amask;

  rsvg_handle_get_dimensions(file_handle, &dimensions);

  /* set scale_x and scale_y */
  if(width < 0 || height < 0)
  {
    width = dimensions.width;
    height = dimensions.height;
    scale_x = 1.0;
    scale_y = 1.0;
  }
  else
  {
    scale_x = (float)width / dimensions.width;
    scale_y = (float)height / dimensions.height;
  }

  /* set color masks */
  Rmask = screen->format->Rmask;
  Gmask = screen->format->Gmask;
  Bmask = screen->format->Bmask;
  if(screen->format->Amask == 0)
    /* find a free byte to use for Amask */
    Amask = ~(Rmask | Gmask | Bmask);
  else
    Amask = screen->format->Amask;

  DEBUGMSG(debug_loaders, "render_svg_from_handle(): color masks: R=%u, G=%u, B=%u, A=%u\n",
        Rmask, Gmask, Bmask, Amask);

  dest = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA,
        width, height, screen->format->BitsPerPixel, Rmask, Gmask, Bmask, Amask);

  SDL_LockSurface(dest);
  temp_surf = cairo_image_surface_create_for_data(dest->pixels,
        CAIRO_FORMAT_ARGB32, dest->w, dest->h, dest->pitch);

  context = cairo_create(temp_surf);
  if(cairo_status(context) != CAIRO_STATUS_SUCCESS)
  {
    DEBUGMSG(debug_loaders, "render_svg_from_handle(): error rendering SVG\n");
    cairo_surface_destroy(temp_surf);
    return NULL;
  }

  cairo_scale(context, scale_x, scale_y);

  /* render appropriate layer */
  rsvg_handle_render_cairo_sub(file_handle, context, layer_name);

  SDL_UnlockSurface(dest);
  cairo_surface_destroy(temp_surf);
  cairo_destroy(context);

  return dest;
}

void get_svg_dimensions(const char* file_name, int* width, int* height)
{
  RsvgHandle* file_handle;
  RsvgDimensionData dimensions;

  rsvg_init();

  file_handle = rsvg_handle_new_from_file(file_name, NULL);
  if(file_handle == NULL)
  {
    DEBUGMSG(debug_loaders, "get_svg_dimensions(): file %s not found\n", file_name);
    rsvg_term();
    return;
  }

  rsvg_handle_get_dimensions(file_handle, &dimensions);

  *width = dimensions.width;
  *height = dimensions.height;

  g_object_unref(file_handle);
  rsvg_term();
}

#endif /* HAVE_RSVG */

/* Load an image without resizing it */
SDL_Surface* LoadImage(const char* file_name, int mode)
{
  return LoadScaledImage(file_name, mode, -1, -1);
}

/* LoadScaledImage : Load an image and resize it to given dimensions.
   If width or height is negative no resizing is applied.
   The loader (load_svg() or IMG_Load()) is chosen depending on file extension,
   If an SVG file is not found try to load its PNG equivalent
   (unless IMG_NO_PNG_FALLBACK is set) */
SDL_Surface* LoadScaledImage(const char* file_name, int mode, int width, int height)
{
  return load_image(file_name, mode, width, height, false);
}

/* LoadImageOfBoundingBox : Same as LoadScaledImage but preserve image proportions
   and fit it into max_width x max_height rectangle.
   Returned surface is not necessarily max_width x max_height ! */
SDL_Surface* LoadImageOfBoundingBox(const char* file_name, int mode, int max_width, int max_height)
{
  return load_image(file_name, mode, max_width, max_height, true);
}


/* load_image : helper function used by LoadScaledImage and LoadImageOfBoundingBox */
SDL_Surface* load_image(const char* file_name, int mode, int w, int h, bool proportional)
{
  SDL_Surface* loaded_pic = NULL;
  SDL_Surface* final_pic = NULL;
  char fn[PATH_MAX];
  int fn_len;
  int width = -1, height = -1;
  bool is_svg = true;

  if(NULL == file_name)
  {
    DEBUGMSG(debug_loaders, "load_image(): file_name is NULL, exiting.\n");
    return NULL;
  }

  /* run loader depending on file extension */

  /* add path prefix */
  snprintf(fn, PATH_MAX, "%s/images/%s", DATA_PREFIX, file_name);
  fn_len = strlen(fn);

  if(strcmp(fn + fn_len - 4, ".svg"))
  {
    DEBUGMSG(debug_loaders, "load_image(): %s is not an SVG, loading using IMG_Load()\n", fn);
    loaded_pic = IMG_Load(fn);
    is_svg = false;
    if (NULL == loaded_pic)
    {
      is_svg = true;
      DEBUGMSG(debug_loaders, "load_image(): Trying to load SVG equivalent of %s\n", fn);
      sprintf(strrchr(fn, '.'), ".svg");
    }
  }
  if (is_svg)
  {
#ifdef HAVE_RSVG
    DEBUGMSG(debug_loaders, "load_image(): trying to load %s as SVG.\n", fn);
    if(proportional)
    {
      get_svg_dimensions(fn, &width, &height);
      if(width > 0 && height > 0)
        fit_in_rectangle(&width, &height, w, h);
    }
    else
    {
      width = w;
      height = h;
    }
    loaded_pic = load_svg(fn, width, height, NULL);
#endif

    if(loaded_pic == NULL)
    {
#ifdef HAVE_RSVG
      DEBUGMSG(debug_loaders, "load_image(): failed to load %s as SVG.\n", fn);
#else
      DEBUGMSG(debug_loaders, "load_image(): SVG support not available.\n");
#endif
      if(mode & IMG_NO_PNG_FALLBACK)
      {
        DEBUGMSG(debug_loaders, "load_image(): %s : IMG_NO_PNG_FALLBACK is set.\n", fn);
      }
      else
      {
        DEBUGMSG(debug_loaders, "load_image(): Trying to load PNG equivalent of %s\n", fn);
        strcpy(fn + fn_len - 3, "png");

        loaded_pic = IMG_Load(fn);
        is_svg = false;
      }
    }
  }

  if (NULL == loaded_pic) /* Could not load image: */
  {
    if (mode & IMG_NOT_REQUIRED)
    {
      DEBUGMSG(debug_loaders, "load_image(): Warning: could not load optional graphics file %s\n", file_name);
      return NULL;  /* Allow program to continue */
    }
    /* If image was required, exit from program: */
    fprintf(stderr, "load_image(): ERROR could not load required graphics file %s\n", file_name);
    fprintf(stderr, "%s", SDL_GetError() );
    cleanup_on_error();
  }
  else if(!is_svg && w > 0 && h > 0)
  {
    if(proportional)
    {
      width = loaded_pic->w;
      height = loaded_pic->h;
      fit_in_rectangle(&width, &height, w, h);
    }
    else
    {
      width = w;
      height = h;
    }
    final_pic = zoom(loaded_pic, width, height);
    SDL_FreeSurface(loaded_pic);
    loaded_pic = final_pic;
    final_pic = NULL;
  }

  final_pic = set_format(loaded_pic, mode);
  SDL_FreeSurface(loaded_pic);
  DEBUGMSG(debug_loaders, "Leaving load_image()\n\n");

  return final_pic;
}

/* adjust width and height to fit in max_width x max_height rectangle
   but preserve their proportion */
void fit_in_rectangle(int* width, int* height, int max_width, int max_height)
{
  float scale_w, scale_h;

  if(width != 0 && height != 0)
  {
    scale_w = (float) max_width / (*width);
    scale_h = (float) max_height / (*height);
    *width *= min(scale_w, scale_h);
    *height *= min(scale_w, scale_h);
  }
}

SDL_Surface* set_format(SDL_Surface* img, int mode)
{
  switch (mode & IMG_MODES)
  {
    case IMG_REGULAR:
    {
      DEBUGMSG(debug_loaders, "set_format(): handling IMG_REGULAR mode.\n");
      return SDL_DisplayFormat(img);
    }

    case IMG_ALPHA:
    {
      DEBUGMSG(debug_loaders, "set_format(): handling IMG_ALPHA mode.\n");
      return SDL_DisplayFormatAlpha(img);
    }

    case IMG_COLORKEY:
    {
      DEBUGMSG(debug_loaders, "set_format(): handling IMG_COLORKEY mode.\n");
      SDL_LockSurface(img);
      SDL_SetColorKey(img, (SDL_SRCCOLORKEY | SDL_RLEACCEL),
                      SDL_MapRGB(img->format, 255, 255, 0));
      return SDL_DisplayFormat(img);
    }

    default:
    {
      DEBUGMSG(debug_loaders, "set_format(): Image mode not recognized\n");
    }
  }

  return NULL;
}


/* LoadBkgd() : a wrapper for LoadImage() that optimizes
   the format of background image */
SDL_Surface* LoadBkgd(const char* file_name, int width, int height)
{
  SDL_Surface* orig = NULL;
  SDL_Surface* final_pic = NULL;

  orig = LoadScaledImage(file_name, IMG_REGULAR, width, height);

  if (!orig)
  {
    DEBUGMSG(debug_loaders, "In LoadBkgd(), LoadImage() returned NULL on %s\n",
             file_name);
    return NULL;
  }

  /* turn off transparency, since it's the background */
  SDL_SetAlpha(orig, SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
  final_pic = SDL_DisplayFormat(orig); /* optimize the format */
  SDL_FreeSurface(orig);

  return final_pic;
}

/* LoadBothBkgds() : loads two scaled images: one for the fullscreen mode
   (fs_res_x,fs_rex_y) and one for the windowed mode (win_res_x,win_rex_y)
   Now we also optimize the format for best performance */
void LoadBothBkgds(const char* file_name, SDL_Surface** fs_bkgd, SDL_Surface** win_bkgd)
{
  DEBUGMSG(debug_loaders, "Entering LoadBothBkgds()\n");
  *fs_bkgd = LoadBkgd(file_name, fs_res_x, fs_res_y);
  *win_bkgd = LoadBkgd(file_name, win_res_x, win_res_y);
}


sprite* LoadSprite(const char* name, int mode)
{
  return LoadScaledSprite(name, mode, -1, -1);
}

sprite* LoadScaledSprite(const char* name, int mode, int width, int height)
{
  return load_sprite(name, mode, width, height, false);
}

sprite* LoadSpriteOfBoundingBox(const char* name, int mode, int max_width, int max_height)
{
  return load_sprite(name, mode, max_width, max_height, true);
}

sprite* load_sprite(const char* name, int mode, int w, int h, bool proportional)
{
  sprite *new_sprite = NULL;
  char fn[PATH_MAX];
  int i, width, height;
  char pngfn[PATH_MAX];
  char cache_path[PATH_MAX];

  /* get caches directory */
  get_user_data_dir_with_subdir(cache_path);
  strcat(cache_path, "caches");

#ifdef HAVE_RSVG
  /* check if SVG sprite file is present */
  sprintf(fn, "%s/images/%s.svg", DATA_PREFIX, name);
  if(1 == check_file(fn))
  {
    if(proportional)
    {
      get_svg_dimensions(fn, &width, &height);
      if(width > 0 && height > 0)
        fit_in_rectangle(&width, &height, w, h);
    }
    else
    {
      width = w;
      height = h;
    }

    /* check if cached PNG version exists */
    sprintf(pngfn, "%s/%sd-%d-%d.png", cache_path, name, width, height);
    if(check_file(pngfn)==1)
    {
      new_sprite=(sprite*)malloc(sizeof(sprite));
      new_sprite->default_img=IMG_Load(pngfn);
      i=0;
      while(1)
      {
        sprintf(pngfn, "%s/%s%d-%d-%d.png", cache_path, name, i, width, height);
        if(check_file(pngfn)==1)
        {
          new_sprite->frame[i]=IMG_Load(pngfn);
          i++;
        }
        else break;
      }
      new_sprite->num_frames=i;
    }
    else
    {
      new_sprite = load_svg_sprite(fn, width, height);
    }

    if(new_sprite)
    {
      set_format(new_sprite->default_img, mode);
      for(i = 0; i < new_sprite->num_frames; i++)
        set_format(new_sprite->frame[i], mode);
      new_sprite->cur = 0;   

      /* cache loaded sprites in PNG files */
      sprintf(pngfn, "%s/%sd-%d-%d.png", cache_path, name, width, height);
      if(check_file(pngfn)!=1) 
        savePNG(new_sprite->default_img,pngfn);
      for(i=0; i<new_sprite->num_frames; i++)
      {
        sprintf(pngfn, "%s/%s%d-%d-%d.png", cache_path, name, i, width, height);
        if(check_file(pngfn)!=1) 
          savePNG(new_sprite->frame[i],pngfn);
      }

     
    }
  }
#endif

  if(!new_sprite)
  {
    /* SVG sprite was not loaded, try to load it frame by frame from PNG files */
    new_sprite = malloc(sizeof(sprite));

    sprintf(fn, "%sd.png", name);  // The 'd' means the default image
    if(proportional)
      new_sprite->default_img = LoadImageOfBoundingBox(fn, mode | IMG_NOT_REQUIRED, w, h);
    else
      new_sprite->default_img = LoadScaledImage(fn, mode | IMG_NOT_REQUIRED, w, h);

    if(!new_sprite->default_img)
      DEBUGMSG(debug_loaders, "load_sprite(): failed to load default image for %s\n", name);

    for(i = 0; i < MAX_SPRITE_FRAMES; i++)
    {
      sprintf(fn, "%s%d.png", name, i);
      if(proportional)
        new_sprite->frame[i] = LoadImageOfBoundingBox(fn, mode | IMG_NOT_REQUIRED, w, h);
      else
        new_sprite->frame[i] = LoadScaledImage(fn, mode | IMG_NOT_REQUIRED, w, h);

      if(new_sprite->frame[i] == NULL)
      {
        new_sprite->cur = 0;
        new_sprite->num_frames = i;
        break;
      }
      else
        DEBUGMSG(debug_loaders, "load_sprite(): loaded frame %d of %s\n", i, name);
    }
  }

  return new_sprite;
}

sprite* FlipSprite(sprite* in, int X, int Y)
{
  sprite *out;

  out = malloc(sizeof(sprite));
  if (in->default_img != NULL)
    out->default_img = Flip( in->default_img, X, Y );
  else
    out->default_img = NULL;
  for( out->num_frames=0; out->num_frames<in->num_frames; out->num_frames++ )
    out->frame[out->num_frames] = Flip( in->frame[out->num_frames], X, Y );
  out->cur = 0;
  return out;
}

void FreeSprite(sprite* gfx)
{
  int x;
  if (!gfx)
    return;

  DEBUGMSG(debug_loaders, "Freeing image at %p", gfx);
  for (x = 0; x < gfx->num_frames; x++)
  {
    DEBUGMSG(debug_loaders, ".");
    if (gfx->frame[x])
    {
      SDL_FreeSurface(gfx->frame[x]);
      gfx->frame[x] = NULL;
    }
  }

  if (gfx->default_img)
  {
    SDL_FreeSurface(gfx->default_img);
    gfx->default_img = NULL;
  }

  DEBUGMSG(debug_loaders, "FreeSprite() - done\n");
  free(gfx);
}

void NextFrame(sprite* s)
{
  if (s && s->num_frames)
    s->cur = (s->cur + 1) % s->num_frames;
}



/* LoadSound : Load a sound/music patch from a file. */
Mix_Chunk* LoadSound( char *datafile )
{
  Mix_Chunk* tempChunk = NULL;
  char fn[PATH_MAX];

//    sprintf(fn , "%s/sounds/%s", realPath[i], datafile);
  sprintf(fn , "%s/sounds/%s", DATA_PREFIX, datafile);
  tempChunk = Mix_LoadWAV(fn);
  if (!tempChunk)
  {
    fprintf(stderr, "LoadSound(): %s not found\n\n", fn);
  }
  return tempChunk;
}

/* LoadMusic : Load music from a datafile */
Mix_Music* LoadMusic(char *datafile )
{
  char fn[PATH_MAX];
  Mix_Music* tempMusic = NULL;

  sprintf( fn , "%s/sounds/%s", DATA_PREFIX, datafile );
  if (1 != check_file(fn))
  {
    fprintf(stderr, "LoadMusic(): %s not found\n\n", fn);
    return NULL;
  }

  tempMusic = Mix_LoadMUS(fn);

  if (!tempMusic)
  {
    fprintf(stderr, "LoadMusic(): %s not loaded successfully\n", fn);
    printf("Error was: %s\n\n", Mix_GetError());
  }
  return tempMusic;
}

static void savePNG(SDL_Surface* surf,char* fn)
{
  FILE* fi;
  DIR* dir_ptr;
  int i;
  char tempc;

  /* create all preceding directories for fn if necessary */
  i=0;
  while(fn[i])
  {
    if(fn[i]=='/')
    {
      tempc=fn[i+1];
      fn[i+1]=0;

      /* test if the directory already exists */
      dir_ptr = opendir(fn);
      if (dir_ptr)
      {
        closedir(dir_ptr);
      }
      else /* create new directory */
      {
        int status;

#ifndef BUILD_MINGW32
        status = mkdir(fn, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
        status = mkdir(fn);
#endif

        /* mkdir () returns 0 if successful */
        if (0 == status)
        {
          /* successful */
          fprintf(stderr, "\nmkdir %s succeeded\n",fn);
        }
        else
        {
          fprintf(stderr, "\nmkdir %s failed\n",fn);
          fn[i+1]=tempc;
          return;
        }
        
      } 
      fn[i+1]=tempc;

    } /* end of fn[i]=='/' */

    i++;
      
  } /* end of while */

  fi = fopen(fn, "wb");
  if(fi==NULL)
    fprintf(stderr, "\nError: Couldn't write to file %s!\n\n", fn);
  else
    do_png_save(fi,fn,surf);
}

/* The following functions are taken from Tuxpaint with minor changes */

/* Actually save the PNG data to the file stream: */
static int do_png_save(FILE * fi, const char *const fname, SDL_Surface * surf)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_text text_ptr[4];
  unsigned char **png_rows;
  Uint8 r, g, b, a;
  int x, y, count;
  Uint32(*getpixel) (SDL_Surface *, int, int) =
    getpixels[surf->format->BytesPerPixel];


  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
  {
    fclose(fi);
    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

    fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
    //draw_tux_text(TUX_OOPS, strerror(errno), 0);
  }
  else
  {
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
      fclose(fi);
      png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

      fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
      //draw_tux_text(TUX_OOPS, strerror(errno), 0);
    }
    else
    {
      if (setjmp(png_jmpbuf(png_ptr)))
      {
	fclose(fi);
	png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	fprintf(stderr, "\nError: Couldn't save the image!\n%s\n\n", fname);
	//draw_tux_text(TUX_OOPS, strerror(errno), 0);

	return 0;
      }
      else
      {
	png_init_io(png_ptr, fi);

	info_ptr->width = surf->w;
	info_ptr->height = surf->h;
	info_ptr->bit_depth = 8;
	info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	info_ptr->interlace_type = 1;
	info_ptr->valid = 0;	/* will be updated by various png_set_FOO() functions */

	png_set_sRGB_gAMA_and_cHRM(png_ptr, info_ptr,
				   PNG_sRGB_INTENT_PERCEPTUAL);

	/* Set headers */

	count = 0;

	/*
	   if (title != NULL && strlen(title) > 0)
	   {
	   text_ptr[count].key = "Title";
	   text_ptr[count].text = title;
	   text_ptr[count].compression = PNG_TEXT_COMPRESSION_NONE;
	   count++;
	   }
	 */

	text_ptr[count].key = (png_charp) "Software";
	text_ptr[count].text =
	  (png_charp) "Tux Paint " /*VER_VERSION " (" VER_DATE ")"*/;
	text_ptr[count].compression = PNG_TEXT_COMPRESSION_NONE;
	count++;


	png_set_text(png_ptr, info_ptr, text_ptr, count);

	png_write_info(png_ptr, info_ptr);



	/* Save the picture: */

	png_rows = malloc(sizeof(char *) * surf->h);

	for (y = 0; y < surf->h; y++)
	{
	  png_rows[y] = malloc(sizeof(char) * 4 * surf->w);

	  for (x = 0; x < surf->w; x++)
	  {
	    SDL_GetRGBA(getpixel(surf, x, y), surf->format, &r, &g, &b, &a);

	    png_rows[y][x * 4 + 0] = r;
	    png_rows[y][x * 4 + 1] = g;
	    png_rows[y][x * 4 + 2] = b;
            png_rows[y][x * 4 + 3] = a;
	  }
	}

	png_write_image(png_ptr, png_rows);

	for (y = 0; y < surf->h; y++)
	  free(png_rows[y]);

	free(png_rows);


	png_write_end(png_ptr, NULL);

	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fi);

	return 1;
      }
    }
  }

  return 0;
}


