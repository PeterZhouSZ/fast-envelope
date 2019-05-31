#pragma once
#include <fastenvelope/Types.hpp>
#include<vector>
#include<fastenvelope/Parameters.h>
#include <fenv.h>
namespace fastEnvelope {
	

	class FastEnvelope
	{
	public:
		static const int ORI_POSITIVE = 1;
		static const int ORI_ZERO = 0;
		static const int ORI_NEGATIVE = -1;
		static const int NOT_INTERSECTD = 2;
		static const int OUT_PRISM = 1;
		static const int IN_PRISM = 0;
		static const int CUT_COPLANAR = 4;
		static const int CUT_EMPTY = -1;
		static const int CUT_FACE = 3;
		
		//static bool FastEnvelopeTest(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);
		//static bool FastEnvelopeTestTemp(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);
		static bool FastEnvelopeTestImplicit(const std::array<Vector3, 3> &triangle, const std::vector<std::array<Vector3, 12>>& envprism);
		
		
		// to check if a point is in the prisms. the jump index shows the prisms not counted in calculation, and jump is sorted from small to big
		static bool point_out_prism(const Vector3& point, const std::vector<std::array<Vector3, 12>>& envprism, const std::vector<int>& jump);

		static void BoxGeneration(const std::vector<Vector3>& m_ver, const std::vector<Vector3i>& m_faces, std::vector<std::array<Vector3, 12>>& envprism, const Scalar& bbd);
		static int orient_3triangles(const Eigen::Matrix<Scalar, 3, 3>& A,const Eigen::Matrix<Scalar, 3, 3>& AT,
			const Eigen::Matrix<Scalar, 3, 3>& ATA,const Eigen::Matrix<Scalar, 3, 1>& B, const std::array<Vector3, 3> & triangle3);
		static int Implicit_Seg_Facet_interpoint_Out_Prism(const Vector3& segpoint0, const Vector3& segpoint1, const std::array<Vector3, 3>& triangle,
			const std::vector<std::array<Vector3, 12>>& envprism,const std::vector<int>& jump);
	
		static int Implicit_Tri_Facet_Facet_interpoint_Out_Prism( const std::array<Vector3, 3>& triangle, const std::array<Vector3, 3>& facet1, const std::array<Vector3, 3>& facet2,
			const std::vector<std::array<Vector3, 12>>& envprism, const std::vector<int>& jump);
		static bool is_seg_facet_intersection(const double& px, const double& py, const double& pz,
			const double& qx, const double& qy, const double& qz,
			const double& rx, const double& ry, const double& rz,
			const double& sx, const double& sy, const double& sz,
			const double& tx, const double& ty, const double& tz,
			double& a11, double& a12, double& a13,
			double& a21, double& a22, double& a23,
			double& a31, double& a32, double& a33,
			double& px_rx, double& py_ry, double& pz_rz,
			double& d, double& n);
		


		static int tri_cut_tri_simple(const Vector3& p1, const Vector3& p2, const Vector3& p3,const Vector3& q1, const Vector3& q2, const Vector3& q3);
	// Indirect 3D orientation predicate with floating point filter.
	//
	// Input: eight points p, q, r, s, t, a, b, c
	// where:
	// <p, q> define a straight line L
	// <r, s, t> define a plane P1
	// <a, b, c> define a plane P2
	//
	// Output:
	// Let i be the exact intersection point between L and P1.
	// orient3D_LPI returns 1 if the tetrahedron (i, a, b, c) has positive volume.
	// Returns -1 if it has negative volume.
	// Zero is returned if one of the following conditions holds:
	// - degenerate input (L and P1 are parallel, <r, s, t> are collinear, ...)
	// - i is exactly on P2
	// - the floating point precision is insufficient to guarantee an exact answer
	// - the input coordinates cause an under/overflow during the computation.

		static	inline double det4x4(
			double a11, double a12, double a13, double a14,
			double a21, double a22, double a23, double a24,
			double a31, double a32, double a33, double a34,
			double a41, double a42, double a43, double a44)
		{
			double m12 = ((a21 * a12) - (a11 * a22));
			double m13 = ((a31 * a12) - (a11 * a32));
			double m14 = ((a41 * a12) - (a11 * a42));
			double m23 = ((a31 * a22) - (a21 * a32));
			double m24 = ((a41 * a22) - (a21 * a42));
			double m34 = ((a41 * a32) - (a31 * a42));
			double m123 = (((m23 * a13) - (m13 * a23)) + (m12 * a33));
			double m124 = (((m24 * a13) - (m14 * a23)) + (m12 * a43));
			double m134 = (((m34 * a13) - (m14 * a33)) + (m13 * a43));
			double m234 = (((m34 * a23) - (m24 * a33)) + (m23 * a43));
			return ((((m234 * a14) - (m134 * a24)) + (m124 * a34)) - (m123 * a44));
		}

		static inline double det3x3(double a11, double a12, double a13, double a21, double a22, double a23, double a31, double a32, double a33) {
			return (((a11 * ((a22 * a33) - (a23 * a32))) - (a12 * ((a21 * a33) - (a23 * a31)))) + (a13 * ((a21 * a32) - (a22 * a31))));
		}

		static inline double dot(double v1x, double v1y, double v1z, double v2x, double v2y, double v2z) { return (((v1x * v2x) + (v1y * v2y)) + (v1z * v2z)); }
		static inline void cross(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3) {
			c1 = a2 * b3 - a3 * b2;
			c2 = b1 * a3 - a1 * b3;
			c3 = a1 * b2 - a2 * b1;
		}
		static inline double det2x2(double a1, double a2, double b1, double b2) { return ((a1 * b2) - (a2 * b1)); }

		// Indirect 3D orientation predicate with floating point filter.
		//
		// Input: eight points p, q, r, s, t, a, b, c
		// where:
		// <p, q> define a straight line L
		// <r, s, t> define a plane P1
		// <a, b, c> define a plane P2
		//
		// Output:
		// Let i be the exact intersection point between L and P1.
		// orient3D_LPI returns 1 if the tetrahedron (i, a, b, c) has positive volume.
		// Returns -1 if it has negative volume.
		// Zero is returned if one of the following conditions holds:
		// - degenerate input (L and P1 are parallel, <r, s, t> are collinear, ...)
		// - i is exactly on P2
		// - the floating point precision is insufficient to guarantee an exact answer
		// - the input coordinates cause an under/overflow during the computation.

		static inline int orient3D_LPI(//right hand law
			const double& px, const double& py, const double& pz,
			const double& ax, const double& ay, const double& az,
			const double& bx, const double& by, const double& bz,
			const double& cx, const double& cy, const double& cz,
			const double& a11, const double& a12, const double& a13,
			const double& a21, const double& a22, const double& a23,
			const double& a31, const double& a32, const double& a33,
			const double& px_rx, const double& py_ry, const double& pz_rz,
			const double& d, const double& n);



		static bool is_3triangle_intersect(double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
			double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
			double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
			double m11, double m12, double m13, double d) {
			::feclearexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
			double t2x = v2x, t2y = v2y, t2z = v2z, t3x = v3x, t3y = v3y, t3z = v3z;//still need these
			double v4x = v1x - v3x;
			double v4y = v1y - v3y;
			double v4z = v1z - v3z;
			v3x -= v2x;
			v3y -= v2y;
			v3z -= v2z;
			v2x -= v1x;
			v2y -= v1y;
			v2z -= v1z;
			double nvx = det2x2(v2y, v2z, v3y, v3z);//v3=v2v3, v2=v1v2;v4=v3v1
			double nvy = det2x2(v3x, v3z, v2x, v2z);
			double nvz = det2x2(v2x, v2y, v3x, v3y);

			w3x -= w2x;
			w3y -= w2y;
			w3z -= w2z;
			w2x -= w1x;
			w2y -= w1y;
			w2z -= w1z;
			double nwx = det2x2(w2y, w2z, w3y, w3z);
			double nwy = det2x2(w3x, w3z, w2x, w2z);
			double nwz = det2x2(w2x, w2y, w3x, w3y);

			u3x -= u2x;
			u3y -= u2y;
			u3z -= u2z;
			u2x -= u1x;
			u2y -= u1y;
			u2z -= u1z;
			double nux = det2x2(u2y, u2z, u3y, u3z);
			double nuy = det2x2(u3x, u3z, u2x, u2z);
			double nuz = det2x2(u2x, u2y, u3x, u3y);

			d = det3x3(nvx, nvy, nvz, nwx, nwy, nwz, nux, nuy, nuz);
			if (d < SCALAR_ZERO_3) {// if not intersected
				return 0;
			}
			double enlarge = abs(nvx);
			//enlarge=enlarge<fabs(nvy)?
			double dot1 = dot(nvx, nvy, nvz, v1x, v1y, v1z);
			double dot2 = dot(nwx, nwy, nwz, w1x, w1y, w1z);
			double dot3 = dot(nux, nuy, nuz, u1x, u1y, u1z);//B

			m11 = det3x3(dot1, nvy, nvz, dot2, nwy, nwz, dot3, nuy, nuz);
			m12 = det3x3(nvx, dot1, nvz, nwx, dot2, nwz, nux, dot3, nuz);
			m13 = det3x3(nvx, nvy, dot1, nwx, nwy, dot2, nux, nuy, dot3);

			////////////////////////////////////////////////////////////////
			// this part is to predicate if the point is in the interior of triangle v
			double  f11, f12, f13, f21, f22, f23, f31, f32, f33,
				at1x = nvx * v1x + nvy * v1y + nvz * v1z,
				at1y = nwx * v1x + nwy * v1y + nwz * v1z,
				at1z = nux * v1x + nuy * v1y + nuz * v1z,

				at2x = nvx * t2x + nvy * t2y + nvz * t2z,
				at2y = nwx * t2x + nwy * t2y + nwz * t2z,
				at2z = nux * t2x + nuy * t2y + nuz * t2z,

				at3x = nvx * t3x + nvy * t3y + nvz * t3z,
				at3y = nwx * t3x + nwy * t3y + nwz * t3z,
				at3z = nux * t3x + nuy * t3y + nuz * t3z;

			cross(dot1 - at1x, dot2 - at1y, dot3 - at1z, at2x - at1x, at2y - at1y, at2z - at1z, f11, f12, f13);
			cross(dot1 - at2x, dot2 - at2y, dot3 - at2z, at3x - at2x, at3y - at2y, at3z - at2z, f21, f22, f23);
			cross(dot1 - at3x, dot2 - at3y, dot3 - at3z, at1x - at3x, at1y - at3y, at1z - at3z, f31, f32, f33);
			double mark1 = dot(f11, f12, f13, f21, f22, f23), mark2 = dot(f11, f12, f13, f31, f32, f33);
			///////////////////////////////////////////////////
			if (mark1 > 0 && mark2 > 0) {
				return 1;
			}
			return 0;
		}



		static inline int orient3D_TPI(
			double v1x, double v1y, double v1z, double v2x, double v2y, double v2z, double v3x, double v3y, double v3z,
			double w1x, double w1y, double w1z, double w2x, double w2y, double w2z, double w3x, double w3y, double w3z,
			double u1x, double u1y, double u1z, double u2x, double u2y, double u2z, double u3x, double u3y, double u3z,
			double q1x, double q1y, double q1z, double q2x, double q2y, double q2z, double q3x, double q3y, double q3z,
			double m11, double m12, double m13, double d)
		{


			// If the same intersection point must be tested against several planes,
			// code up to here can be extracted and computed only once.

			double det = det4x4(
				m11, m12, m13, 1,
				d*q1x, d*q1y, d*q1z, 1,
				d*q2x, d*q2y, d*q2z, 1,
				d*q3x, d*q3y, d*q3z, 1
			);
			if (::fetestexcept(FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)) return 0; // Fast reject in case of under/overflow

			// Almost static filter

			double fv1x = fabs(v1x);
			double fv1y = fabs(v1y);
			double fv1z = fabs(v1z);
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

			double fu1x = fabs(u1x);
			double fu1y = fabs(u1y);
			double fu1z = fabs(u1z);
			double fu2x = fabs(u2x);
			double fu2y = fabs(u2y);
			double fu2z = fabs(u2z);
			double fu3x = fabs(u3x);
			double fu3y = fabs(u3y);
			double fu3z = fabs(u3z);

			double fq1x = fabs(q1x);
			double fq1y = fabs(q1y);
			double fq1z = fabs(q1z);
			double fq2x = fabs(q2x);
			double fq2y = fabs(q2y);
			double fq2z = fabs(q2z);
			double fq3x = fabs(q3x);
			double fq3y = fabs(q3y);
			double fq3z = fabs(q3z);

			double max1, max2, max3, max4, max5, max6, max7, max8, max9, max10, max11, max12, max13, max14;

			max10 = fq3z;
			if (max10 < fq1z) max10 = fq1z;
			if (max10 < fq2z) max10 = fq2z;
			max1 = max10;
			max4 = fu3z;
			if (max4 < fu2z) max4 = fu2z;
			if (max4 < fw2z) max4 = fw2z;
			if (max4 < fw3z) max4 = fw3z;
			if (max1 < max4) max1 = max4;
			max11 = fu3y;
			if (max11 < fw2y) max11 = fw2y;
			if (max11 < fw3y) max11 = fw3y;
			if (max11 < fu2y) max11 = fu2y;
			max2 = max11;
			max14 = fv2z;
			if (max14 < fw2z) max14 = fw2z;
			if (max14 < fv3z) max14 = fv3z;
			if (max14 < fw3z) max14 = fw3z;
			if (max2 < max14) max2 = max14;
			max13 = fw2y;
			if (max13 < fw3y) max13 = fw3y;
			if (max13 < fv2y) max13 = fv2y;
			if (max13 < fv3y) max13 = fv3y;
			if (max2 < max13) max2 = max13;
			max3 = max14;
			if (max3 < max4) max3 = max4;
			if (max2 < max3) max2 = max3;
			if (max2 < max4) max2 = max4;
			max12 = fw2x;
			if (max12 < fw3x) max12 = fw3x;
			if (max12 < fv3x) max12 = fv3x;
			if (max12 < fv2x) max12 = fv2x;
			max5 = max12;
			if (max5 < fq1x) max5 = fq1x;
			if (max5 < fq2x) max5 = fq2x;
			if (max5 < fq3x) max5 = fq3x;
			max6 = max11;
			if (max6 < max12) max6 = max12;
			if (max6 < max13) max6 = max13;
			max8 = fu3x;
			if (max8 < fw2x) max8 = fw2x;
			if (max8 < fw3x) max8 = fw3x;
			if (max8 < fu2x) max8 = fu2x;
			if (max6 < max8) max6 = max8;
			max7 = max12;
			if (max7 < max8) max7 = max8;
			if (max6 < max7) max6 = max7;
			max9 = max11;
			if (max9 < max13) max9 = max13;
			if (max9 < fq1y) max9 = fq1y;
			if (max9 < fq2y) max9 = fq2y;
			if (max9 < fq3y) max9 = fq3y;

			double eps = 4.5906364908640589e-008 * max13 * max14 * max12 * max4 * max8 * max11 * max5 * max13 * max14 * max12 * max4 * max8 * max11 * max9 * max9 * max3 * max7 * max1 * max6 * max2 * max10;

			if (det > eps) return (d > 0) ? (1) : (-1);
			if (det < -eps) return (d > 0) ? (-1) : (1);

			return 0;
		}


	
	};
}
