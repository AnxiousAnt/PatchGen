#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 101)
#error You need at least flext version 0.1.1 
#endif


class mydsp:
	public flext_dsp
{
	// f_header(class, parent class)
	FLEXT_HEADER(mydsp,flext_dsp)

	public:
		mydsp(float notused=0);
		~mydsp();
	
	protected:
		virtual void m_signal(int n, float *const *in, float *const *out);
};


// this complains about static variable flext_obj::m_name
// FLEXT_TILDE_NEW("mydsp", mydsp)

// make implementation of a tilde object with one float arg
FLEXT_TILDE_1ARG("mydsp", mydsp, float)


void mydsp::cb_setup(t_class *c) 
{
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
