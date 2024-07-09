////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// mark@danks.org
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
/*
    this is an attempt at a Linux version of pix_video by Miller Puckette.
    Anyone conversant in c++ will probably howl at this.  I'm uncertain of
    several things.
    
    First, the #includes I threw in pix_video.h may not all be necessary; I
    notice that far fewer are needed for the other OSes.
    
    Second, shouldn't the os-dependent state variables be "private"?  I
    followed the lead of the other os-dependent state variables.  Also,
    I think the indentation is goofy but perhaps there's some reason for it.

    Third, I probably shouldn't be using sprintf to generate filenames; I
    don't know the "modern" c++ way to do this.
    
    Fourth, I don't know why some state variables 
    show up as "arguments" in the pix_video :: pix_video().
     
    This code is written with the "bttv" device in mind, which memory mapes
    images up to 24 bits per pixel.  So we request the whole 24 and don't
    settle for anything of lower quality (nor do we offer anything of higher
    quality; it seems that Gem is limited to 32 bits per pixel including
    alpha.)  We take all video images to be opaque by setting the alpha
    channel to 255.

*/
    
#include "pix_video.h"
#include "Base/GemCache.h"

CPPEXTERN_NEW(pix_video)

#define BYTESIN 3

/////////////////////////////////////////////////////////
//
// video
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_video :: pix_video()
    	   : m_haveVideo(0), m_swap(1), m_colorSwap(0)
{
    char buf[256];
    int i, dataSize;
    frame = 0;
    int width, height;
    
    skipnext = 0;
    m_pixBlock.image.data = NULL;
    sprintf(buf, "/dev/video%d", DEVICENO);
    if ((tvfd = open(buf, O_RDWR)) < 0)
    {
	perror(buf);
	goto closit;
    }
    if (ioctl(tvfd, VIDIOCGCAP, &vcap) < 0)
    {
	perror("get capabilities");
	goto closit;
    }
    post("cap: name %s type %d channels %d maxw %d maxh %d minw %d minh %d\n",
    	vcap.name, vcap.type,  vcap.channels,  vcap.maxwidth,  vcap.maxheight,
	    vcap.minwidth,  vcap.minheight);
    if (ioctl(tvfd, VIDIOCGPICT, &vpicture) < 0)
    {
	perror("VIDIOCGCAP");
	goto closit;
    }
    
    post("picture: brightness %d depth %d palette %d\n",
	    vpicture.brightness, vpicture.depth, vpicture.palette);

    for (i = 0; i < vcap.channels; i++)
    {
	vchannel.channel = i;
	if (ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0)
	{
	    perror("VDIOCGCHAN");
	    goto closit;
	}
    	printf("channel %d name %s type %d flags %d\n",
    	    vchannel.channel, vchannel.name, 
	    vchannel.type, vchannel.flags);
    }
    vchannel.channel = COMPOSITEIN;
    if (ioctl(tvfd, VIDIOCGCHAN, &vchannel) < 0)
    {
	perror("VDIOCGCHAN");
	goto closit;
    }
    vchannel.norm = VIDEO_MODE_NTSC;
    if (ioctl(tvfd, VIDIOCSCHAN, &vchannel) < 0)
    {
	perror("VDIOCSCHAN");
	goto closit;
    }
    	/* get mmap numbers */
    if (ioctl(tvfd, VIDIOCGMBUF, &vmbuf) < 0)
    {
	perror("VIDIOCGMBUF");
	goto closit;
    }
    post("buffer size %d, frames %d, offset %d %d\n", vmbuf.size,
    	vmbuf.frames, vmbuf.offsets[0], vmbuf.offsets[1]);
    if (!(videobuf = (unsigned char *)
    	mmap(0, vmbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, tvfd, 0)))
    {
	perror("mmap");
	goto closit;
    }
#if 0
    width = 768+76;
    height = vcap.maxheight;
#endif    
    width = 320;    	/* N.B. this is overwritten later */
    height = 240;
   
    for (i = 0; i < NBUF; i++)
    {
    	vmmap[i].format = VIDEO_PALETTE_RGB24;
    	vmmap[i].width = width;
    	vmmap[i].height = height;
	vmmap[i].frame  = i;
    }
    if (ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0)
    {
    	if (errno == EAGAIN)
	    fprintf(stderr, "can't sync (no video source?)\n");
    	else 
	    perror("VIDIOCMCAPTURE");
	goto closit;
    }
    post("frame %d %d, format %d, width %d, height %d\n",
    	frame, vmmap[frame].frame, vmmap[frame].format,
    	vmmap[frame].width, vmmap[frame].height);

    	/* fill in image specifics for Gem pixel object.  Could we have
	just used RGB, I wonder? */
    m_pixBlock.image.xsize = width;
    m_pixBlock.image.ysize = height;
    m_pixBlock.image.csize = 4;
    m_pixBlock.image.format = GL_RGBA;
    m_pixBlock.image.type = GL_UNSIGNED_BYTE;
    myleftmargin = 0;
    myrightmargin = 0;
    mytopmargin = 0;
    mybottommargin = 0;
    
    dataSize = m_pixBlock.image.xsize * m_pixBlock.image.ysize
    	    	     * 4 * sizeof(unsigned char);
    m_pixBlock.image.data = new unsigned char[dataSize];

    m_haveVideo = 1;
    post("GEM: pix_video: Opened video connection");
    return;

closit:
    if (tvfd >= 0)
    {
    	close(tvfd);
	tvfd = -1;
    }
    m_haveVideo = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_video :: ~pix_video()
{
    if (m_haveVideo)
    	stopTransfer();

    if (tvfd >= 0)
    	close(tvfd);

    cleanPixBlock();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_video :: render(GemState *state)
{
    int i, row, column;
    unsigned char *pixp;
    
    if (!m_haveVideo)
    {
	post("GEM: pix_video: do not have a video stream");
	return;
    }
    
    if (ioctl(tvfd, VIDIOCSYNC, &vmmap[frame].frame) < 0)
    {
	perror("VIDIOCSYNC");
	m_haveVideo = 0;
	if (tvfd >= 0)
	{
	    close(tvfd);
	    tvfd = 0;
	}
	return;
    }
    unsigned char *newimage = videobuf + vmbuf.offsets[frame];

    int dataSize =
    	m_pixBlock.image.xsize * m_pixBlock.image.ysize *
    	m_pixBlock.image.csize * sizeof(unsigned char);

    if (skipnext)
    {
    	for (i = 0; i < dataSize; i += 4)
	{
    	    m_pixBlock.image.data[i + 0] = 0;
    	    m_pixBlock.image.data[i + 1] = 0;
    	    m_pixBlock.image.data[i + 2] = 0;
    	    m_pixBlock.image.data[i + 3] = 255;
	}
	skipnext = 0;
    }
    	/* copy the image, converting from RGB to RGBA. */
    else for (row = 0; row < m_pixBlock.image.ysize; row++)
    {
    	unsigned char * inrow = (newimage + BYTESIN *
	    ((m_pixBlock.image.xsize + myleftmargin + myrightmargin) *
	    (row + mytopmargin) + myleftmargin));
    	unsigned char *outrow = m_pixBlock.image.data +
	    4 * m_pixBlock.image.xsize * (m_pixBlock.image.ysize - row - 1);
	for (column = 0; column < m_pixBlock.image.xsize; column++)
	{
    	    outrow[0] = *(inrow+2);
    	    outrow[1] = *(inrow+1);
    	    outrow[2] = *(inrow);
    	    outrow[3] = 255; 	/* opaque */
    	    inrow += 3;
	    outrow += 4;
	}
    }
    m_pixBlock.newimage = 1;
    state->image = &m_pixBlock;
    frame = !frame;
    vmmap[frame].width = m_pixBlock.image.xsize + myleftmargin + myrightmargin;
    vmmap[frame].height = m_pixBlock.image.ysize + mytopmargin + mybottommargin;
    if (ioctl(tvfd, VIDIOCMCAPTURE, &vmmap[frame]) < 0)
    {
    	if (errno == EAGAIN)
	    fprintf(stderr, "can't sync (no video source?)\n");
    	else 
	    perror("VIDIOCMCAPTURE");
	m_haveVideo = 0;
	if (tvfd >= 0)
	{
	    close(tvfd);
	    tvfd = 0;
	}
	return;
    }
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_video :: startRendering()
{
    m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_video :: stopRendering()
{
    // this is a no-op
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_video :: postrender(GemState *state)
{
    m_pixBlock.newimage = 0;
    state->image = NULL;
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
int pix_video :: startTransfer()
{
    if (!m_haveVideo)
    	return(0);

    return(1);
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
int pix_video :: stopTransfer()
{
    if ( !m_haveVideo )
    	return(0);
    
    return(1);
}

/////////////////////////////////////////////////////////
// offsetMess
//
/////////////////////////////////////////////////////////
void pix_video :: offsetMess(int x, int y)
{
    post("warning: pix_video_offset does nothing in Linux");
}


void pix_video :: normMess(t_symbol* s)
{
     if (!strcmp(s->s_name, "pal"))	  
	  vchannel.norm = VIDEO_MODE_PAL;
     else if (!strcmp(s->s_name, "ntsc"))	  
	  vchannel.norm = VIDEO_MODE_NTSC;

     if (ioctl(tvfd, VIDIOCSCHAN, &vchannel) < 0)
    {
	perror("VDIOCSCHAN");
    }
}

/////////////////////////////////////////////////////////
// dimenMess
//
/////////////////////////////////////////////////////////
void pix_video :: dimenMess(int x, int y, int leftmargin, int rightmargin,
    int topmargin, int bottommargin)
{
    int xtotal = x + leftmargin + rightmargin;
    int ytotal = y + topmargin + bottommargin;
    if (xtotal > 844)
    	post("x dimensions too great");
    else if (xtotal < vcap.minwidth || x < 1 ||
    	leftmargin < 0 || rightmargin < 0)
    	    post("x dimensions too small");
    if (ytotal > vcap.maxheight)
    	post("y dimensions too great");
    else if (ytotal < vcap.minheight || y < 1 ||
    	topmargin < 0 || bottommargin < 0)
    	    post("y dimensions too small");

    myleftmargin = leftmargin;
    myrightmargin = rightmargin;
    mytopmargin = topmargin;
    mybottommargin = bottommargin;

    m_pixBlock.image.xsize = x;
    m_pixBlock.image.ysize = y;

    cleanPixBlock();
    int dataSize = m_pixBlock.image.xsize * m_pixBlock.image.ysize
    	    	    * 4 * sizeof(unsigned char);
    m_pixBlock.image.data = new unsigned char[dataSize];
    skipnext = 1;
}

/////////////////////////////////////////////////////////
// cleanPixBlock -- free the pixel buffer memory
//
/////////////////////////////////////////////////////////
void pix_video :: cleanPixBlock()
{
    delete [] m_pixBlock.image.data;
    m_pixBlock.image.data = NULL;
}

/////////////////////////////////////////////////////////
// swapMess
//
/////////////////////////////////////////////////////////
void pix_video :: swapMess(int state)
{
    post("warning: pix_video_swap does nothing in Linux");
    if (state) m_swap = 1;
    else m_swap = 0;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_video :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, (t_method)&pix_video::dimenMessCallback,
    	    gensym("dimen"), A_GIMME, A_NULL);
    class_addmethod(classPtr, (t_method)&pix_video::offsetMessCallback,
    	    gensym("offset"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, (t_method)&pix_video::swapMessCallback,
    	    gensym("swap"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, (t_method)&pix_video::normMessCallback,
    	    gensym("norm"), A_SYMBOL, A_NULL);
}
void pix_video :: dimenMessCallback(void *data, t_symbol *s, int ac, t_atom *av)
{
    GetMyClass(data)->dimenMess(atom_getfloatarg(0, ac, av),
    	atom_getfloatarg(1, ac, av),
    	atom_getfloatarg(2, ac, av),
    	atom_getfloatarg(3, ac, av),
    	atom_getfloatarg(4, ac, av),
    	atom_getfloatarg(5, ac, av) );
}
void pix_video :: offsetMessCallback(void *data, t_floatarg x, t_floatarg y)
{
    GetMyClass(data)->offsetMess(x, y);
}

void pix_video :: normMessCallback(void *data, t_symbol* s)
{
    GetMyClass(data)->normMess(s);
}

void pix_video :: swapMessCallback(void *data, t_floatarg state)
{
    GetMyClass(data)->swapMess(state);
}
