/*-----------------------------------------------------------------

    GEM - Graphics Environment for Multimedia

    Load an video into a pix block

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_VIDEO_H_
#define INCLUDE_PIX_VIDEO_H_

#include "Base/GemBase.h"

#ifdef __sgi
#include <vl/vl.h>
#elif _WINDOWS
#include <vfw.h>
#elif defined (__linux__)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#define DEVICENO 0
#define NBUF 2
#define COMPOSITEIN 1
#else
#error No video defined for OS
#endif

#include "Base/GemPixUtil.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	pix_video
    
    Loads in an video
    
KEYWORDS
    pix
    
DESCRIPTION

    "dimen" (int, int) - set the x,y dimensions
    "zoom" (int, int) - the zoom factor (1.0 is nominal) (num / denom)
    "bright" (int) - the brightnes
    "contrast" (int) - the contrast
    "hue" (int) - the hue
    "sat" (int) - the saturation
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_video : public GemBase
{
    CPPEXTERN_HEADER(pix_video, GemBase)

    public:

        //////////
        // Constructor
#ifdef _WINDOWS
    	pix_video(t_floatarg num);
#else
    	pix_video();
#endif
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_video();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Clear the dirty flag on the pixBlock
    	virtual void 	postrender(GemState *state);

    	//////////
    	virtual void	startRendering();

    	//////////
    	// If you care about the stop of rendering
    	virtual void	stopRendering();
    	
    	//////////
    	// Clean up the pixBlock
    	void	    	cleanPixBlock();
    
    	//////////
    	// Set the video dimensions
#if __linux__
    	    /* in Linux the camera's own dimensions are specifiable; the
	    image is resized appropriately.  So we have 6 independent components
	    to specify.  X and Y give the OUTPUT dimensions as a pix object;
	    the margins are added to that to make the rectangle filled by the
	    video input. */
    	void	    	dimenMess(int x, int y,
	    	    	    int leftmargin, int rightmargin,
    	    	    	    int topmargin, int bottommargin);
	void normMess(t_symbol* s);
#else
    	void	    	dimenMess(int x, int y);
#endif
    	//////////
    	// Set the video offset
    	void	    	offsetMess(int x, int y);
    
    	//////////
    	// Start up the video device
    	// [out] int - returns 0 if bad
    	int	    		startTransfer();
    
    	//////////
    	// Stop the video device
    	// [out] int - returns 0 if bad
    	int	    		stopTransfer();
    
    	////////// 
    	// Stop the video device
    	// [out] int - returns 0 if bad
    	void	    	swapMess(int state);
    
	    //-----------------------------------
	    // GROUP:	Video data
	    //-----------------------------------
    
     	//////////
    	// If video is connected
    	int 	    	m_haveVideo;
    	
    	//////////
    	// The pixBlock with the current image
    	pixBlock    	m_pixBlock;
    	
    	//////////
    	// Should swap the pixels?
    	int 	    	m_swap;
    	 
    	//////////
    	// Do we have to color swap?
    	int 	    	m_colorSwap;
    	
#ifdef __sgi
		VLServer    	m_svr;
		VLPath      	m_path;
		VLNode      	m_src;
		VLNode	    	m_drn;
		VLBuffer    	m_buffer;
#elif _WINDOWS
		HWND			m_hWndC;
		void			videoFrame(LPVIDEOHDR lpVHdr);
		int				m_vidXSize;
		int				m_vidYSize;
		int				m_newFrame;
#elif defined(__linux__)
		struct video_tuner vtuner;
		struct video_picture vpicture;
		struct video_buffer vbuffer;
		struct video_capability vcap;
		struct video_channel vchannel;
		struct video_audio vaudio;
		struct video_mbuf vmbuf;
		struct video_mmap vmmap[NBUF];
		int tvfd;
		int frame;
		unsigned char *videobuf;
		int skipnext;
		int mytopmargin, mybottommargin;
		int myleftmargin, myrightmargin;
#else
#error Define pix_video for this OS
#endif

    private:
    	
    	//////////
    	// static member functions
#if __linux__
    	static void dimenMessCallback(void *data, 
	    t_symbol *s, int ac, t_atom *av);
	static void normMessCallback(void *data, t_symbol* s);
#else
    	static void dimenMessCallback(void *data, t_floatarg x, t_floatarg y);
#endif
    	static void offsetMessCallback(void *data, t_floatarg x, t_floatarg y);
    	static void swapMessCallback(void *data, t_floatarg state);

#ifdef _WINDOWS
		static void videoFrameCallback(HWND hWnd, LPVIDEOHDR lpVHdr);
#endif

};

#endif	// for header file
