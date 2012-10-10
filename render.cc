#include "palette.h"
#include "vec3.h"

namespace {
const int bayer_matrix[] = {
   1, 9, 3,11,
  13, 5,15, 7,
   4,12, 2,10,
  16, 8,14, 6};

// get gamma-corrected intensity of a pixel value
float gamma(float x) {
  return pow(x*(1.0/255.0), 2.2);
}

void nearestcolor(const vec3& c, int *fg, int *bg) {
  // brute force
  float minerr = 1e30;
  *fg = -1; *bg = -1;
  for(int i = 0; i < 17; i++) {
    for(int j = 0; j < 9; j++) {
      float er = 0.7*gamma(palette_[j*3  ]) + 0.3*gamma(palette_[i*3  ]) - c.x;
      float eg = 0.7*gamma(palette_[j*3+1]) + 0.3*gamma(palette_[i*3+1]) - c.y;
      float eb = 0.7*gamma(palette_[j*3+2]) + 0.3*gamma(palette_[i*3+2]) - c.z;
      float vr = gamma(palette_[j*3  ]) - gamma(palette_[i*3  ]);
      float vg = gamma(palette_[j*3+1]) - gamma(palette_[i*3+1]);
      float vb = gamma(palette_[j*3+2]) - gamma(palette_[i*3+2]);
#if 0
      float ey =  0.257*er + 0.504*eg + 0.098*eb;
      float ev =  0.439*er - 0.368*eg - 0.071*eb;
      float eu = -0.148*er - 0.291*eg + 0.439*eb;
      float err = ey*ey + ev*ev + eu*eu;
#if 0
      err += 0.001*(vr*vr + vg*vg + vb*vg);
#endif
#if 0
      float vy =  0.257*vr + 0.504*vg + 0.098*vb;
      float vv =  0.439*vr - 0.368*vg - 0.071*vb;
      float vu = -0.148*vr - 0.291*vg + 0.439*vb;
      err += 0.01*(vu*vu + vb*vb);
#endif
#else
      float err = er*er + eg*eg + eb*eb;
      err += 0.001*(vr*vr + vg*vg + vb*vg);
#endif
      if (err < minerr) { minerr = err; *fg = i-1; *bg = j-1; }
    }
  }
  //printf("minerr=%g best=%d,%d c=%g,%g,%g\n", minerr, *fg, *bg, c.x, c.y, c.z);
}

int color_LUT[16*16*16];

}

void render_init() {
  const float scale = 1.0/15.0;
  for (int r = 0; r < 16; r++) {
    for (int g = 0; g < 16; g++) {
      for (int b = 0; b < 16; b++) {
        int fg, bg;
        nearestcolor(vec3(r*scale, g*scale, b*scale), &fg, &bg);
        color_LUT[r+g*16+b*256] = (bg+1)*32+(fg+1);
        // printf("%x%x%x=f=%d b=%d\n", r,g,b, fg, bg);
      }
    }
  }
}

// nearest color fg/bg, dithering offset x,y
void render_color(const vec3& c, int x, int y) {
  float o = bayer_matrix[(x&3) + (y&3)*4]*(1.0/8.0);
  int r = std::max(0, std::min(15, (int) (c.x*15.0 - o)));
  int g = std::max(0, std::min(15, (int) (c.y*15.0 - o)));
  int b = std::max(0, std::min(15, (int) (c.z*15.0 - o)));
  unsigned int entry = color_LUT[r+g*16+b*256];
  int fg = (entry&31)-1;
#if 1
  int bg = (entry>>5)-1;
  if(bg >= 0) printf("\x1b[%d;%d;%dm#", !!(fg&8), 40+(bg&7), 30+(fg&7));
  else if(fg >= 0) printf("\x1b[0;%d;%dm#", !!(fg&8), 30+(fg&7));
  else printf("\x1b[0m ");
#else
  if(fg >= 0) printf("\x1b[%d;%dm#", !!(fg&8), 30+(fg&7));
  else printf(" ");
#endif
}


