/**********************************************************************
Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

/*!
 * Sample kernel which multiplies every element of the input array with
 * a constant and stores it at the corresponding output array
 */


__kernel void templateKernel(__global  uint * output,
                             __global  float * plA,
							 __global  float * plB,
							 __global  float * plC,
							 __global  float * plD,
							 __global  float * pt1x,
							 __global  float * pt1y,
							 __global  float * pt1z,
							 __global  float * pt2x,
							 __global  float * pt2y,
							 __global  float * pt2z,
							 __global  float * abso,
							 __local  uint * cache,
                             const     unsigned int level)
{
    uint L = get_global_id(0);
	uint LL = L;
	float Sx = 0.2;
	float Sy = 0.4;
	float Sz = 0.3;
	float Rx = 0.6;
	float Ry = -1.7;
	float Rz = 0.4;
	uint N = 6;
	bool czy_zly = false;
    bool czy_wektor = false;
    for(int i = 0 ; i<level; i++)
    {
        cache[i] = L%N;
        L=L/N;
		float t = -(plA[cache[i]]*Sx + plB[cache[i]]*Sy + plC[cache[i]]*Sz + plD[cache[i]])/(plA[cache[i]]*plA[cache[i]] + plB[cache[i]]*plB[cache[i]] + plC[cache[i]]*plC[cache[i]]);
		Sx = 2*plA[cache[i]]*t+Sx;
		Sy = 2*plB[cache[i]]*t+Sy;
		Sz = 2*plC[cache[i]]*t+Sz;
		
    }
	
    float vNx = Rx-Sx;
	float vNy = Ry-Sy;
	float vNz = Rz-Sz;
	
    for( int i = level-1 ; i>=0 ; i--)
    {
        float t = -(plA[cache[i]]*Sx + plB[cache[i]]*Sy + plC[cache[i]]*Sz + plD[cache[i]])/(plA[cache[i]]*vNx + plB[cache[i]]*vNy + plC[cache[i]]*vNz);
		
        float Cx = vNx*t + Sx;
		float Cy = vNy*t + Sy;
		float Cz = vNz*t + Sz;
        if( vNx*plA[cache[i]] + vNy*plB[cache[i]] + vNz*plC[cache[i]] < 0)
		{
            czy_wektor = true;
			break;
		}
        float eps = 0.00001;
        if(Cx - pt1x[cache[i]] < -eps || Cx - pt2x[cache[i]] > eps || Cy - pt1y[cache[i]] < -eps || Cy - pt2y[cache[i]] > eps || Cz - pt1z[cache[i]] < -eps || Cz - pt2z[cache[i]] > eps)
        {
            czy_zly = true;
            break;
        }
        float t2 = -(plA[cache[i]]*vNx + plB[cache[i]]*vNy + plC[cache[i]]*vNz)/(plA[cache[i]]*plA[cache[i]] + plB[cache[i]]*plB[cache[i]] + plC[cache[i]]*plC[cache[i]]);

        vNx = 2*plA[cache[i]]*t2 + vNx;
		vNy = 2*plB[cache[i]]*t2 + vNy;
		vNz = 2*plC[cache[i]]*t2 + vNz;
        Sx = Cx;
		Sy = Cy;
		Sz = Cz;
    }
    
}
