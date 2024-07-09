#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 101)
#error You need at least flext version 0.1.1 
#endif

class mydsp:
	public flext_dsp
{
	// flext_header(class, parent class)
	FLEXT_HEADER(mydsp,flext_dsp)

	public:
		mydsp() 
		{
			add_in_signal();   // audio in
			add_out_signal();  // audio out
			setup_inout();     // set up inlets and outlets
		}
		//~mydsp(){}
	
	protected:
		virtual void m_signal(int n, float *const *in, float *const *out);
};


FLEXT_TILDE_NEW("mydsp~", mydsp)


void mydsp::cb_setup(t_class *c) 
{
	// define
	// no additional methods to set up
}


void mydsp::m_signal(int n, float *const *in, float *const *out)
{
	const float *ins = in[0];
	float *outs = out[0];

	while (n--)
	{
		float f = (*ins++);
		*outs++ = (f > 0 ? f : -f);
	}
}
