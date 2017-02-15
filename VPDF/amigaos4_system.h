ULONG VARARGS68K ProcessPixelArray(struct RastPort *, LONG, LONG, LONG, LONG, struct DrawInfo *dri, ...);

enum
{
   PPA_Reset = 0x84140000,  /* Apply effects, start new sequence */
   PPA_Gradient,            /* Gradient type; see definitions below */
   PPA_GradientStart,       /* 00RRGGBB value of gradient start */
   PPA_GradientEnd,         /* 00RRGGBB value of gradient end */
   PPA_Brighten,            /* Add this delta to all pixels */
   PPA_Darken,              /* Subtract this delta from all pixels */
   PPA_MakeGrey,            /* Convert rectangle to greyscale? */
   PPA_Blur,                /* Apply a light blur to rectangle? */
   PPA_Impose               /* AARRGGBB value to impose over pixels */
};

enum
{
   GRAD_TOPTOBOTTOM =   0,  /* Top-to-bottom gradient */
   GRAD_LEFTTORIGHT =  90,  /* Left-to-right gradient */
   GRAD_BOTTOMTOTOP = 180,  /* Bottom-to-top gradient */
   GRAD_RIGHTTOLEFT = 270,  /* Right-to-left gradient */
};
