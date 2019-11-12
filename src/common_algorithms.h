#pragma once
#include<fastenvelope/Types.hpp>

namespace fastEnvelope {
	namespace algorithms {
		 int seg_cut_plane(const Vector3 &seg0, const Vector3 &seg1, const Vector3 &t0, const Vector3 &t1, const Vector3 &t2);
		
		 int is_triangle_degenerated(const Vector3& triangle0, const Vector3& triangle1, const Vector3& triangle2);
		 Vector3 accurate_normal_vector(const Vector3 &p0, const Vector3 &p1, const Vector3 &q0, const Vector3 &q1);
		 void get_tri_corners(const Vector3 &triangle0, const Vector3 &triangle1, const Vector3 &triangle2, Vector3 &mint, Vector3 &maxt);
		 bool box_box_intersection(const Vector3 &min1, const Vector3 &max1, const Vector3 &min2, const Vector3 &max2);//TDOO;
		 Vector2 to_2d(const Vector3 &p, int t);
	}
}