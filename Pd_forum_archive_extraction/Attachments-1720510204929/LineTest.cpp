///// rough clone of pd's line object:


using e_float = float;

class LineRamp
{
public:
	LineRamp();
	LineRamp(e_float target, e_float time);
	LineRamp(e_float initial, e_float target, e_float time);
	~LineRamp();
	
	void setLine(e_float target, e_float time);
	void setLine(e_float initial, e_float target, e_float timeMs);
	
	void trigger();
	
	void stop();
	
	void updateSampleRate(int newSampleRate);

	e_float performDSP();	
	
private:
	
	void commonConstructorStuff();
	
	void setTotalValueChange();
	void setMsPerSample();
	void setTotalTicks();
	void setIncrement();
    
    void resetValues();
		
	unsigned long int sampleRate; // correct?
	
	bool initialise;
		
	e_float startValue;
	e_float targetValue;
	e_float currentValue;
	e_float totalValueChange;
	
	e_float increment;
	e_float msPerSample;
	e_float lineTimeInMs;
	
	unsigned long int totalTicks; // use longest int possible
	unsigned long int remainingTicks;
};


LineRamp::LineRamp()
{
	startValue = 0;
	targetValue = 0;
	lineTimeInMs = 0;
    initialise = false;
    
	commonConstructorStuff();	
}

LineRamp::LineRamp(e_float lineTarget, e_float lineTime)
{
	startValue = 0;
	targetValue = lineTarget;
	lineTimeInMs = lineTime;
	initialise = false;
	
	commonConstructorStuff();	
}

LineRamp::LineRamp(e_float lineInitial, e_float lineTarget, e_float lineTime)
{
	startValue = lineInitial;
	targetValue = lineTarget;
	lineTimeInMs = lineTime;
	initialise = true;

	commonConstructorStuff();
}

LineRamp::~LineRamp()
{
    //does nothing, OK?
}

void LineRamp::commonConstructorStuff()
{
	sampleRate = 44100;  // remove this magic number later!
    setMsPerSample();
    
	currentValue = startValue;
    
    resetValues();
}

void LineRamp::resetValues()
{
    setTotalValueChange();
    setTotalTicks();
    setIncrement();
}


void LineRamp::setLine(e_float target, e_float timeMs)
{
	targetValue = target;	
	lineTimeInMs = timeMs;
	initialise = false;
    
    resetValues();
}

void LineRamp::setLine (e_float start, e_float target, e_float timeMs)
{
	startValue = start;
	targetValue = target;		
	lineTimeInMs = timeMs;
	initialise = true;
    
    resetValues();
}

void LineRamp::setTotalValueChange()
{
	totalValueChange = targetValue - currentValue;
}

void LineRamp::setMsPerSample()
{
	msPerSample = sampleRate * 0.001;
}

void LineRamp::setTotalTicks()
{
	totalTicks = msPerSample * lineTimeInMs;
}

void LineRamp::setIncrement()
{
	if (lineTimeInMs == 0)
	{
        increment = totalValueChange;
	}
	else
    {
        increment = totalValueChange / totalTicks;
    }
}

void LineRamp::updateSampleRate(int newSampleRate)
{
	sampleRate = newSampleRate;
	setMsPerSample();
	setTotalTicks(); 
	setIncrement(); 
}

void LineRamp::trigger()
{
	remainingTicks = totalTicks;
	
	if (initialise)
	{
		currentValue = startValue;
        resetValues();        
	}

}

void LineRamp::stop()
{
	remainingTicks = 0;
}


e_float LineRamp::performDSP()
{
	e_float output = currentValue;
	
	if (remainingTicks)
	{
		currentValue += increment;
		--remainingTicks;
	}
	return output;
}

/*

notes:  

!! currently, there is the possibility of a race condition if trigger() is called during the performDSP loop?

!! still not checking if line actually makes it 100% of the way to its target.  This may be ok for all intents and purposes?

*/