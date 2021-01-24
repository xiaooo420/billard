#include "Plan.h"

double baseline = 500.0;
double spacing_X = 60 * 0.85;
double spacing_Y = 31;

double kugeln_X[16] =  {
	-635.0,
	baseline,
	baseline + spacing_X,
	baseline + spacing_X*2,
	baseline + spacing_X*3,
	baseline + spacing_X*3,
	baseline + spacing_X*4,
	baseline + spacing_X*4,
	baseline + spacing_X*2,
	baseline + spacing_X,
	baseline + spacing_X*2,
	baseline + spacing_X*3,
	baseline + spacing_X*3,
	baseline + spacing_X*4,
	baseline + spacing_X*4,
	baseline + spacing_X*4
};

double kugeln_Y[16]  = {
	spacing_Y * 0,
	spacing_Y * 0,
	- spacing_Y,
	- spacing_Y * 2,
	- spacing_Y,
	- spacing_Y * 3,
	spacing_Y * 2,
	spacing_Y * 4,
	spacing_Y * 0,
	spacing_Y,
	spacing_Y * 2,
	spacing_Y * 1,
	spacing_Y * 3,
	spacing_Y * 0,
	- spacing_Y * 2,
	- spacing_Y * 4
};

TColor colors[16] = {
	Weiss,
	Gelb,
	Blau,
	Hellrot,
	Violett,
	Gelb,
	Gruen,
	Rot,
	Schwarz,
	Gelb,
	Blau,
	Hellrot,
	Violett,
	Gelb,
	Gruen,
	Rot
};


