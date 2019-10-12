#include <fastenvelope/AABB.h>
#include <fastenvelope/FastEnvelope.h>
#include<fastenvelope/Predicates.hpp>
#include <cassert>

namespace fastEnvelope {
	namespace {
		bool box_box_intersection(const Vector3 &min1, const Vector3 &max1, const Vector3 &min2, const Vector3 &max2)
		{
			if (max1[0] < min2[0] || max1[1] < min2[1] || max1[2] < min2[2])
				return 0;
			if (max2[0] < min1[0] || max2[1] < min1[1] || max2[2] < min1[2])
				return 0;
			return 1;
		}

		void get_tri_corners(const Vector3 &triangle0, const Vector3 &triangle1, const Vector3 &triangle2, Vector3 &mint, Vector3 &maxt)
		{
			mint[0] = std::min(std::min(triangle0[0], triangle1[0]), triangle2[0]);
			mint[1] = std::min(std::min(triangle0[1], triangle1[1]), triangle2[1]);
			mint[2] = std::min(std::min(triangle0[2], triangle1[2]), triangle2[2]);
			maxt[0] = std::max(std::max(triangle0[0], triangle1[0]), triangle2[0]);
			maxt[1] = std::max(std::max(triangle0[1], triangle1[1]), triangle2[1]);
			maxt[2] = std::max(std::max(triangle0[2], triangle1[2]), triangle2[2]);
		}
	}


	void AABB::init_envelope_boxes_recursive(
		const std::vector<std::array<Vector3, 2>> &cornerlist,
		int node_index,
		int b, int e)
	{
		assert(b != e);
		assert(node_index < boxlist.size());


		if (b + 1 == e) {
			boxlist[node_index] = cornerlist[b];
			return;
		}
		int m = b + (e - b) / 2;
		int childl = 2 * node_index;
		int childr = 2 * node_index + 1;

		assert(childl < boxlist.size());
		assert(childr < boxlist.size());

		init_envelope_boxes_recursive(cornerlist, childl, b, m);
		init_envelope_boxes_recursive(cornerlist, childr, m, e);

		assert(childl < boxlist.size());
		assert(childr < boxlist.size());
		for (int c = 0; c < 3; ++c) {
			boxlist[node_index][0][c] = std::min(boxlist[childl][0][c], boxlist[childr][0][c]);
			boxlist[node_index][1][c] = std::max(boxlist[childl][1][c], boxlist[childr][1][c]);
		}
	}

	void AABB::facet_in_envelope_recursive(
		const Vector3 &triangle0, const Vector3 &triangle1, const Vector3 &triangle2,
		std::vector<unsigned int> &list,
		int n, int b, int e) const
	{
		assert(e != b);

		assert(n < boxlist.size());
		bool cut = is_triangle_cut_bounding_box(triangle0, triangle1, triangle2, n);

		if (cut == false) return;

		// Leaf case
		if (e == b + 1) {
			list.emplace_back(b);
			return;
		}

		int m = b + (e - b) / 2;
		int childl = 2 * n;
		int childr = 2 * n + 1;

		//assert(childl < boxlist.size());
		//assert(childr < boxlist.size());



		// Traverse the "nearest" child first, so that it has more chances
		// to prune the traversal of the other child.
		facet_in_envelope_recursive(
			triangle0, triangle1, triangle2, list,
			childl, b, m
		);
		facet_in_envelope_recursive(
			triangle0, triangle1, triangle2, list,
			childr, m, e
		);
	}
	
	void AABB::bbd_searching_recursive(
		const Vector3 &bbd0, const Vector3 &bbd1,
		std::vector<unsigned int> &list,
		int n, int b, int e) const
	{
		assert(e != b);

		assert(n < boxlist.size());
		bool cut = is_bbd_cut_bounding_box(bbd0, bbd1, n);

		if (cut == false) return;

		// Leaf case
		if (e == b + 1) {
			list.emplace_back(b);
			return;
		}

		int m = b + (e - b) / 2;
		int childl = 2 * n;
		int childr = 2 * n + 1;

		//assert(childl < boxlist.size());
		//assert(childr < boxlist.size());



		// Traverse the "nearest" child first, so that it has more chances
		// to prune the traversal of the other child.
		bbd_searching_recursive(
			bbd0, bbd1, list,
			childl, b, m
		);
		bbd_searching_recursive(
			bbd0, bbd1, list,
			childr, m, e
		);
	}
	int AABB::envelope_max_node_index(int node_index, int b, int e) {
		assert(e > b);
		if (b + 1 == e) {
			return node_index;
		}
		int m = b + (e - b) / 2;
		int childl = 2 * node_index;
		int childr = 2 * node_index + 1;
		return std::max(
			envelope_max_node_index(childl, b, m),
			envelope_max_node_index(childr, m, e)
		);
	}

	void AABB::init_envelope(const std::vector<std::array<Vector3, 2>> &cornerlist)
	{
		n_corners = cornerlist.size();

		boxlist.resize(
			envelope_max_node_index(
				1, 0, n_corners) +
			1 // <-- this is because size == max_index + 1 !!!
		);

		init_envelope_boxes_recursive(cornerlist, 1, 0, n_corners);
	}
	static const int NOT_INTERSECTED = 2;
	static const int INTERSECTED = 1;
	static const int OUT_PRISM = 1;
	static const int IN_PRISM = 0;
	static const int CUT_COPLANAR = 4;
	static const int CUT_EMPTY = -1;
	static const int CUT_FACE = 3;

	static const int NOT_DEGENERATED = 0;
	static const int NERLY_DEGENERATED = 1;
	static const int DEGENERATED_SEGMENT = 2;
	static const int DEGENERATED_POINT = 3;

	int is_triangle_degenerated(const Vector3 &triangle0, const Vector3 &triangle1, const Vector3 &triangle2)
	{
		const auto to_2d = [](const Vector3 &p, int t)
		{
			return Vector2(p[(t + 1) % 3], p[(t + 2) % 3]);
		};

		Vector3 a = triangle0 - triangle1, b = triangle0 - triangle2;
		Vector3 normal = a.cross(b);
		Scalar nbr = normal.norm();

		if (nbr > SCALAR_ZERO)
		{
			return NOT_DEGENERATED;
		}
		int ori;
		std::array<Vector2, 3> p;
		for (int j = 0; j < 3; j++)
		{

			p[0] = to_2d(triangle0, j);
			p[1] = to_2d(triangle1, j);
			p[2] = to_2d(triangle2, j);

			ori = Predicates::orient_2d(p[0], p[1], p[2]);
			if (ori != 0)
			{
				return NERLY_DEGENERATED;
			}
		}

		if (triangle0[0] != triangle1[0] || triangle0[1] != triangle1[1] || triangle0[2] != triangle1[2])
		{
			return DEGENERATED_SEGMENT;
		}
		if (triangle0[0] != triangle2[0] || triangle0[1] != triangle2[1] || triangle0[2] != triangle2[2])
		{
			return DEGENERATED_SEGMENT;
		}
		return DEGENERATED_POINT;
	}


	bool AABB::is_triangle_cut_bounding_box(
		const Vector3 &tri0, const Vector3 &tri1, const Vector3 &tri2, int index) const
	{
		const auto &bmin = boxlist[index][0];
		const auto &bmax = boxlist[index][1];
		Vector3 tmin, tmax;
		
		get_tri_corners(tri0, tri1, tri2, tmin, tmax);
		bool cut= box_box_intersection(tmin, tmax, bmin, bmax);
		if (cut == false) return false;
		int de = is_triangle_degenerated(tri0, tri1, tri2);
		if (de == DEGENERATED_SEGMENT || DEGENERATED_POINT) return true;
		if (cut) {
			
			std::array<Vector2,3> tri;
			std::array<Vector2,4> mp;
			int o0, o1, o2, o3, ori;
			for (int i = 0; i < 3; i++) {
				tri[0] = to_2d(tri0, i);
				tri[1] = to_2d(tri1, i);
				tri[2] = to_2d(tri2, i);

				mp[0] = to_2d(bmin, i);
				mp[1] = to_2d(bmax, i);
				mp[2][0] = mp[0][0]; mp[2][1] = mp[1][1];
				mp[3][0] = mp[1][0]; mp[3][1] = mp[0][1];
		
				for (int j = 0; j < 3; j++) {
					o0 = fastEnvelope::Predicates::orient_2d(mp[0], tri[j % 3], tri[(j + 1) % 3]);
					o1 = fastEnvelope::Predicates::orient_2d(mp[1], tri[j % 3], tri[(j + 1) % 3]);
					o2 = fastEnvelope::Predicates::orient_2d(mp[2], tri[j % 3], tri[(j + 1) % 3]);
					o3 = fastEnvelope::Predicates::orient_2d(mp[3], tri[j % 3], tri[(j + 1) % 3]);
					ori = fastEnvelope::Predicates::orient_2d(tri[(j + 2) % 3], tri[j % 3], tri[(j + 1) % 3]);
					if (ori == 0) continue;
					if (ori*o0 <= 0 && ori*o1 <= 0 && ori*o2 <= 0 && ori*o3 <= 0) return false;
				}
			}
		}

		return cut;
	}
	bool AABB::is_bbd_cut_bounding_box(
		const Vector3 &bbd0, const Vector3 &bbd1, int index) const
	{
		const auto &bmin = boxlist[index][0];
		const auto &bmax = boxlist[index][1];

		
		return box_box_intersection(bbd0, bbd1, bmin, bmax);
	}
}
