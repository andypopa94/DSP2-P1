#include "processing.h"
#include "iir.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FLT 32767


static Int16 coeff_lp[4];
static Int16 coeff_hp[4];
static Int16 coeff_m1[6];
static Int16 coeff_m2[6];


void calculateShelvingCoeff(float alpha, Int16* output)
{
	Int16 a = FLT * alpha;

	output[0] = a;
	output[1] = -1 * FLT;

	output[2] = 1*FLT - 1;
	output[3] = -a;
}


void calculatePeekCoeff(float alpha, float beta, Int16* output)
{
	Int16 a = FLT * alpha;
	Int16 b = FLT * beta;

	output[0] = a;
	output[1] = (-b * (1 + a)) >> 1;
	output[2] = 1 * FLT;

	output[3] = 1 * FLT;
	output[4] = (-b * (1 + a)) >> 1;
	output[5] = a;

}


void shelvingHP(Int16* input, Int16* coeff, Int16* z_x, Int16* z_y, Int16 n, Int16 k, Int16* output)
{
	int i;

	for (i = 0; i < n; i++)
	{
		Int16 tmp = first_order_IIR(input[i], coeff, z_x, z_y);
		Int16 tmp2 = input[i] + tmp;
		output[i] = ((input[i] - tmp) >> 1)  +  (k > 0? tmp2 << (k - 1) : tmp2 >> (1 - k));
	}
}


void shelvingLP(Int16* input, Int16* coeff, Int16* z_x, Int16* z_y, Int16 n, Int16 k, Int16* output)
{
	int i;

	for (i = 0; i < n; i++)
	{
		Int16 tmp = first_order_IIR(input[i], coeff, z_x, z_y);
		Int16 tmp2 = input[i] - tmp;
		output[i] = (k > 0? tmp2 << (k - 1) : tmp2 >> (1 - k))  +  ((input[i] + tmp) >> 1);
	}
}


void shelvingPeek(Int16* input, Int16* coeff, Int16* z_x, Int16* z_y, Int16 n, Int16 k, Int16* output)
{
	int i;

	for (i = 0; i < n; i++)
	{
		Int16 tmp = second_order_IIR(input[i], coeff, z_x, z_y);
		Int16 tmp2 = input[i] - tmp;
		output[i] = ((input[i] + tmp) >> 1)  +  (k > 0? tmp2 << (k - 1) : tmp2 >> (1 - k));
	}
}


void setAlphaBeta(float OmegaLP, float OmegaHP, float OmegaP1, float BOmegaP1, float OmegaP2, float BOmegaP2)
{
	calculateShelvingCoeff(0.3, coeff_lp);
	calculateShelvingCoeff(0.5, coeff_hp);
	calculatePeekCoeff(0.4, 0.1, coeff_m1);
	calculatePeekCoeff(0.5, 0.0, coeff_m2);

	// Waiting for table...
	/*calculateShelvingCoeff(1/cos(OmegaLP) + tan(OmegaLP), coeff_lp);
	calculateShelvingCoeff(1/cos(OmegaHP) + tan(OmegaHP), coeff_hp);
	calculatePeekCoeff(1/cos(BOmegaP1) + tan(BOmegaP1), cos(OmegaP1), coeff_m1);
	calculatePeekCoeff(1/cos(BOmegaP2) + tan(BOmegaP2), cos(OmegaP2), coeff_m2);*/
}


void equalize(Int16* input, Int16 n, int *k, Int16* output)
{
	Int16 z_x2[2], z_y2[2];
	Int16 z_x3[3], z_y3[3];

	shelvingLP(input, coeff_lp, z_x2, z_y2, n, k[0] - 2, input);
	shelvingPeek(input, coeff_m1, z_x3, z_y3, n, k[1] - 2, input);

	memset(z_x2, 0, sizeof(z_x2));
	memset(z_y2, 0, sizeof(z_y2));
	memset(z_x3, 0, sizeof(z_x3));
	memset(z_y3, 0, sizeof(z_y3));

	shelvingPeek(input, coeff_m2, z_x3, z_y3, n, k[2] - 2, input);
	shelvingHP(input, coeff_m2, z_x2, z_y2, n, k[3] - 2, output);
}
