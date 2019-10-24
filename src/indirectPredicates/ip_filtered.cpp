/****************************************************************************
* Indirect exact 3D orientation predicates with floating point filter       *
*                                                                           *
* Consiglio Nazionale delle Ricerche                                        *
* Istituto di Matematica Applicata e Tecnologie Informatiche                *
* Sezione di Genova                                                         *
* IMATI-GE / CNR                                                            *
*                                                                           *
* Authors: Marco Attene                                                     *
* Copyright(C) 2019: IMATI-GE / CNR                                         *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU Lesser General Public License as published  *
* by the Free Software Foundation; either version 3 of the License, or (at  *
* your option) any later version.                                           *
*                                                                           *
* This program is distributed in the hope that it will be useful, but       *
* WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser  *
* General Public License for more details.                                  *
*                                                                           *
* You should have received a copy of the GNU Lesser General Public License  *
* along with this program.  If not, see http://www.gnu.org/licenses/.       *
*                                                                           *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// To compile on MSVC: use /fp:strict
// On GNU GCC: use -frounding-math
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <fenv.h>
#include "ip_filtered.h"
#include<iostream>
#ifndef WIN32
#if __APPLE__
#define _FPU_SETCW(cw) // nothing
#else
#include <fpu_control.h>
#endif
#endif


// The following 'monolithic' versions are commented and replaced by the new versions
// at the end of this file.

//int orient3D_LPI_filtered(
//	double px, double py, double pz,
//	double qx, double qy, double qz,
//	double rx, double ry, double rz,
//	double sx, double sy, double sz,
//	double tx, double ty, double tz,
//	double ax, double ay, double az,
//	double bx, double by, double bz,
//	double cx, double cy, double cz)
//{
//	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
//
//	double a11, a12, a13, a21, a22, a23, a31, a32, a33, d21, d31, d22, d32, d23, d33;
//	double px_rx, py_ry, pz_rz, px_cx, py_cy, pz_cz;
//	double a2233, a2133, a2132;
//	double d, n;
//	double d11, d12, d13;
//	double d2233, d2332, d2133, d2331, d2132, d2231;
//	double det;
//
//	a11 = (px - qx);
//	a12 = (py - qy);
//	a13 = (pz - qz);
//	a21 = (sx - rx);
//	a22 = (sy - ry);
//	a23 = (sz - rz);
//	a31 = (tx - rx);
//	a32 = (ty - ry);
//	a33 = (tz - rz);
//	a2233 = ((a22 * a33) - (a23 * a32));
//	a2133 = ((a21 * a33) - (a23 * a31));
//	a2132 = ((a21 * a32) - (a22 * a31));
//	d = (((a11 * a2233) - (a12 * a2133)) + (a13 * a2132));
//
//
//	// Almost static filter for d
//
//	double fa11 = fabs(a11);
//	double fa21 = fabs(a21);
//	double fa31 = fabs(a31);
//	double fa12 = fabs(a12);
//	double fa22 = fabs(a22);
//	double fa32 = fabs(a32);
//	double fa13 = fabs(a13);
//	double fa23 = fabs(a23);
//	double fa33 = fabs(a33);
//
//	double max1, max2, max5;
//
//	max1 = fa23;
//	if (max1 < fa13) max1 = fa13;
//	if (max1 < fa33) max1 = fa33;
//
//	max2 = fa12;
//	if (max2 < fa22) max2 = fa22;
//	if (max2 < fa32) max2 = fa32;
//
//	max5 = fa11;
//	if (max5 < fa21) max5 = fa21;
//	if (max5 < fa31) max5 = fa31;
//
//	double deps = 5.1107127829973299e-015 * max1 * max2 * max5;
//	if (d <= deps && d >= -deps) return Filtered_Orientation::UNCERTAIN;
//
//
//	px_rx = px - rx;
//	py_ry = py - ry;
//	pz_rz = pz - rz;
//	n = ((((py_ry)* a2133) - ((px_rx)* a2233)) - ((pz_rz)* a2132));
//
//	px_cx = px - cx;
//	py_cy = py - cy;
//	pz_cz = pz - cz;
//
//	d11 = (d * px_cx) + (a11 * n);
//	d21 = (ax - cx);
//	d31 = (bx - cx);
//	d12 = (d * py_cy) + (a12 * n);
//	d22 = (ay - cy);
//	d32 = (by - cy);
//	d13 = (d * pz_cz) + (a13 * n);
//	d23 = (az - cz);
//	d33 = (bz - cz);
//
//	d2233 = d22*d33;
//	d2332 = d23*d32;
//	d2133 = d21*d33;
//	d2331 = d23*d31;
//	d2132 = d21*d32;
//	d2231 = d22*d31;
//
//	det = d11 * (d2233 - d2332) - d12 * (d2133 - d2331) + d13 * (d2132 - d2231);
//
//	if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return Filtered_Orientation::UNCERTAIN; // Fast reject in case of under/overflow
//
//	// An additional static filter here may be advantageous... possibly initialized based on input coordinates
//
//	// Almost static filter
//
//	double fpxrx = fabs(px_rx);
//	double fpyry = fabs(py_ry);
//	double fpzrz = fabs(pz_rz);
//	double fd11 = fabs(d11);
//	double fd21 = fabs(d21);
//	double fd31 = fabs(d31);
//	double fd12 = fabs(d12);
//	double fd22 = fabs(d22);
//	double fd32 = fabs(d32);
//	double fd13 = fabs(d13);
//	double fd23 = fabs(d23);
//	double fd33 = fabs(d33);
//	double fpxcx = fabs(px_cx);
//	double fpycy = fabs(py_cy);
//	double fpzcz = fabs(pz_cz);
//
//	if (max1 < fpzrz) max1 = fpzrz;
//	if (max2 < fpyry) max2 = fpyry;
//	if (max5 < fpxrx) max5 = fpxrx;
//
//	double max3, max4, max6;
//
//	max3 = fa12;
//	if (max3 < fd32) max3 = fd32;
//	if (max3 < fd22) max3 = fd22;
//	if (max3 < fpycy) max3 = fpycy;
//
//	max4 = fa13;
//	if (max4 < fpzcz) max4 = fpzcz;
//	if (max4 < fd33) max4 = fd33;
//	if (max4 < fd23) max4 = fd23;
//
//	max6 = fa11;
//	if (max6 < fd21) max6 = fd21;
//	if (max6 < fd31) max6 = fd31;
//	if (max6 < fpxcx) max6 = fpxcx;
//
//	double eps = 1.3865993466947057e-013 * max2 * max1 * max5 * max6 * max3 * max4;
//
//	if ((det > eps)) return (d>0) ? (Filtered_Orientation::POSITIVE) : (Filtered_Orientation::NEGATIVE);
//	if ((det < -eps)) return (d>0) ? (Filtered_Orientation::NEGATIVE) : (Filtered_Orientation::POSITIVE);
//	return Filtered_Orientation::UNCERTAIN;
//}
//
//
//
//
//int orient3D_TPI_filtered(
//	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
//	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
//	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
//	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
//	)
//{
//	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
//
//	v3x -= v2x;
//	v3y -= v2y;
//	v3z -= v2z;
//	v2x -= v1x;
//	v2y -= v1y;
//	v2z -= v1z;
//	double nvx = v2y*v3z - v2z*v3y;
//	double nvy = v3x*v2z - v3z*v2x;
//	double nvz = v2x*v3y - v2y*v3x;
//
//	w3x -= w2x;
//	w3y -= w2y;
//	w3z -= w2z;
//	w2x -= w1x;
//	w2y -= w1y;
//	w2z -= w1z;
//	double nwx = w2y*w3z - w2z*w3y;
//	double nwy = w3x*w2z - w3z*w2x;
//	double nwz = w2x*w3y - w2y*w3x;
//
//	u3x -= u2x;
//	u3y -= u2y;
//	u3z -= u2z;
//	u2x -= u1x;
//	u2y -= u1y;
//	u2z -= u1z;
//	double nux = u2y*u3z - u2z*u3y;
//	double nuy = u3x*u2z - u3z*u2x;
//	double nuz = u2x*u3y - u2y*u3x;
//
//	double nwyuz = nwy*nuz - nwz*nuy;
//	double nwxuz = nwx*nuz - nwz*nux;
//	double nwxuy = nwx*nuy - nwy*nux;
//
//	double nvyuz = nvy*nuz - nvz*nuy;
//	double nvxuz = nvx*nuz - nvz*nux;
//	double nvxuy = nvx*nuy - nvy*nux;
//
//	double nvywz = nvy*nwz - nvz*nwy;
//	double nvxwz = nvx*nwz - nvz*nwx;
//	double nvxwy = nvx*nwy - nvy*nwx;
//
//	double d = nvx*nwyuz - nvy*nwxuz + nvz*nwxuy;
//
//
//	// Almost static filter for d
//	double fv2x = fabs(v2x);
//	double fv2y = fabs(v2y);
//	double fv2z = fabs(v2z);
//	double fv3x = fabs(v3x);
//	double fv3y = fabs(v3y);
//	double fv3z = fabs(v3z);
//
//	double fw2x = fabs(w2x);
//	double fw2y = fabs(w2y);
//	double fw2z = fabs(w2z);
//	double fw3x = fabs(w3x);
//	double fw3y = fabs(w3y);
//	double fw3z = fabs(w3z);
//
//	double fu2x = fabs(u2x);
//	double fu3x = fabs(u3x);
//	double fu2y = fabs(u2y);
//	double fu2z = fabs(u2z);
//	double fu3y = fabs(u3y);
//	double fu3z = fabs(u3z);
//
//	double max2, max4, max5, max7;
//
//	max4 = fv2y;
//	if (max4 < fv3y) max4 = fv3y;
//	if (max4 < fw3y) max4 = fw3y;
//	if (max4 < fw2y) max4 = fw2y;
//	max2 = fv3x;
//	if (max2 < fv2x) max2 = fv2x;
//	if (max2 < fw2x) max2 = fw2x;
//	if (max2 < fw3x) max2 = fw3x;
//	max5 = fv2z;
//	if (max5 < fv3z) max5 = fv3z;
//	if (max5 < fw3z) max5 = fw3z;
//	if (max5 < fw2z) max5 = fw2z;
//	max7 = fu2x;
//	if (max7 < fu3x) max7 = fu3x;
//	if (max7 < fw2x) max7 = fw2x;
//	if (max7 < fw3x) max7 = fw3x;
//
//	double max9 = fu2y;
//	if (max9 < fu3y) max9 = fu3y;
//	if (max9 < fw2y) max9 = fw2y;
//	if (max9 < fw3y) max9 = fw3y;
//	double max10 = fu2y;
//	if (max10 < fu3z) max10 = fu3z;
//	if (max10 < fw2z) max10 = fw2z;
//	if (max10 < fw3z) max10 = fw3z;
//
//	double deps = 8.8881169117764924e-014 * (((((max4 * max5) * max2) * max10) * max7) * max9);
//	if (d <= deps && d >= -deps) return Filtered_Orientation::UNCERTAIN;
//
//
//	double p1 = nvx*v1x + nvy*v1y + nvz*v1z;
//	double p2 = nwx*w1x + nwy*w1y + nwz*w1z;
//	double p3 = nux*u1x + nuy*u1y + nuz*u1z;
//
//	double n1 = p1*nwyuz - p2*nvyuz + p3*nvywz;
//	double n2 = p2*nvxuz - p3*nvxwz - p1*nwxuz;
//	double n3 = p3*nvxwy - p2*nvxuy + p1*nwxuy;
//
//	double dq3x = d*q3x;
//	double dq3y = d*q3y;
//	double dq3z = d*q3z;
//
//	double a11 = n1 - dq3x;
//	double a12 = n2 - dq3y;
//	double a13 = n3 - dq3z;
//	double a21 = q1x - q3x;
//	double a22 = q1y - q3y;
//	double a23 = q1z - q3z;
//	double a31 = q2x - q3x;
//	double a32 = q2y - q3y;
//	double a33 = q2z - q3z;
//
//	double det = a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);
//
//	if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return Filtered_Orientation::UNCERTAIN; // Fast reject in case of under/overflow
//	
//	// Almost static filter
//
//	double fa21 = fabs(a21);
//	double fa22 = fabs(a22);
//	double fa23 = fabs(a23);
//	double fa31 = fabs(a31);
//	double fa32 = fabs(a32);
//	double fa33 = fabs(a33);
//
//	double max1, max3, max6, max8;
//
//	if (max4 < fu2y) max4 = fu2y;
//	if (max4 < fu3y) max4 = fu3y;
//	if (max2 < fu2x) max2 = fu2x;
//	if (max2 < fu3x) max2 = fu3x;
//	if (max5 < fu2z) max5 = fu2z;
//	if (max5 < fu3z) max5 = fu3z;
//	if (max7 < fa21) max7 = fa21;
//	if (max7 < fa31) max7 = fa31;
//
//	max1 = max4;
//	if (max1 < max2) max1 = max2;
//	max3 = max5;
//	if (max3 < max4) max3 = max4;
//	max6 = fu2x;
//	if (max6 < fu3x) max6 = fu3x;
//	if (max6 < fu2z) max6 = fu2z;
//	if (max6 < fw3y) max6 = fw3y;
//	if (max6 < fw2x) max6 = fw2x;
//	if (max6 < fw3z) max6 = fw3z;
//	if (max6 < fw2y) max6 = fw2y;
//	if (max6 < fw2z) max6 = fw2z;
//	if (max6 < fu2y) max6 = fu2y;
//	if (max6 < fu3z) max6 = fu3z;
//	if (max6 < fu3y) max6 = fu3y;
//	if (max6 < fw3x) max6 = fw3x;
//	if (max6 < fa22) max6 = fa22;
//	if (max6 < fa32) max6 = fa32;
//	max8 = fa22;
//	if (max8 < fa23) max8 = fa23;
//	if (max8 < fa33) max8 = fa33;
//	if (max8 < fa32) max8 = fa32;
//
//	double eps = 3.4025182954957945e-012 * (((((((max1 * max3) * max2) * max5) * max7) * max4) * max6) * max8);
//
//	if ((det > eps)) return (d>0) ? (Filtered_Orientation::POSITIVE) : (Filtered_Orientation::NEGATIVE);
//	if ((det < -eps)) return (d>0) ? (Filtered_Orientation::NEGATIVE) : (Filtered_Orientation::POSITIVE);
//	return Filtered_Orientation::UNCERTAIN;
//}


int lpre = 0, lpost = 0, lpree = 0, lposte = 0, tpre = 0, tposet = 0, tpree = 0, tposte = 0;
void count_ip() {
	std::cout << "lpi: pre,post,pre_exact,post_exact " << lpre << " " << lpost << " " << lpree << " " << lposte << " " << std::endl;
	std::cout << "tpi: pre,post,pre_exact,post_exact " << tpre << " " << tposet << " " << tpree << " " << tposte << " " << std::endl;
}
bool orient3D_LPI_prefilter(
	const double& px, const double& py, const double& pz,
	const double& qx, const double& qy, const double& qz,
	const double& rx, const double& ry, const double& rz,
	const double& sx, const double& sy, const double& sz,
	const double& tx, const double& ty, const double& tz,
	double& a11, double& a12, double& a13,
	double& d,
	double& fa11, double& fa12, double& fa13,
	double& max1, double& max2, double& max5)
{
	double a21, a22, a23, a31, a32, a33;

	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
	lpre++;
	a11 = (px - qx);
	a12 = (py - qy);
	a13 = (pz - qz);
	a21 = (sx - rx);
	a22 = (sy - ry);
	a23 = (sz - rz);
	a31 = (tx - rx);
	a32 = (ty - ry);
	a33 = (tz - rz);
	double a2233 = ((a22 * a33) - (a23 * a32));
	double a2133 = ((a21 * a33) - (a23 * a31));
	double a2132 = ((a21 * a32) - (a22 * a31));
	d = (((a11 * a2233) - (a12 * a2133)) + (a13 * a2132));

	fa11 = fabs(a11);
	double fa21 = fabs(a21);
	double fa31 = fabs(a31);
	fa12 = fabs(a12);
	double fa22 = fabs(a22);
	double fa32 = fabs(a32);
	fa13 = fabs(a13);
	double fa23 = fabs(a23);
	double fa33 = fabs(a33);

	max1 = fa23;
	if (max1 < fa13) max1 = fa13;
	if (max1 < fa33) max1 = fa33;

	max2 = fa12;
	if (max2 < fa22) max2 = fa22;
	if (max2 < fa32) max2 = fa32;

	max5 = fa11;
	if (max5 < fa21) max5 = fa21;
	if (max5 < fa31) max5 = fa31;

	double deps = 5.1107127829973299e-015 * max1 * max2 * max5;
	if (d <= deps && d >= -deps) return false;

	double px_rx = px - rx;
	double py_ry = py - ry;
	double pz_rz = pz - rz;

	double n = ((((py_ry)* a2133) - ((px_rx)* a2233)) - ((pz_rz)* a2132));
	
	a11 *= n;
	a12 *= n;
	a13 *= n;

	if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return false;

	double fpxrx = fabs(px_rx);
	double fpyry = fabs(py_ry);
	double fpzrz = fabs(pz_rz);

	if (max1 < fpzrz) max1 = fpzrz;
	if (max2 < fpyry) max2 = fpyry;
	if (max5 < fpxrx) max5 = fpxrx;

	return true;
}

int orient3D_LPI_postfilter(
	const double& a11, const double& a12, const double& a13,
	const double& d,
	const double& fa11, const double& fa12, const double& fa13,
	const double& max1, const double& max2, const double& max5,
	const double& px, const double& py, const double& pz,
	const double& ax, const double& ay, const double& az,
	const double& bx, const double& by, const double& bz,
	const double& cx, const double& cy, const double& cz)
{
	double px_cx, py_cy, pz_cz;
	double d11, d12, d13, d21, d31, d22, d32, d23, d33;
	double d2233, d2332, d2133, d2331, d2132, d2231;
	double det;

	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
	lpost++;
	px_cx = px - cx;
	py_cy = py - cy;
	pz_cz = pz - cz;

	d11 = (d * px_cx) + (a11);
	d21 = (ax - cx);
	d31 = (bx - cx);
	d12 = (d * py_cy) + (a12);
	d22 = (ay - cy);
	d32 = (by - cy);
	d13 = (d * pz_cz) + (a13);
	d23 = (az - cz);
	d33 = (bz - cz);

	d2233 = d22*d33;
	d2332 = d23*d32;
	d2133 = d21*d33;
	d2331 = d23*d31;
	d2132 = d21*d32;
	d2231 = d22*d31;

	det = d11 * (d2233 - d2332) - d12 * (d2133 - d2331) + d13 * (d2132 - d2231);

	if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return Filtered_Orientation::UNCERTAIN; // Fast reject in case of under/overflow

	double fd11 = fabs(d11);
	double fd21 = fabs(d21);
	double fd31 = fabs(d31);
	double fd12 = fabs(d12);
	double fd22 = fabs(d22);
	double fd32 = fabs(d32);
	double fd13 = fabs(d13);
	double fd23 = fabs(d23);
	double fd33 = fabs(d33);
	double fpxcx = fabs(px_cx);
	double fpycy = fabs(py_cy);
	double fpzcz = fabs(pz_cz);

	double max3, max4, max6;

	max3 = fa12;
	if (max3 < fd32) max3 = fd32;
	if (max3 < fd22) max3 = fd22;
	if (max3 < fpycy) max3 = fpycy;

	max4 = fa13;
	if (max4 < fpzcz) max4 = fpzcz;
	if (max4 < fd33) max4 = fd33;
	if (max4 < fd23) max4 = fd23;

	max6 = fa11;
	if (max6 < fd21) max6 = fd21;
	if (max6 < fd31) max6 = fd31;
	if (max6 < fpxcx) max6 = fpxcx;

	double eps = 1.3865993466947057e-013 * max1 * max2 * max5 * max6 * max3 * max4;

	if ((det > eps)) return (d>0) ? (Filtered_Orientation::POSITIVE) : (Filtered_Orientation::NEGATIVE);
	if ((det < -eps)) return (d>0) ? (Filtered_Orientation::NEGATIVE) : (Filtered_Orientation::POSITIVE);
	return Filtered_Orientation::UNCERTAIN;
}


int orient3D_LPI_filtered(
	double px, double py, double pz,
	double qx, double qy, double qz,
	double rx, double ry, double rz,
	double sx, double sy, double sz,
	double tx, double ty, double tz,
	double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	double a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;

	if (!orient3D_LPI_prefilter(px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz, tx, ty, tz, a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5))
		return Filtered_Orientation::UNCERTAIN;
	return orient3D_LPI_postfilter(a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5, px, py, pz, ax, ay, az, bx, by, bz, cx, cy, cz);
}


bool orient3D_TPI_prefilter(
	const double& ov1x, const double& ov1y, const double& ov1z, const double& ov2x, const double& ov2y, const double& ov2z, const double& ov3x, const double& ov3y, const double& ov3z,
	const double& ow1x, const double& ow1y, const double& ow1z, const double& ow2x, const double& ow2y, const double& ow2z, const double& ow3x, const double& ow3y, const double& ow3z,
	const double& ou1x, const double& ou1y, const double& ou1z, const double& ou2x, const double& ou2y, const double& ou2z, const double& ou3x, const double& ou3y, const double& ou3z,
	double& d, double& n1, double& n2, double& n3,
	double& max1, double& max2, double& max3, double& max4, double& max5, double& max6, double& max7
	)
{
	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
	tpre++;
	double v3x = ov3x - ov2x;
	double v3y = ov3y - ov2y;
	double v3z = ov3z - ov2z;
	double v2x = ov2x - ov1x;
	double v2y = ov2y - ov1y;
	double v2z = ov2z - ov1z;
	double w3x = ow3x - ow2x;
	double w3y = ow3y - ow2y;
	double w3z = ow3z - ow2z;
	double w2x = ow2x - ow1x;
	double w2y = ow2y - ow1y;
	double w2z = ow2z - ow1z;
	double u3x = ou3x - ou2x;
	double u3y = ou3y - ou2y;
	double u3z = ou3z - ou2z;
	double u2x = ou2x - ou1x;
	double u2y = ou2y - ou1y;
	double u2z = ou2z - ou1z;

	double nvx = v2y*v3z - v2z*v3y;
	double nvy = v3x*v2z - v3z*v2x;
	double nvz = v2x*v3y - v2y*v3x;

	double nwx = w2y*w3z - w2z*w3y;
	double nwy = w3x*w2z - w3z*w2x;
	double nwz = w2x*w3y - w2y*w3x;

	double nux = u2y*u3z - u2z*u3y;
	double nuy = u3x*u2z - u3z*u2x;
	double nuz = u2x*u3y - u2y*u3x;

	double nwyuz = nwy*nuz - nwz*nuy;
	double nwxuz = nwx*nuz - nwz*nux;
	double nwxuy = nwx*nuy - nwy*nux;

	double nvyuz = nvy*nuz - nvz*nuy;
	double nvxuz = nvx*nuz - nvz*nux;
	double nvxuy = nvx*nuy - nvy*nux;

	double nvywz = nvy*nwz - nvz*nwy;
	double nvxwz = nvx*nwz - nvz*nwx;
	double nvxwy = nvx*nwy - nvy*nwx;

	d = nvx*nwyuz - nvy*nwxuz + nvz*nwxuy;


	// Almost static filter for d
	double fv2x = fabs(v2x);
	double fv2y = fabs(v2y);
	double fv2z = fabs(v2z);
	double fv3x = fabs(v3x);
	double fv3y = fabs(v3y);
	double fv3z = fabs(v3z);

	double fw2x = fabs(w2x);
	double fw2y = fabs(w2y);
	double fw2z = fabs(w2z);
	double fw3x = fabs(w3x);
	double fw3y = fabs(w3y);
	double fw3z = fabs(w3z);

	double fu2x = fabs(u2x);
	double fu3x = fabs(u3x);
	double fu2y = fabs(u2y);
	double fu2z = fabs(u2z);
	double fu3y = fabs(u3y);
	double fu3z = fabs(u3z);

	max4 = fv2y;
	if (max4 < fv3y) max4 = fv3y;
	if (max4 < fw3y) max4 = fw3y;
	if (max4 < fw2y) max4 = fw2y;
	max2 = fv3x;
	if (max2 < fv2x) max2 = fv2x;
	if (max2 < fw2x) max2 = fw2x;
	if (max2 < fw3x) max2 = fw3x;
	max5 = fv2z;
	if (max5 < fv3z) max5 = fv3z;
	if (max5 < fw3z) max5 = fw3z;
	if (max5 < fw2z) max5 = fw2z;
	max7 = fu2x;
	if (max7 < fu3x) max7 = fu3x;
	if (max7 < fw2x) max7 = fw2x;
	if (max7 < fw3x) max7 = fw3x;

	double max9 = fu2y;
	if (max9 < fu3y) max9 = fu3y;
	if (max9 < fw2y) max9 = fw2y;
	if (max9 < fw3y) max9 = fw3y;
	double max10 = fu2y;
	if (max10 < fu3z) max10 = fu3z;
	if (max10 < fw2z) max10 = fw2z;
	if (max10 < fw3z) max10 = fw3z;

	double deps = 8.8881169117764924e-014 * (((((max4 * max5) * max2) * max10) * max7) * max9);
	if (d <= deps && d >= -deps) return false;


	double p1 = nvx*ov1x + nvy*ov1y + nvz*ov1z;
	double p2 = nwx*ow1x + nwy*ow1y + nwz*ow1z;
	double p3 = nux*ou1x + nuy*ou1y + nuz*ou1z;

	n1 = p1*nwyuz - p2*nvyuz + p3*nvywz;
	n2 = p2*nvxuz - p3*nvxwz - p1*nwxuz;
	n3 = p3*nvxwy - p2*nvxuy + p1*nwxuy;

	if (max4 < fu2y) max4 = fu2y;
	if (max4 < fu3y) max4 = fu3y;
	if (max2 < fu2x) max2 = fu2x;
	if (max2 < fu3x) max2 = fu3x;
	if (max5 < fu2z) max5 = fu2z;
	if (max5 < fu3z) max5 = fu3z;

	max1 = max4;
	if (max1 < max2) max1 = max2;
	max3 = max5;
	if (max3 < max4) max3 = max4;
	max6 = fu2x;
	if (max6 < fu3x) max6 = fu3x;
	if (max6 < fu2z) max6 = fu2z;
	if (max6 < fw3y) max6 = fw3y;
	if (max6 < fw2x) max6 = fw2x;
	if (max6 < fw3z) max6 = fw3z;
	if (max6 < fw2y) max6 = fw2y;
	if (max6 < fw2z) max6 = fw2z;
	if (max6 < fu2y) max6 = fu2y;
	if (max6 < fu3z) max6 = fu3z;
	if (max6 < fu3y) max6 = fu3y;
	if (max6 < fw3x) max6 = fw3x;

	return true;
}


int orient3D_TPI_postfilter(
	const double& d, const double& n1, const double& n2, const double& n3,
	const double& max1, const double& max2, const double& max3, const double& max4, const double& max5, const double& max6, const double& max7,
	const double& q1x, const double& q1y, const double& q1z, const double& q2x, const double& q2y, const double& q2z, const double& q3x, const double& q3y, const double& q3z
	)
{
	::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
	tposet++;
	double dq3x = d*q3x;
	double dq3y = d*q3y;
	double dq3z = d*q3z;

	double a11 = n1 - dq3x;
	double a12 = n2 - dq3y;
	double a13 = n3 - dq3z;
	double a21 = q1x - q3x;
	double a22 = q1y - q3y;
	double a23 = q1z - q3z;
	double a31 = q2x - q3x;
	double a32 = q2y - q3y;
	double a33 = q2z - q3z;

	double det = a11*(a22*a33 - a23*a32) - a12*(a21*a33 - a23*a31) + a13*(a21*a32 - a22*a31);

	if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return Filtered_Orientation::UNCERTAIN; // Fast reject in case of under/overflow

	double fa21 = fabs(a21);
	double fa22 = fabs(a22);
	double fa23 = fabs(a23);
	double fa31 = fabs(a31);
	double fa32 = fabs(a32);
	double fa33 = fabs(a33);

	double nmax7 = max7;
	if (nmax7 < fa21) nmax7 = fa21;
	if (nmax7 < fa31) nmax7 = fa31;

	double nmax6 = max6;
	if (nmax6 < fa22) nmax6 = fa22;
	if (nmax6 < fa32) nmax6 = fa32;

	double max8 = fa22;
	if (max8 < fa23) max8 = fa23;
	if (max8 < fa33) max8 = fa33;
	if (max8 < fa32) max8 = fa32;

	double eps = 3.4025182954957945e-012 * (((((((max1 * max3) * max2) * max5) * nmax7) * max4) * nmax6) * max8);

	if ((det > eps)) return (d>0) ? (Filtered_Orientation::POSITIVE) : (Filtered_Orientation::NEGATIVE);
	if ((det < -eps)) return (d>0) ? (Filtered_Orientation::NEGATIVE) : (Filtered_Orientation::POSITIVE);
	return Filtered_Orientation::UNCERTAIN;
}

int orient3D_TPI_filtered(
	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
	)
{
	double d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7;

	if (!orient3D_TPI_prefilter(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z, u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z, d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7))
		return Filtered_Orientation::UNCERTAIN;
	return orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, q1x, q1y, q1z, q2x, q2y, q2z, q3x, q3y, q3z);
}

#define TO_NONNEGATIVE(d) ((d) = fabs(d))


// The following macros are fast implementations of basic expansion arithmetic due
// to Dekker, Knuth, Priest, Shewchuk, and others.

// See Y. Hida, X. S. Li,  D. H. Bailey "Algorithms for Quad-Double Precision Floating Point Arithmetic"

// Sums
#define Quick_Two_Sum(a, b, x, y) x = a + b; y = b - (x - a)
#define Two_Sum(a, b, x, y) x = a + b; _bv = x - a; y = (a - (x - _bv)) + (b - _bv)
#define Two_One_Sum(a1, a0, b, x2, x1, x0) Two_Sum(a0, b , _i, x0); Two_Sum(a1, _i, x2, x1)

// Differences
#define Two_Diff(a, b, x, y) x = a - b; _bv = a - x; y = (a - (x + _bv)) + (_bv - b)
#define Two_One_Diff(a1, a0, b, x2, x1, x0) Two_Diff(a0, b , _i, x0); Two_Sum( a1, _i, x2, x1)

// Products
#define Split(a, _ah, _al) _c = 1.3421772800000003e+008 * a; _ah = _c - (_c - a); _al = a - _ah
#define Two_Prod_PreSplit(a, b, _bh, _bl, x, y) x = a * b; Split(a, _ah, _al); y = (_al * _bl) - (((x - (_ah * _bh)) - (_al * _bh)) - (_ah * _bl))
#define Two_Product_2Presplit(a, _ah, _al, b, _bh, _bl, x, y) x = a * b; y = (_al * _bl) - (((x - _ah * _bh) - (_al * _bh)) - (_ah * _bl))


// An instance of the following must be created to access functions for expansion arithmetic
class expansionObject
{
	// Temporary vars used in low-level arithmetic
	double _bv, _c, _ah, _al, _bh, _bl, _ch, _cl, _i, _j, _k, _l, _m, _n, _0, _1, _2, _u3;

public:
	expansionObject() {}

	inline void two_Sum(const double& a, const double&b, double& x, double& y) { Two_Sum(a, b, x, y); }
	inline void two_Diff(const double& a, const double&b, double& x, double& y) { Two_Diff(a, b, x, y); }

	// [x,y] = [a]*[b]		 Multiplies two expansions [a] and [b] of length one
	inline void Two_Prod(const double& a, const double&b, double& x, double& y)
	{
		_u3 = a * b;
		Split(a, _ah, _al); Split(b, _bh, _bl);
		y = ((_ah*_bh - _u3) + _ah*_bl + _al*_bh) + _al*_bl;
		x = _u3;
	}
	inline void Two_Prod(const double& a, const double&b, double*xy) { Two_Prod(a, b, xy[1], xy[0]); }

	// [x,y] = [a]^2		Squares an expansion of length one
	inline void Square(const double& a, double& x, double& y)
	{
		x = a * a;
		Split(a, _ah, _al);
		y = (_al * _al) - ((x - (_ah * _ah)) - ((_ah + _ah) * _al));
	}

	// [x3,x2,x1,x0] = [a1,a0]*[b]		Multiplies an expansion [a1,a0] of length two by an expansion [b] of length one
	inline void Two_One_Prod(const double& a1, const double&a0, const double& b, double& x3, double& x2, double& x1, double& x0)
	{
		Split(b, _bh, _bl);
		Two_Prod_PreSplit(a0, b, _bh, _bl, _i, x0); Two_Prod_PreSplit(a1, b, _bh, _bl, _j, _0);
		Two_Sum(_i, _0, _k, x1); Quick_Two_Sum(_j, _k, x3, x2);
	}
	inline void Two_One_Prod(const double *a, const double& b, double *x) { Two_One_Prod(a[1], a[0], b, x[3], x[2], x[1], x[0]); }

	// [x3,x2,x1,x0] = [a1,a0]+[b1,b0]		Calculates the sum of two expansions of length two
	inline void Two_Two_Sum(const double& a1, const double&a0, const double& b1, const double& b0, double& x3, double& x2, double& x1, double& x0)
	{
		Two_One_Sum(a1, a0, b0, _j, _0, x0); Two_One_Sum(_j, _0, b1, x3, x2, x1);
	}

	// [x3,x2,x1,x0] = [a1,a0]-[b1,b0]		Calculates the difference between two expansions of length two
	inline void Two_Two_Diff(const double& a1, const double&a0, const double& b1, const double& b0, double& x3, double& x2, double& x1, double& x0)
	{
		Two_One_Diff(a1, a0, b0, _j, _0, x0); Two_One_Diff(_j, _0, b1, _u3, x2, x1); x3 = _u3;
	}
	inline void Two_Two_Diff(const double *a, const double *b, double *x) { Two_Two_Diff(a[1], a[0], b[1], b[0], x[3], x[2], x[1], x[0]); }

	// Calculates the second component 'y' of the expansion [x,y] = [a]-[b] when 'x' is known
	inline void Two_Diff_Back(const double& a, const double&b, double& x, double& y) { _bv = a - x; y = (a - (x + _bv)) + (_bv - b); }
	inline void Two_Diff_Back(const double& a, const double&b, double *xy) { Two_Diff_Back(a, b, xy[1], xy[0]); }

	// [h] = [a1,a0]^2		Squares an expansion of length 2
	// 'h' must be allocated by the caller with 6 components.
	void Two_Square(const double& a1, const double& a0, double *x);

	// [h7,h6,...,h0] = [a1,a0]*[b1,b0]		Calculates the product of two expansions of length two.
	// 'h' must be allocated by the caller with eight components.
	void Two_Two_Prod(const double a1, const double a0, const double b1, const double b0, double *h);

	// [e] <- 2*[e]		Inplace inversion
	inline void Gen_Invert(const int elen, double *e) { for (int i = 0; i < elen; i++) e[i] = -e[i]; }

	// [h] = [e] + [f]		Sums two expansions and returns number of components of result
	// 'h' must be allocated by the caller with at least elen+flen components.
	int Gen_Sum(const int elen, const double *e, const int flen, const double *f, double *h);

	// Same as above, but 'h' is allocated internally. The caller must still call 'free' to release the memory.
	int Gen_Sum_With_Alloc(const int elen, const double *e, const int flen, const double *f, double **h)
	{
		*h = (double *)malloc((elen + flen) * sizeof(double));
		return Gen_Sum(elen, e, flen, f, *h);
	}

	// [h] = [e] * b		Multiplies an expansion by a scalar
	// 'h' must be allocated by the caller with at least elen*2 components.
	int Gen_Scale(const int elen, const double *e, const double& b, double *h);

	// [h] = [a] * [b]
	// 'h' must be allocated by the caller with at least 2*alen*blen components.
	int Sub_product(const int alen, const double *a, const int blen, const double *b, double *h);

	// [h] = [a] * [b]
	// 'h' must be allocated by the caller with at least MAX(2*alen*blen, 8) components.
	int Gen_Product(const int alen, const double *a, const int blen, const double *b, double *h);

	// Same as above, but 'h' is allocated internally. The caller must still call 'free' to release the memory.
	int Gen_Product_With_Alloc(const int alen, const double *a, const int blen, const double *b, double **h)
	{
		int h_len = alen * blen * 2;
		if (h_len < 8) h_len = 8;
		*h = (double *)malloc(h_len * sizeof(double));
		return Gen_Product(alen, a, blen, b, *h);
	}


	// Assume that *h is pre-allocated with hlen doubles.
	// If more elements are required, *h is re-allocated internally.
	// In any case, the function returns the size of the resulting expansion.
	// The caller must verify whether reallocation took place, and possibly call 'free' to release the memory.
	// When reallocation takes place, *h becomes different from its original value.

	int Gen_Scale_With_PreAlloc(const int elen, const double *e, const double& b, double **h, const int hlen)
	{
		int newlen = elen * 2;
		if (hlen < newlen) *h = (double *)malloc(newlen * sizeof(double));
		return Gen_Scale(elen, e, b, *h);
	}

	int Gen_Sum_With_PreAlloc(const int elen, const double *e, const int flen, const double *f, double **h, const int hlen)
	{
		int newlen = elen + flen;
		if (hlen < newlen) *h = (double *)malloc(newlen * sizeof(double));
		return Gen_Sum(elen, e, flen, f, *h);
	}

	int Gen_Product_With_PreAlloc(const int alen, const double *a, const int blen, const double *b, double **h, const int hlen)
	{
		int h_len = alen * blen * 2;
		if (h_len < 8) h_len = 8;
		if (hlen < h_len) *h = (double *)malloc(h_len * sizeof(double));
		return Gen_Product(alen, a, blen, b, *h);
	}

	// Approximates the expansion to a double
	static double To_Double(const int elen, const double *e);
};

#include <cassert>

int expansionObject::Gen_Sum(const int elen, const double *e, const int flen, const double *f, double *h)
{
	double Q, Qn, hh, en = e[0], fn = f[0];
	int e_k, f_k, h_k;

	h_k = e_k = f_k = 0;
	if ((fn > en) == (fn > -en)) { assert(e_k+1 < elen); Q = en; en = e[++e_k]; }
	else { assert(f_k + 1 < flen); Q = fn; fn = f[++f_k]; }

	if ((e_k < elen) && (f_k < flen))
	{
		if ((fn > en) == (fn > -en)) { Quick_Two_Sum(en, Q, Qn, hh); assert(e_k + 1 < elen); en = e[++e_k]; } else { Quick_Two_Sum(fn, Q, Qn, hh); assert(f_k + 1 < flen); fn = f[++f_k]; }
		Q = Qn;
		if (hh != 0.0) h[h_k++] = hh;
		while ((e_k < elen) && (f_k < flen))
		{
			if ((fn > en) == (fn > -en)) { Two_Sum(Q, en, Qn, hh); assert(e_k + 1 < elen); en = e[++e_k]; } else { Two_Sum(Q, fn, Qn, hh); assert(f_k + 1 < flen); fn = f[++f_k]; }
			Q = Qn;
			if (hh != 0.0) h[h_k++] = hh;
		}
	}

	while (e_k < elen)
	{
		assert(e_k + 1 < elen);
		Two_Sum(Q, en, Qn, hh);
		en = e[++e_k];
		Q = Qn;
		if (hh != 0.0) h[h_k++] = hh;
	}
	while (f_k < flen)
	{
		assert(f_k + 1 < flen);
		Two_Sum(Q, fn, Qn, hh);
		fn = f[++f_k];
		Q = Qn;
		if (hh != 0.0) h[h_k++] = hh;
	}
	if ((Q != 0.0) || (h_k == 0)) h[h_k++] = Q;

	return h_k;
}

int expansionObject::Gen_Scale(const int elen, const double *e, const double& b, double *h)
{
	double Q, sum, hh, pr1, pr0, enow;
	int e_k, h_k;

	Split(b, _bh, _bl);
	Two_Prod_PreSplit(e[0], b, _bh, _bl, Q, hh);
	h_k = 0;
	if (hh != 0) h[h_k++] = hh;

	for (e_k = 1; e_k < elen; e_k++)
	{
		enow = e[e_k];
		Two_Prod_PreSplit(enow, b, _bh, _bl, pr1, pr0);
		Two_Sum(Q, pr0, sum, hh);
		if (hh != 0) h[h_k++] = hh;
		Quick_Two_Sum(pr1, sum, Q, hh);
		if (hh != 0) h[h_k++] = hh;
	}
	if ((Q != 0.0) || (h_k == 0)) h[h_k++] = Q;

	return h_k;
}


void expansionObject::Two_Square(const double& a1, const double& a0, double *x)
{
	Square(a0, _j, x[0]);
	_0 = a0 + a0;
	Two_Prod(a1, _0, _k, _1);
	Two_One_Sum(_k, _1, _j, _l, _2, x[1]);
	Square(a1, _j, _1);
	Two_Two_Sum(_j, _1, _l, _2, x[5], x[4], x[3], x[2]);
}


void expansionObject::Two_Two_Prod(const double a1, const double a0, const double b1, const double b0, double *h)
{
	Split(a0, _ah, _al);
	Split(b0, _bh, _bl);
	Two_Product_2Presplit(a0, _ah, _al, b0, _bh, _bl, _i, h[0]);
	Split(a1, _ch, _cl);
	Two_Product_2Presplit(a1, _ch, _cl, b0, _bh, _bl, _j, _0);
	Two_Sum(_i, _0, _k, _1);
	Quick_Two_Sum(_j, _k, _l, _2);
	Split(b1, _bh, _bl);
	Two_Product_2Presplit(a0, _ah, _al, b1, _bh, _bl, _i, _0);
	Two_Sum(_1, _0, _k, h[1]);
	Two_Sum(_2, _k, _j, _1);
	Two_Sum(_l, _j, _m, _2);
	Two_Product_2Presplit(a1, _ch, _cl, b1, _bh, _bl, _j, _0);
	Two_Sum(_i, _0, _n, _0);
	Two_Sum(_1, _0, _i, h[2]);
	Two_Sum(_2, _i, _k, _1);
	Two_Sum(_m, _k, _l, _2);
	Two_Sum(_j, _n, _k, _0);
	Two_Sum(_1, _0, _j, h[3]);
	Two_Sum(_2, _j, _i, _1);
	Two_Sum(_l, _i, _m, _2);
	Two_Sum(_1, _k, _i, h[4]);
	Two_Sum(_2, _i, _k, h[5]);
	Two_Sum(_m, _k, h[7], h[6]);
}


int expansionObject::Sub_product(const int alen, const double *a, const int blen, const double *b, double *h)
{
	if (alen == 1) return Gen_Scale(blen, b, a[0], h);
	else
	{
		const double* a1 = a;
		int a1len = alen / 2;
		const double* a2 = a1 + a1len;
		int a2len = alen - a1len;

		int a1blen, a2blen;
		double *a1b = (double *)malloc(2 * a1len * blen * sizeof(double));
		double *a2b = (double *)malloc(2 * a2len * blen * sizeof(double));
		a1blen = Sub_product(a1len, a1, blen, b, a1b);
		a2blen = Sub_product(a2len, a2, blen, b, a2b);
		int hlen = Gen_Sum(a1blen, a1b, a2blen, a2b, h);
		free(a1b);
		free(a2b);
		return hlen;
	}
}


int expansionObject::Gen_Product(const int alen, const double *a, const int blen, const double *b, double *h)
{
	if (alen == 1)
	{
		if (blen == 1) { Two_Prod(a[0], b[0], h); return 2; } else if (blen == 2) { Two_One_Prod(b, a[0], h); return 4; } else return Gen_Scale(blen, b, a[0], h);
	} else if (alen == 2)
	{
		if (blen == 1) { Two_One_Prod(a, b[0], h); return 4; } else if (blen == 2) { Two_Two_Prod(a[1], a[0], b[1], b[0], h); return 8; } else return Sub_product(alen, a, blen, b, h);
	} else
	{
		if (blen == 1) return Gen_Scale(alen, a, b[0], h);
		else if (alen < blen) return Sub_product(alen, a, blen, b, h);
		else return Sub_product(blen, b, alen, a, h);
	}
}


double expansionObject::To_Double(const int elen, const double *e)
{
	double Q = e[0];
	for (int e_i = 1; e_i < elen; e_i++) Q += e[e_i];
	return Q;
}

void initFPU()
{
#if _WIN64
	//do nothing
#else
#ifdef WIN32
	_control87(_PC_53, _MCW_PC); /* Set FPU control word for double precision. */
#else
	int cword;
	cword = 4722;                 /* set FPU control word for double precision */
	_FPU_SETCW(cword);
#endif
#endif
}


//////////////////////////////////////////////////////////////////////////////////
//
//   O R I E N T 3 D _ L P I
//
//////////////////////////////////////////////////////////////////////////////////

//int orient3D_LPI_monolithic_exact(
//	double px, double py, double pz,
//	double qx, double qy, double qz,
//	double rx, double ry, double rz,
//	double sx, double sy, double sz,
//	double tx, double ty, double tz,
//	double ax, double ay, double az,
//	double bx, double by, double bz,
//	double cx, double cy, double cz)
//{
//	double a11[2], a12[2], a13[2], a21[2], a22[2], a23[2], a31[2], a32[2], a33[2];
//	double d21[2], d22[2], d23[2], d31[2], d32[2], d33[2];
//	double px_rx[2], py_ry[2], pz_rz[2];
//	double px_cx[2], py_cy[2], pz_cz[2];
//	double t1[8], t2[8];
//	double a2233[16], a2133[16], a2132[16];
//	double tt1[64], tt2[64], tt3[64];
//	double ttt1[128];
//	double d[192], n[192];
//	double ii1[768], ii2[768];
//	double d11[1536], d12[1536], d13[1536];
//	double d2233[8], d2332[8], d2133[8], d2331[8], d2132[8], d2231[8];
//
//	double s1p[256], s2p[256], s3p[256];
//	double *s1 = s1p, *s2 = s2p, *s3 = s3p; // Bound is 49152 each
//	int s1l = 256, s2l = 256, s3l = 256;
//	double ss1p[256];
//	double *ss1 = ss1p; // Bound is 98304
//	int ss1l = 256;
//	double detp[256];
//	double *det = detp; // Bound is 147456
//	int detl = 256;
//	int a2233l, a2133l, a2132l, tt1l, tt2l, tt3l, ttt1l, dl, nl, ii1l, ii2l, d11l, d12l, d13l;
//
//	expansionObject o;
//
//	o.two_Diff(px, qx, a11[1], a11[0]); // a11 = px - qx;
//	o.two_Diff(py, qy, a12[1], a12[0]); // a12 = py - qy;
//	o.two_Diff(pz, qz, a13[1], a13[0]); // a13 = pz - qz;
//	o.two_Diff(sx, rx, a21[1], a21[0]); // a21 = sx - rx;
//	o.two_Diff(sy, ry, a22[1], a22[0]); // a22 = sy - ry;
//	o.two_Diff(sz, rz, a23[1], a23[0]); // a23 = sz - rz;
//	o.two_Diff(tx, rx, a31[1], a31[0]); // a31 = tx - rx;
//	o.two_Diff(ty, ry, a32[1], a32[0]); // a32 = ty - ry;
//	o.two_Diff(tz, rz, a33[1], a33[0]); // a33 = tz - rz;
//	o.two_Diff(px, rx, px_rx[1], px_rx[0]); // px_rx = px - rx;
//	o.two_Diff(py, ry, py_ry[1], py_ry[0]); // py_ry = py - ry;
//	o.two_Diff(pz, rz, pz_rz[1], pz_rz[0]); // pz_rz = pz - rz;
//
//	o.Two_Two_Prod(a22[1], a22[0], a33[1], a33[0], t1); // t1 = a22 * a33;
//	o.Two_Two_Prod(a23[1], a23[0], a32[1], a32[0], t2); // t2 = a23 * a32;
//	o.Gen_Invert(8, t2);								// t2 = -t2;
//	a2233l = o.Gen_Sum(8, t1, 8, t2, a2233);			// a2233 = t1 + t2; // = a22*a33 - a23*a32;
//
//	o.Two_Two_Prod(a21[1], a21[0], a33[1], a33[0], t1); // t1 = a21 * a33;
//	o.Two_Two_Prod(a23[1], a23[0], a31[1], a31[0], t2); // t2 = a23 * a31;
//	o.Gen_Invert(8, t2);								// t2 = -t2;
//	a2133l = o.Gen_Sum(8, t1, 8, t2, a2133);			// a2133 = t1 + t2; // = a21*a33 - a23*a31;
//
//	o.Two_Two_Prod(a21[1], a21[0], a32[1], a32[0], t1); // t1 = a21 * a32;
//	o.Two_Two_Prod(a22[1], a22[0], a31[1], a31[0], t2); // t2 = a22 * a31;
//	o.Gen_Invert(8, t2);								// t2 = -t2;
//	a2132l = o.Gen_Sum(8, t1, 8, t2, a2132);			// a2132 = t1 + t2; // = a21*a32 - a22*a31;
//
//	tt1l = o.Gen_Product(a2233l, a2233, 2, a11, tt1); // tt1 = a2233 * a11;
//	tt2l = o.Gen_Product(a2133l, a2133, 2, a12, tt2); // tt2 = a2133 * a12;
//	tt3l = o.Gen_Product(a2132l, a2132, 2, a13, tt3); // tt3 = a2132 * a13;
//	o.Gen_Invert(tt2l, tt2);							  // tt2 = -tt2;
//	ttt1l = o.Gen_Sum(tt1l, tt1, tt2l, tt2, ttt1);	  // ttt1 = tt1 + tt2;
//	dl = o.Gen_Sum(ttt1l, ttt1, tt3l, tt3, d);		  // d = ttt1 + tt3; // = tt1 + tt2 + tt3; // = a2233 * a11 - a2133 * a12 + a2132 * a13;
//
//	tt1l = o.Gen_Product(a2133l, a2133, 2, py_ry, tt1); // tt1 = a2133 * py_ry;
//	tt2l = o.Gen_Product(a2233l, a2233, 2, px_rx, tt2); // tt2 = a2233 * px_rx;
//	tt3l = o.Gen_Product(a2132l, a2132, 2, pz_rz, tt3); // tt3 = a2132 * pz_rz;
//	o.Gen_Invert(tt2l, tt2);							// tt2 = -tt2;
//	ttt1l = o.Gen_Sum(tt1l, tt1, tt2l, tt2, ttt1);	    // ttt1 = tt1 + tt2;
//	o.Gen_Invert(tt3l, tt3);							// tt3 = -tt3;
//	nl = o.Gen_Sum(ttt1l, ttt1, tt3l, tt3, n);		// n = ttt1 + tt3; // = tt1 + tt2 + tt3; // = a2133 * py_ry - a2233 * px_rx - a2132 * pz_rz;
//
//	o.two_Diff(px, cx, px_cx[1], px_cx[0]); //px_cx = px - cx;
//	o.two_Diff(py, cy, py_cy[1], py_cy[0]); //py_cy = py - cy;
//	o.two_Diff(pz, cz, pz_cz[1], pz_cz[0]); //pz_cz = pz - cz;
//
//	ii1l = o.Gen_Product(dl, d, 2, px_cx, ii1);
//	ii2l = o.Gen_Product(nl, n, 2, a11, ii2);
//	d11l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d11); //d11 = (d * px_cx) + (a11 * n);
//
//	ii1l = o.Gen_Product(dl, d, 2, py_cy, ii1);
//	ii2l = o.Gen_Product(nl, n, 2, a12, ii2);
//	d12l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d12); //d12 = (d * py_cy) + (a12 * n);
//
//	ii1l = o.Gen_Product(dl, d, 2, pz_cz, ii1);
//	ii2l = o.Gen_Product(nl, n, 2, a13, ii2);
//	d13l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d13); //d13 = (d * pz_cz) + (a13 * n);
//
//	o.two_Diff(ax, cx, d21[1], d21[0]); //d21 = (ax - cx);
//	o.two_Diff(bx, cx, d31[1], d31[0]); //d31 = (bx - cx);
//	o.two_Diff(ay, cy, d22[1], d22[0]); //d22 = (ay - cy);
//	o.two_Diff(by, cy, d32[1], d32[0]); //d32 = (by - cy);
//	o.two_Diff(az, cz, d23[1], d23[0]); //d23 = (az - cz);
//	o.two_Diff(bz, cz, d33[1], d33[0]); //d33 = (bz - cz);
//
//	o.Two_Two_Prod(d22[1], d22[0], d33[1], d33[0], d2233); //d2233 = d22*d33;
//	o.Two_Two_Prod(-d23[1], -d23[0], d32[1], d32[0], d2332); //d2332 = -d23*d32;
//	o.Two_Two_Prod(d21[1], d21[0], d33[1], d33[0], d2133); //d2133 = d21*d33;
//	o.Two_Two_Prod(-d23[1], -d23[0], d31[1], d31[0], d2331); //d2331 = -d23*d31;
//	o.Two_Two_Prod(d21[1], d21[0], d32[1], d32[0], d2132); //d2132 = d21*d32;
//	o.Two_Two_Prod(-d22[1], -d22[0], d31[1], d31[0], d2231); //d2231 = -d22*d31;
//
//	a2233l = o.Gen_Sum(8, d2233, 8, d2332, a2233); // a2233 = d2233 + d2332;
//	a2133l = o.Gen_Sum(8, d2133, 8, d2331, a2133); // a2133 = d2133 + d2331;
//	a2132l = o.Gen_Sum(8, d2132, 8, d2231, a2132); // a2132 = d2132 + d2231;
//
//	s1l = o.Gen_Product_With_PreAlloc(d11l, d11, a2233l, a2233, &s1, s1l); // s1 = d11 * a2233;
//	s2l = o.Gen_Product_With_PreAlloc(d12l, d12, a2133l, a2133, &s2, s2l); // s2 = d12 * a2133;
//	s3l = o.Gen_Product_With_PreAlloc(d13l, d13, a2132l, a2132, &s3, s3l); // s3 = d13 * a2132;
//
//	o.Gen_Invert(s2l, s2);
//	ss1l = o.Gen_Sum_With_PreAlloc(s1l, s1, s2l, s2, &ss1, ss1l); // ss1 = s1 - s2;
//	detl = o.Gen_Sum_With_PreAlloc(ss1l, ss1, s3l, s3, &det, detl); // det = ss1 + s3; // = s1 - s2 + s3;
//
//	if (s1 != s1p) free(s1); 
//	if (s2 != s2p) free(s2);
//	if (s3 != s3p) free(s3);
//	if (ss1 != ss1p) free(ss1);
//
//	double s = det[detl - 1];
//	double sd = d[dl - 1];
//
//	if (det != detp) free(det);
//
//	if ((s > 0)) return (sd>0) ? (1) : (-1);
//	if ((s < 0)) return (sd>0) ? (-1) : (1);
//	return 0;
//}
//


bool orient3D_LPI_pre_exact(
	double px, double py, double pz,
	double qx, double qy, double qz,
	double rx, double ry, double rz,
	double sx, double sy, double sz,
	double tx, double ty, double tz,
	double *a11, double *a12, double *a13,
	double *d, double *n,
	int& dl, int& nl
	)
{
	
	double a21[2], a22[2], a23[2], a31[2], a32[2], a33[2];
	double px_rx[2], py_ry[2], pz_rz[2];
	double t1[8], t2[8];
	double a2233[16], a2133[16], a2132[16];
	double tt1[64], tt2[64], tt3[64];
	double ttt1[128];
	int a2233l, a2133l, a2132l, tt1l, tt2l, tt3l, ttt1l;

	expansionObject o;

	o.two_Diff(px, qx, a11[1], a11[0]); // a11 = px - qx;
	o.two_Diff(py, qy, a12[1], a12[0]); // a12 = py - qy;
	o.two_Diff(pz, qz, a13[1], a13[0]); // a13 = pz - qz;
	o.two_Diff(sx, rx, a21[1], a21[0]); // a21 = sx - rx;
	o.two_Diff(sy, ry, a22[1], a22[0]); // a22 = sy - ry;
	o.two_Diff(sz, rz, a23[1], a23[0]); // a23 = sz - rz;
	o.two_Diff(tx, rx, a31[1], a31[0]); // a31 = tx - rx;
	o.two_Diff(ty, ry, a32[1], a32[0]); // a32 = ty - ry;
	o.two_Diff(tz, rz, a33[1], a33[0]); // a33 = tz - rz;
	o.two_Diff(px, rx, px_rx[1], px_rx[0]); // px_rx = px - rx;
	o.two_Diff(py, ry, py_ry[1], py_ry[0]); // py_ry = py - ry;
	o.two_Diff(pz, rz, pz_rz[1], pz_rz[0]); // pz_rz = pz - rz;

	o.Two_Two_Prod(a22[1], a22[0], a33[1], a33[0], t1); // t1 = a22 * a33;
	o.Two_Two_Prod(a23[1], a23[0], a32[1], a32[0], t2); // t2 = a23 * a32;
	o.Gen_Invert(8, t2);								// t2 = -t2;
	a2233l = o.Gen_Sum(8, t1, 8, t2, a2233);			// a2233 = t1 + t2; // = a22*a33 - a23*a32;

	o.Two_Two_Prod(a21[1], a21[0], a33[1], a33[0], t1); // t1 = a21 * a33;
	o.Two_Two_Prod(a23[1], a23[0], a31[1], a31[0], t2); // t2 = a23 * a31;
	o.Gen_Invert(8, t2);								// t2 = -t2;
	a2133l = o.Gen_Sum(8, t1, 8, t2, a2133);			// a2133 = t1 + t2; // = a21*a33 - a23*a31;

	o.Two_Two_Prod(a21[1], a21[0], a32[1], a32[0], t1); // t1 = a21 * a32;
	o.Two_Two_Prod(a22[1], a22[0], a31[1], a31[0], t2); // t2 = a22 * a31;
	o.Gen_Invert(8, t2);								// t2 = -t2;
	a2132l = o.Gen_Sum(8, t1, 8, t2, a2132);			// a2132 = t1 + t2; // = a21*a32 - a22*a31;

	tt1l = o.Gen_Product(a2233l, a2233, 2, a11, tt1); // tt1 = a2233 * a11;
	tt2l = o.Gen_Product(a2133l, a2133, 2, a12, tt2); // tt2 = a2133 * a12;
	tt3l = o.Gen_Product(a2132l, a2132, 2, a13, tt3); // tt3 = a2132 * a13;
	o.Gen_Invert(tt2l, tt2);							  // tt2 = -tt2;
	ttt1l = o.Gen_Sum(tt1l, tt1, tt2l, tt2, ttt1);	  // ttt1 = tt1 + tt2;
	dl = o.Gen_Sum(ttt1l, ttt1, tt3l, tt3, d);		  // d = ttt1 + tt3; // = tt1 + tt2 + tt3; // = a2233 * a11 - a2133 * a12 + a2132 * a13;

	if (dl == 0) return false;

	tt1l = o.Gen_Product(a2133l, a2133, 2, py_ry, tt1); // tt1 = a2133 * py_ry;
	tt2l = o.Gen_Product(a2233l, a2233, 2, px_rx, tt2); // tt2 = a2233 * px_rx;
	tt3l = o.Gen_Product(a2132l, a2132, 2, pz_rz, tt3); // tt3 = a2132 * pz_rz;
	o.Gen_Invert(tt2l, tt2);							// tt2 = -tt2;
	ttt1l = o.Gen_Sum(tt1l, tt1, tt2l, tt2, ttt1);	    // ttt1 = tt1 + tt2;
	o.Gen_Invert(tt3l, tt3);							// tt3 = -tt3;
	nl = o.Gen_Sum(ttt1l, ttt1, tt3l, tt3, n);		// n = ttt1 + tt3; // = tt1 + tt2 + tt3; // = a2133 * py_ry - a2233 * px_rx - a2132 * pz_rz;

	if (nl == 0) return false;

	return true;
}

bool orient3D_LPI_pre_exact(
	double px, double py, double pz,
	double qx, double qy, double qz,
	double rx, double ry, double rz,
	double sx, double sy, double sz,
	double tx, double ty, double tz,
	LPI_exact_suppvars& s
	)
{
	lpree++;
	return orient3D_LPI_pre_exact(px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz, tx, ty, tz, s.a11, s.a12, s.a13, s.d, s.n, s.dl, s.nl);
}

int orient3D_LPI_post_exact(
	double *a11, double *a12, double *a13,
	double *d, double *n,
	int dl, int nl,
	double px, double py, double pz,
	double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	double px_cx[2], py_cy[2], pz_cz[2];
	double d21[2], d22[2], d23[2], d31[2], d32[2], d33[2];
	double a2233[16], a2133[16], a2132[16];

	double ii1[768], ii2[768];
	double d11[1536], d12[1536], d13[1536];
	double d2233[8], d2332[8], d2133[8], d2331[8], d2132[8], d2231[8];

	double s1p[256], s2p[256], s3p[256];
	double *s1 = s1p, *s2 = s2p, *s3 = s3p; // Bound is 49152 each
	int s1l = 256, s2l = 256, s3l = 256;
	double ss1p[256];
	double *ss1 = ss1p; // Bound is 98304
	int ss1l = 256;
	double detp[256];
	double *det = detp; // Bound is 147456
	int detl = 256;
	int a2233l, a2133l, a2132l, ii1l, ii2l, d11l, d12l, d13l;

	expansionObject o;

	o.two_Diff(px, cx, px_cx[1], px_cx[0]); //px_cx = px - cx;
	o.two_Diff(py, cy, py_cy[1], py_cy[0]); //py_cy = py - cy;
	o.two_Diff(pz, cz, pz_cz[1], pz_cz[0]); //pz_cz = pz - cz;

	ii1l = o.Gen_Product(dl, d, 2, px_cx, ii1);
	ii2l = o.Gen_Product(nl, n, 2, a11, ii2);
	d11l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d11); //d11 = (d * px_cx) + (a11 * n);

	ii1l = o.Gen_Product(dl, d, 2, py_cy, ii1);
	ii2l = o.Gen_Product(nl, n, 2, a12, ii2);
	d12l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d12); //d12 = (d * py_cy) + (a12 * n);

	ii1l = o.Gen_Product(dl, d, 2, pz_cz, ii1);
	ii2l = o.Gen_Product(nl, n, 2, a13, ii2);
	d13l = o.Gen_Sum(ii1l, ii1, ii2l, ii2, d13); //d13 = (d * pz_cz) + (a13 * n);

	o.two_Diff(ax, cx, d21[1], d21[0]); //d21 = (ax - cx);
	o.two_Diff(bx, cx, d31[1], d31[0]); //d31 = (bx - cx);
	o.two_Diff(ay, cy, d22[1], d22[0]); //d22 = (ay - cy);
	o.two_Diff(by, cy, d32[1], d32[0]); //d32 = (by - cy);
	o.two_Diff(az, cz, d23[1], d23[0]); //d23 = (az - cz);
	o.two_Diff(bz, cz, d33[1], d33[0]); //d33 = (bz - cz);

	o.Two_Two_Prod(d22[1], d22[0], d33[1], d33[0], d2233); //d2233 = d22*d33;
	o.Two_Two_Prod(-d23[1], -d23[0], d32[1], d32[0], d2332); //d2332 = -d23*d32;
	o.Two_Two_Prod(d21[1], d21[0], d33[1], d33[0], d2133); //d2133 = d21*d33;
	o.Two_Two_Prod(-d23[1], -d23[0], d31[1], d31[0], d2331); //d2331 = -d23*d31;
	o.Two_Two_Prod(d21[1], d21[0], d32[1], d32[0], d2132); //d2132 = d21*d32;
	o.Two_Two_Prod(-d22[1], -d22[0], d31[1], d31[0], d2231); //d2231 = -d22*d31;

	a2233l = o.Gen_Sum(8, d2233, 8, d2332, a2233); // a2233 = d2233 + d2332;
	a2133l = o.Gen_Sum(8, d2133, 8, d2331, a2133); // a2133 = d2133 + d2331;
	a2132l = o.Gen_Sum(8, d2132, 8, d2231, a2132); // a2132 = d2132 + d2231;

	s1l = o.Gen_Product_With_PreAlloc(d11l, d11, a2233l, a2233, &s1, s1l); // s1 = d11 * a2233;
	s2l = o.Gen_Product_With_PreAlloc(d12l, d12, a2133l, a2133, &s2, s2l); // s2 = d12 * a2133;
	s3l = o.Gen_Product_With_PreAlloc(d13l, d13, a2132l, a2132, &s3, s3l); // s3 = d13 * a2132;

	o.Gen_Invert(s2l, s2);
	ss1l = o.Gen_Sum_With_PreAlloc(s1l, s1, s2l, s2, &ss1, ss1l); // ss1 = s1 - s2;
	detl = o.Gen_Sum_With_PreAlloc(ss1l, ss1, s3l, s3, &det, detl); // det = ss1 + s3; // = s1 - s2 + s3;

	if (s1 != s1p) free(s1);
	if (s2 != s2p) free(s2);
	if (s3 != s3p) free(s3);
	if (ss1 != ss1p) free(ss1);

	double s = det[detl - 1];
	double sd = d[dl - 1];

	if (det != detp) free(det);

	if ((s > 0)) return (sd>0) ? (1) : (-1);
	if ((s < 0)) return (sd>0) ? (-1) : (1);
	return 0;
}

int orient3D_LPI_post_exact(
	LPI_exact_suppvars& s,
	double px, double py, double pz,
	double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	lposte++;
	return orient3D_LPI_post_exact(s.a11, s.a12, s.a13, s.d, s.n, s.dl, s.nl, px, py, pz, ax, ay, az, bx, by, bz, cx, cy, cz);
}

int orient3D_LPI_exact(
	double px, double py, double pz,
	double qx, double qy, double qz,
	double rx, double ry, double rz,
	double sx, double sy, double sz,
	double tx, double ty, double tz,
	double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	LPI_exact_suppvars s;

	if (orient3D_LPI_pre_exact(px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz, tx, ty, tz, s))
		return orient3D_LPI_post_exact(s, px, py, pz, ax, ay, az, bx, by, bz, cx, cy, cz);
	return 0;
}



//////////////////////////////////////////////////////////////////////////////////
//
//   O R I E N T 3 D _ T P I
//
//////////////////////////////////////////////////////////////////////////////////

inline void o3dTPI_tf1(expansionObject& o, double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double *nvx, double *nvy, double *nvz, int& nvxl, int& nvyl, int& nvzl)
{
	double v32x[2], v32y[2], v32z[2], v21x[2], v21y[2], v21z[2]; // 2
	double tp1[8], tp2[8]; // 8

	o.two_Diff(v3x, v2x, v32x[1], v32x[0]); // v32x = v3x - v2x;
	o.two_Diff(v3y, v2y, v32y[1], v32y[0]); // v32y = v3y - v2y;
	o.two_Diff(v3z, v2z, v32z[1], v32z[0]); // v32z = v3z - v2z;
	o.two_Diff(v2x, v1x, v21x[1], v21x[0]); // v21x = v2x - v1x;
	o.two_Diff(v2y, v1y, v21y[1], v21y[0]); // v21y = v2y - v1y;
	o.two_Diff(v2z, v1z, v21z[1], v21z[0]); // v21z = v2z - v1z;

	o.Two_Two_Prod(v21y[1], v21y[0], v32z[1], v32z[0], tp1); // tp1 = v21y*v32z;
	o.Two_Two_Prod(v21z[1], v21z[0], v32y[1], v32y[0], tp2); // tp2 = v21z*v32y;
	o.Gen_Invert(8, tp2);
	nvxl = o.Gen_Sum(8, tp1, 8, tp2, nvx);

	o.Two_Two_Prod(v32x[1], v32x[0], v21z[1], v21z[0], tp1); // tp1 = v32x*v21z;
	o.Two_Two_Prod(v32z[1], v32z[0], v21x[1], v21x[0], tp2); // tp2 = v32z*v21x;
	o.Gen_Invert(8, tp2);
	nvyl = o.Gen_Sum(8, tp1, 8, tp2, nvy);

	o.Two_Two_Prod(v21x[1], v21x[0], v32y[1], v32y[0], tp1); // tp1 = v21x*v32y;
	o.Two_Two_Prod(v21y[1], v21y[0], v32x[1], v32x[0], tp2); // tp2 = v21y*v32x;
	o.Gen_Invert(8, tp2);
	nvzl = o.Gen_Sum(8, tp1, 8, tp2, nvz);
}

inline void o3dTPI_tf2(expansionObject& o, double *nwx, double *nwy, double *nwz, int nwxl, int nwyl, int nwzl, double *nux, double *nuy, double *nuz, int nuxl, int nuyl, int nuzl,
	double *nwyuz, double *nwxuz, double *nwxuy, int& nwyuzl, int& nwxuzl, int& nwxuyl)
{
	double tq1[64], tq2[64]; // 64
	int tq1l, tq2l;

	tq1l = o.Gen_Product(nwyl, nwy, nuzl, nuz, tq1); // tq1 = nwy*nuz;
	tq2l = o.Gen_Product(nwzl, nwz, nuyl, nuy, tq2); // tq2 = nwz*nuy;
	o.Gen_Invert(tq2l, tq2);
	nwyuzl = o.Gen_Sum(tq1l, tq1, tq2l, tq2, nwyuz); // nwyuz = tq1 - tq2;

	tq1l = o.Gen_Product(nwxl, nwx, nuzl, nuz, tq1); // tq1 = nwx*nuz;
	tq2l = o.Gen_Product(nwzl, nwz, nuxl, nux, tq2); // tq2 = nwz*nux;
	o.Gen_Invert(tq2l, tq2);
	nwxuzl = o.Gen_Sum(tq1l, tq1, tq2l, tq2, nwxuz); // nwxuz = tq1 - tq2;

	tq1l = o.Gen_Product(nwxl, nwx, nuyl, nuy, tq1); // tq1 = nwx*nuy;
	tq2l = o.Gen_Product(nwyl, nwy, nuxl, nux, tq2); // tq2 = nwy*nux;
	o.Gen_Invert(tq2l, tq2);
	nwxuyl = o.Gen_Sum(tq1l, tq1, tq2l, tq2, nwxuy); // nwxuy = tq1 - tq2;
}

// g = a*b + c*d - e*f
inline void o3dTPI_tf3(expansionObject& o, double *a, double *b, double *c, double *d, double *e, double *f,
	int al, int bl, int cl, int dl, int el, int fl,
	double **g, int& gl)
{
	int s1 = al * bl;
	int s2 = el * fl;
	int s3 = cl * dl;
	if (s2 > s1) s1 = s2;
	int ss = s2 * s3;
	int tr1l, tr2l, tr3l, tsl;

	double tr1p[256], tr2p[256], tr3p[256], tsp[256];
	double *tr1 = tr1p, *tr2 = tr2p, *tr3 = tr3p, *ts = tsp;

	tr1l = o.Gen_Product_With_PreAlloc(al, a, bl, b, &tr1, 256); // tr1 = a*b;
	tr2l = o.Gen_Product_With_PreAlloc(cl, c, dl, d, &tr2, 256); // tr2 = c*d;
	tsl = o.Gen_Sum_With_PreAlloc(tr1l, tr1, tr2l, tr2, &ts, 256); // ts = tr1 + tr2;
	tr3l = o.Gen_Product_With_PreAlloc(el, e, fl, f, &tr3, 256); // tr3 = e*f;
	o.Gen_Invert(tr3l, tr3);
	gl = o.Gen_Sum_With_PreAlloc(tsl, ts, tr3l, tr3, g, gl); // g = ts + tr3;

	if (tr1 != tr1p) free(tr1);
	if (tr2 != tr2p) free(tr2);
	if (tr3 != tr3p) free(tr3);
	if (ts != tsp) free(ts);
}

inline void o3dTPI_tf3s(expansionObject& o, double *a, double b, double *c, double d, double *e, double f, 
	int al, int cl, int el,
	double *g, int& gl)
{
	double tr1[32], tr2[32]; // 32
	double ts[64]; // 64
	int tr1l, tr2l, tsl;

	tr1l = o.Gen_Scale(al, a, b, tr1); // tr1 = a*b;
	tr2l = o.Gen_Scale(cl, c, d, tr2); // tr2 = c*d;
	tsl = o.Gen_Sum(tr1l, tr1, tr2l, tr2, ts); // ts = tr1 + tr2;
	tr1l = o.Gen_Scale(el, e, f, tr1); // tr1 = e*f;
	gl = o.Gen_Sum(tsl, ts, tr1l, tr1, g); // g = ts + tr1;
}

//int orient3D_TPI_monolithic_exact(
//	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
//	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
//	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
//	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
//	)
//{
//	double a21[2], a22[2], a23[2], a31[2], a32[2], a33[2]; // 2
//	double a2233[8], a2332[8], a2133[8], a2331[8], a2132[8], a2231[8]; // 8
//	double dd1[16], dd2[16], dd3[16], nvx[16], nvy[16], nvz[16], nwx[16], nwy[16], nwz[16], nux[16], nuy[16], nuz[16]; // 16
//	int dd1l, dd2l, dd3l, nvxl, nvyl, nvzl, nwxl, nwyl, nwzl, nuxl, nuyl, nuzl;
//	double p1[96], p2[96], p3[96]; // 96
//	int p1l, p2l, p3l;
//	double nwyuz[128], nwxuz[128], nwxuy[128], nvyuz[128], nvxuz[128], nvxuy[128], nvywz[128], nvxwz[128], nvxwy[128]; // 128
//	int nwyuzl, nwxuzl, nwxuyl, nvyuzl, nvxuzl, nvxuyl, nvywzl, nvxwzl, nvxwyl;
//
//	double dp[256];
//	double *d = dp; // 12288
//	int dl = 256;
//	double dq3xp[256], dq3yp[256], dq3zp[256];
//	double *dq3x = dq3xp, *dq3y = dq3yp, *dq3z = dq3zp; // 24576
//	int dq3xl = 256, dq3yl = 256, dq3zl = 256;
//	double n1p[256], n2p[256], n3p[256];
//	double *n1 = n1p, *n2 = n2p, *n3 = n3p; // 73728
//	int n1l = 256, n2l = 256, n3l = 256;
//	double a11p[256], a12p[256], a13p[256];
//	double *a11 = a11p, *a12 = a12p, *a13 = a13p; // 98304
//	int a11l = 256, a12l = 256, a13l = 256;
//	double ee1p[256], ee2p[256];
//	double *ee1 = ee1p, *ee2 = ee2p; // 3145728
//	int ee1l = 256, ee2l = 256;
//	double ffp[256];
//	double *ff = ffp; // 6291456
//	int ffl = 256;
//	double detp[256];
//	double *det = detp; // 9437184
//	int detl = 256;
//
//	expansionObject o;
//
//	o3dTPI_tf1(o, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, nvx, nvy, nvz, nvxl, nvyl, nvzl);
//	o3dTPI_tf1(o, w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z, nwx, nwy, nwz, nwxl, nwyl, nwzl);
//	o3dTPI_tf1(o, u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z, nux, nuy, nuz, nuxl, nuyl, nuzl);
//
//	o3dTPI_tf2(o, nwx, nwy, nwz, nwxl, nwyl, nwzl, nux, nuy, nuz, nuxl, nuyl, nuzl, nwyuz, nwxuz, nwxuy, nwyuzl, nwxuzl, nwxuyl);
//	o3dTPI_tf2(o, nvx, nvy, nvz, nvxl, nvyl, nvzl, nux, nuy, nuz, nuxl, nuyl, nuzl, nvyuz, nvxuz, nvxuy, nvyuzl, nvxuzl, nvxuyl);
//	o3dTPI_tf2(o, nvx, nvy, nvz, nvxl, nvyl, nvzl, nwx, nwy, nwz, nwxl, nwyl, nwzl, nvywz, nvxwz, nvxwy, nvywzl, nvxwzl, nvxwyl);
//
//	o3dTPI_tf3s(o, nvx, v1x, nvy, v1y, nvz, v1z, nvxl, nvyl, nvzl, p1, p1l);
//	o3dTPI_tf3s(o, nwx, w1x, nwy, w1y, nwz, w1z, nwxl, nwyl, nwzl, p2, p2l);
//	o3dTPI_tf3s(o, nux, u1x, nuy, u1y, nuz, u1z, nuxl, nuyl, nuzl, p3, p3l);
//
//	o3dTPI_tf3(o, p1, nwyuz, p3, nvywz, p2, nvyuz, p1l, nwyuzl, p3l, nvywzl, p2l, nvyuzl, &n1, n1l); // n1 = p1*nwyuz - p2*nvyuz + p3*nvywz;
//	o3dTPI_tf3(o, p3, nvxwy, p1, nwxuy, p2, nvxuy, p3l, nvxwyl, p1l, nwxuyl, p2l, nvxuyl, &n3, n3l); // n3 = p3*nvxwy - p2*nvxuy + p1*nwxuy;
//	o.Gen_Invert(p3l, p3);
//	o3dTPI_tf3(o, p2, nvxuz, p3, nvxwz, p1, nwxuz, p2l, nvxuzl, p3l, nvxwzl, p1l, nwxuzl, &n2, n2l); // n2 = p2*nvxuz - p3*nvxwz - p1*nwxuz;
//
//	o3dTPI_tf3(o, nvx, nwyuz, nvz, nwxuy, nvy, nwxuz, nvxl, nwyuzl, nvzl, nwxuyl, nvyl, nwxuzl, &d, dl);
//
//	dq3xl = o.Gen_Scale_With_PreAlloc(dl, d, -q3x, &dq3x, dq3xl); // dq3x = -d * q3x;
//	dq3yl = o.Gen_Scale_With_PreAlloc(dl, d, -q3y, &dq3y, dq3yl); // dq3y = -d * q3y;
//	dq3zl = o.Gen_Scale_With_PreAlloc(dl, d, -q3z, &dq3z, dq3zl); // dq3z = -d * q3z;
//
//	a11l = o.Gen_Sum_With_PreAlloc(n1l, n1, dq3xl, dq3x, &a11, a11l); // a11 = n1 + dq3x;
//	a12l = o.Gen_Sum_With_PreAlloc(n2l, n2, dq3yl, dq3y, &a12, a12l); // a12 = n2 + dq3y;
//	a13l = o.Gen_Sum_With_PreAlloc(n3l, n3, dq3zl, dq3z, &a13, a13l); // a13 = n3 + dq3z;
//
//	if (dq3x != dq3xp) free(dq3x);
//	if (dq3y != dq3yp) free(dq3y);
//	if (dq3z != dq3zp) free(dq3z);
//	if (n1 != n1p) free(n1);
//	if (n2 != n2p) free(n2);
//	if (n3 != n3p) free(n3);
//
//	o.two_Diff(q1x, q3x, a21[1], a21[0]); // a21 = q1x - q3x;
//	o.two_Diff(q1y, q3y, a22[1], a22[0]); // a22 = q1y - q3y;
//	o.two_Diff(q1z, q3z, a23[1], a23[0]); // a23 = q1z - q3z;
//	o.two_Diff(q2x, q3x, a31[1], a31[0]); // a31 = q2x - q3x;
//	o.two_Diff(q2y, q3y, a32[1], a32[0]); // a32 = q2y - q3y;
//	o.two_Diff(q2z, q3z, a33[1], a33[0]); // a33 = q2z - q3z;
//
//	o.Two_Two_Prod(a22[1], a22[0], a33[1], a33[0], a2233); // a2233 = a22*a33;
//	o.Two_Two_Prod(a23[1], a23[0], a32[1], a32[0], a2332); // a2332 = a23*a32;
//	o.Two_Two_Prod(a21[1], a21[0], a33[1], a33[0], a2133); // a2133 = a21*a33;
//	o.Two_Two_Prod(a23[1], a23[0], a31[1], a31[0], a2331); // a2331 = a23*a31;
//	o.Two_Two_Prod(a21[1], a21[0], a32[1], a32[0], a2132); // a2132 = a21*a32;
//	o.Two_Two_Prod(a22[1], a22[0], a31[1], a31[0], a2231); // a2231 = a22*a31;
//
//	o.Gen_Invert(8, a2332);
//	o.Gen_Invert(8, a2331);
//	o.Gen_Invert(8, a2231);
//	dd1l = o.Gen_Sum(8, a2233, 8, a2332, dd1); // dd1 = a2233 + a2332;
//	dd2l = o.Gen_Sum(8, a2133, 8, a2331, dd2); // dd2 = a2133 + a2331;
//	dd3l = o.Gen_Sum(8, a2132, 8, a2231, dd3); // dd3 = a2132 + a2231;
//
//	ee1l = o.Gen_Product_With_PreAlloc(a11l, a11, dd1l, dd1, &ee1, ee1l); // ee1 = a11*dd1;
//	ee2l = o.Gen_Product_With_PreAlloc(a13l, a13, dd3l, dd3, &ee2, ee2l); // ee2 = a13*dd3;
//	ffl = o.Gen_Sum_With_PreAlloc(ee1l, ee1, ee2l, ee2, &ff, ffl); // ff = ee1 + ee2;
//	if (ee1 != ee1p) { free(ee1); ee1 = ee1p; }
//	if (ee2 != ee2p) free(ee2);
//	ee1l = o.Gen_Product_With_PreAlloc(a12l, a12, dd2l, dd2, &ee1, ee1l); // ee1 = a12*dd2;
//
//	if (a11 != a11p) free(a11); 
//	if (a12 != a12p) free(a12);
//	if (a13 != a13p) free(a13);
//
//	o.Gen_Invert(ee1l, ee1);
//	detl = o.Gen_Sum_With_PreAlloc(ffl, ff, ee1l, ee1, &det, detl); // det = ff + ee1;
//	if (ee1 != ee1p) free(ee1);
//	if (ff != ffp) free(ff);
//
//	double s = det[detl - 1];
//	double sd = d[dl - 1];
//
//	if (d != dp) free(d);
//	if (det != detp) free(det);
//
//	if ((s > 0)) return (sd>0) ? (1) : (-1);
//	if ((s < 0)) return (sd>0) ? (-1) : (1);
//	return 0;
//}



bool orient3D_TPI_pre_exact(
	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
	double **d, int& dl, double **n1, int& n1l, double **n2, int& n2l, double **n3, int& n3l
	)
{
	double nvx[16], nvy[16], nvz[16], nwx[16], nwy[16], nwz[16], nux[16], nuy[16], nuz[16]; // 16
	int nvxl, nvyl, nvzl, nwxl, nwyl, nwzl, nuxl, nuyl, nuzl;
	double p1[96], p2[96], p3[96]; // 96
	int p1l, p2l, p3l;
	double nwyuz[128], nwxuz[128], nwxuy[128], nvyuz[128], nvxuz[128], nvxuy[128], nvywz[128], nvxwz[128], nvxwy[128]; // 128
	int nwyuzl, nwxuzl, nwxuyl, nvyuzl, nvxuzl, nvxuyl, nvywzl, nvxwzl, nvxwyl;

	expansionObject o;

	o3dTPI_tf1(o, v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, nvx, nvy, nvz, nvxl, nvyl, nvzl);
	o3dTPI_tf1(o, w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z, nwx, nwy, nwz, nwxl, nwyl, nwzl);
	o3dTPI_tf1(o, u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z, nux, nuy, nuz, nuxl, nuyl, nuzl);

	o3dTPI_tf2(o, nwx, nwy, nwz, nwxl, nwyl, nwzl, nux, nuy, nuz, nuxl, nuyl, nuzl, nwyuz, nwxuz, nwxuy, nwyuzl, nwxuzl, nwxuyl);
	o3dTPI_tf2(o, nvx, nvy, nvz, nvxl, nvyl, nvzl, nux, nuy, nuz, nuxl, nuyl, nuzl, nvyuz, nvxuz, nvxuy, nvyuzl, nvxuzl, nvxuyl);
	o3dTPI_tf2(o, nvx, nvy, nvz, nvxl, nvyl, nvzl, nwx, nwy, nwz, nwxl, nwyl, nwzl, nvywz, nvxwz, nvxwy, nvywzl, nvxwzl, nvxwyl);

	o3dTPI_tf3s(o, nvx, v1x, nvy, v1y, nvz, v1z, nvxl, nvyl, nvzl, p1, p1l);
	o3dTPI_tf3s(o, nwx, w1x, nwy, w1y, nwz, w1z, nwxl, nwyl, nwzl, p2, p2l);
	o3dTPI_tf3s(o, nux, u1x, nuy, u1y, nuz, u1z, nuxl, nuyl, nuzl, p3, p3l);

	o3dTPI_tf3(o, p1, nwyuz, p3, nvywz, p2, nvyuz, p1l, nwyuzl, p3l, nvywzl, p2l, nvyuzl, n1, n1l); // n1 = p1*nwyuz - p2*nvyuz + p3*nvywz;
	o3dTPI_tf3(o, p3, nvxwy, p1, nwxuy, p2, nvxuy, p3l, nvxwyl, p1l, nwxuyl, p2l, nvxuyl, n3, n3l); // n3 = p3*nvxwy - p2*nvxuy + p1*nwxuy;
	o.Gen_Invert(p3l, p3);
	o3dTPI_tf3(o, p2, nvxuz, p3, nvxwz, p1, nwxuz, p2l, nvxuzl, p3l, nvxwzl, p1l, nwxuzl, n2, n2l); // n2 = p2*nvxuz - p3*nvxwz - p1*nwxuz;

	o3dTPI_tf3(o, nvx, nwyuz, nvz, nwxuy, nvy, nwxuz, nvxl, nwyuzl, nvzl, nwxuyl, nvyl, nwxuzl, d, dl);

	return (dl != 0);
}

bool orient3D_TPI_pre_exact(
	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
	TPI_exact_suppvars& s
	)
{
	tpree++;
	return orient3D_TPI_pre_exact(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z,
		u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z, &s.d, s.dl, &s.n1, s.n1l, &s.n2, s.n2l, &s.n3, s.n3l);
}

int orient3D_TPI_post_exact(
	double *d, int dl, double *n1, int n1l, double *n2, int n2l, double *n3, int n3l,
	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
	)
{
	double a21[2], a22[2], a23[2], a31[2], a32[2], a33[2]; // 2
	double a2233[8], a2332[8], a2133[8], a2331[8], a2132[8], a2231[8]; // 8
	double dd1[16], dd2[16], dd3[16]; // 16
	int dd1l, dd2l, dd3l;

	double dq3xp[256], dq3yp[256], dq3zp[256];
	double *dq3x = dq3xp, *dq3y = dq3yp, *dq3z = dq3zp; // 24576
	int dq3xl = 256, dq3yl = 256, dq3zl = 256;

	double a11p[256], a12p[256], a13p[256];
	double *a11 = a11p, *a12 = a12p, *a13 = a13p; // 98304
	int a11l = 256, a12l = 256, a13l = 256;
	double ee1p[256], ee2p[256];
	double *ee1 = ee1p, *ee2 = ee2p; // 3145728
	int ee1l = 256, ee2l = 256;
	double ffp[256];
	double *ff = ffp; // 6291456
	int ffl = 256;
	double detp[256];
	double *det = detp; // 9437184
	int detl = 256;

	expansionObject o;

	dq3xl = o.Gen_Scale_With_PreAlloc(dl, d, -q3x, &dq3x, dq3xl); // dq3x = -d * q3x;
	dq3yl = o.Gen_Scale_With_PreAlloc(dl, d, -q3y, &dq3y, dq3yl); // dq3y = -d * q3y;
	dq3zl = o.Gen_Scale_With_PreAlloc(dl, d, -q3z, &dq3z, dq3zl); // dq3z = -d * q3z;

	a11l = o.Gen_Sum_With_PreAlloc(n1l, n1, dq3xl, dq3x, &a11, a11l); // a11 = n1 + dq3x;
	a12l = o.Gen_Sum_With_PreAlloc(n2l, n2, dq3yl, dq3y, &a12, a12l); // a12 = n2 + dq3y;
	a13l = o.Gen_Sum_With_PreAlloc(n3l, n3, dq3zl, dq3z, &a13, a13l); // a13 = n3 + dq3z;

	if (dq3x != dq3xp) free(dq3x);
	if (dq3y != dq3yp) free(dq3y);
	if (dq3z != dq3zp) free(dq3z);

	o.two_Diff(q1x, q3x, a21[1], a21[0]); // a21 = q1x - q3x;
	o.two_Diff(q1y, q3y, a22[1], a22[0]); // a22 = q1y - q3y;
	o.two_Diff(q1z, q3z, a23[1], a23[0]); // a23 = q1z - q3z;
	o.two_Diff(q2x, q3x, a31[1], a31[0]); // a31 = q2x - q3x;
	o.two_Diff(q2y, q3y, a32[1], a32[0]); // a32 = q2y - q3y;
	o.two_Diff(q2z, q3z, a33[1], a33[0]); // a33 = q2z - q3z;

	o.Two_Two_Prod(a22[1], a22[0], a33[1], a33[0], a2233); // a2233 = a22*a33;
	o.Two_Two_Prod(a23[1], a23[0], a32[1], a32[0], a2332); // a2332 = a23*a32;
	o.Two_Two_Prod(a21[1], a21[0], a33[1], a33[0], a2133); // a2133 = a21*a33;
	o.Two_Two_Prod(a23[1], a23[0], a31[1], a31[0], a2331); // a2331 = a23*a31;
	o.Two_Two_Prod(a21[1], a21[0], a32[1], a32[0], a2132); // a2132 = a21*a32;
	o.Two_Two_Prod(a22[1], a22[0], a31[1], a31[0], a2231); // a2231 = a22*a31;

	o.Gen_Invert(8, a2332);
	o.Gen_Invert(8, a2331);
	o.Gen_Invert(8, a2231);
	dd1l = o.Gen_Sum(8, a2233, 8, a2332, dd1); // dd1 = a2233 + a2332;
	dd2l = o.Gen_Sum(8, a2133, 8, a2331, dd2); // dd2 = a2133 + a2331;
	dd3l = o.Gen_Sum(8, a2132, 8, a2231, dd3); // dd3 = a2132 + a2231;

	ee1l = o.Gen_Product_With_PreAlloc(a11l, a11, dd1l, dd1, &ee1, ee1l); // ee1 = a11*dd1;
	ee2l = o.Gen_Product_With_PreAlloc(a13l, a13, dd3l, dd3, &ee2, ee2l); // ee2 = a13*dd3;
	ffl = o.Gen_Sum_With_PreAlloc(ee1l, ee1, ee2l, ee2, &ff, ffl); // ff = ee1 + ee2;
	if (ee1 != ee1p) { free(ee1); ee1 = ee1p; }
	if (ee2 != ee2p) free(ee2);
	ee1l = o.Gen_Product_With_PreAlloc(a12l, a12, dd2l, dd2, &ee1, ee1l); // ee1 = a12*dd2;

	if (a11 != a11p) free(a11);
	if (a12 != a12p) free(a12);
	if (a13 != a13p) free(a13);

	o.Gen_Invert(ee1l, ee1);
	detl = o.Gen_Sum_With_PreAlloc(ffl, ff, ee1l, ee1, &det, detl); // det = ff + ee1;
	if (ee1 != ee1p) free(ee1);
	if (ff != ffp) free(ff);

	double s = det[detl - 1];
	double sd = d[dl - 1];

	if (det != detp) free(det);

	if ((s > 0)) return (sd>0) ? (1) : (-1);
	if ((s < 0)) return (sd>0) ? (-1) : (1);
	return 0;
}

int orient3D_TPI_post_exact(
	TPI_exact_suppvars& s,
	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
	)
{
	tposte++;
	return orient3D_TPI_post_exact(s.d, s.dl, s.n1, s.n1l, s.n2, s.n2l, s.n3, s.n3l, q1x, q1y, q1z, q2x, q2y, q2z, q3x, q3y, q3z);
}

TPI_exact_suppvars::TPI_exact_suppvars()
{
		d = dp;
		n1 = n1p;
		n2 = n2p;
		n3 = n3p;
		dl = n1l = n2l = n3l = 256;
}

TPI_exact_suppvars::~TPI_exact_suppvars()
{
		if (dl > 256) free(d);
		if (n1l > 256) free(n1);
		if (n2l > 256) free(n2);
		if (n3l > 256) free(n3);
}

int orient3D_TPI_exact(
	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
	)
{
	TPI_exact_suppvars s;

	if (orient3D_TPI_pre_exact(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z, u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z, s))
		return orient3D_TPI_post_exact(s, q1x, q1y, q1z, q2x, q2y, q2z, q3x, q3y, q3z);
	return 0;
}

int orient3D_LPI(
	double px, double py, double pz,
	double qx, double qy, double qz,
	double rx, double ry, double rz,
	double sx, double sy, double sz,
	double tx, double ty, double tz,
	double ax, double ay, double az,
	double bx, double by, double bz,
	double cx, double cy, double cz)
{
	int r = orient3D_LPI_filtered(px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz, tx, ty, tz, ax, ay, az, bx, by, bz, cx, cy, cz);
	if (r != Filtered_Orientation::UNCERTAIN) return r;
	return orient3D_LPI_exact(px, py, pz, qx, qy, qz, rx, ry, rz, sx, sy, sz, tx, ty, tz, ax, ay, az, bx, by, bz, cx, cy, cz);
}

int orient3D_TPI(
	double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
	double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
	double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
	double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z
)
{
	int r = orient3D_TPI_filtered(
		v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
		w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z,
		u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z,
		q1x, q1y, q1z, q2x, q2y, q2z, q3x, q3y, q3z);
	if (r != Filtered_Orientation::UNCERTAIN) return r;
	return orient3D_TPI_exact(
		v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z,
		w1x, w1y, w1z, w2x, w2y, w2z, w3x, w3y, w3z,
		u1x, u1y, u1z, u2x, u2y, u2z, u3x, u3y, u3z,
		q1x, q1y, q1z, q2x, q2y, q2z, q3x, q3y, q3z);
}

