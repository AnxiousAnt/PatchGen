
/*
	query a biquads amplitude and phase response
	designed after richard dudas' "big digital filter tutorial"
	vb, mai 2007
*/

/*
	H(z) = (AC + BD) / (C^2 + D^2)  +  j ((BC - AD) / (C^2 + D^2))
	where:
	A = (a0 + a1X1 + a2X2)
	B = (a1Y1 + a2Y2)
	C = (1 + b1X1 + b2X2)
	D = (b1Y1 + b2Y2)
	
	and:
	X1 = cos(-w)
	X2 = cos(-2w)
	Y1 = sin(-w)
	Y2 = sin(-2w)
	
	w: omega
*/

inlets = 1;
outlets = 2;

var pi = Math.PI;
var pi2 = 2*Math.PI;
var sr2 = 22050.;			// nyquist frequency
var a0, a1, a2, b1, b2 = 0;


function list() {
    var input = arrayfromargs(arguments);
    a0 = input[0];
    a1 = input[1];
    a2 = input[2];
    b1 = input[3];
    b2 = input[4];
}

function query( f ) {
	var freq_rad = f*pi/sr2;		//input freq to query in radians	
	calcul( freq_rad );
}

/* calculate amplitude response (mag) and phase of desired input freq */
function calcul ( f ) {				// f is input freq in radians
	var x1 = Math.cos( f  );
	var x2 = Math.cos( 2*f  );
	var y1 = Math.sin( f  );
	var y2 = Math.sin( 2*f  );
	
	var A = a0 + a1*x1 + a2*x2;
	var B = a1*y1 + a2*y2;
	var C = 1 + b1*x1 + b2*x2;
	var D = b1*y1 + b2*y2;
	
	var r = (A*C + B*D) / ( C*C + D*D );
	var i = (A*D - B*C) / (C*C + D*D);
	
	var mag = Math.sqrt(r*r + i*i);
	var phas = Math.atan2(i, r);
	
	outlet(1, phas);
	outlet(0, mag);
}


function tellme() {
	post( a0, a1, a2, b1, b2, "\n");
}