#include <fastenvelope/FastEnvelope.h>
#include <fastenvelope/Predicates.hpp>
#include <fastenvelope/Logger.hpp>
#include<fastenvelope/Morton.h>
#include<fastenvelope/common_algorithms.h>

#include <fstream>



namespace fastEnvelope
{
	namespace {
		

		//delete me in the future
		void triangle_sample_segment(const std::array<Vector3, 3> &triangle, Vector3 &ps, const int &pieces, const int &nbr)
		{
			int t = pieces - 1;
			if (triangle[1] - triangle[0] == Vector3(0, 0, 0))
			{

				ps = (triangle[0] + (triangle[2] - triangle[0]) * nbr / t);

				return;
			}
			if (triangle[2] - triangle[0] == Vector3(0, 0, 0))
			{

				ps = (triangle[0] + (triangle[1] - triangle[0]) * nbr / t);

				return;
			}
			if (triangle[2] - triangle[1] == Vector3(0, 0, 0))
			{

				ps = (triangle[0] + (triangle[1] - triangle[0]) * nbr / t);

				return;
			}

			Scalar d1 = (triangle[1] - triangle[0]).norm(), d2 = (triangle[2] - triangle[0]).norm(), d3 = (triangle[1] - triangle[2]).norm();
			if (d1 >= d2 && d1 >= d3)
			{
				ps = (triangle[0] + (triangle[1] - triangle[0]) * nbr / t);

				return;
			}
			if (d2 >= d1 && d2 >= d3)
			{
				ps = (triangle[0] + (triangle[2] - triangle[0]) * nbr / t);

				return;
			}
			if (d3 >= d1 && d3 >= d2)
			{
				ps = (triangle[1] + (triangle[2] - triangle[1]) * nbr / t);

				return;
			}
		}
	}
	void FastEnvelope::printnumber() {
		
	}


	FastEnvelope::FastEnvelope(const std::vector<Vector3> &m_ver, const std::vector<Vector3i> &m_faces, const Scalar eps)
	{
	
		Vector3 min, max;
		min = m_ver.front();
		max = m_ver.front();

		for (size_t j = 0; j < m_ver.size(); j++)
		{
			for (int i = 0; i < 3; i++)
			{
				min(i) = std::min(min(i), m_ver[j](i));
				max(i) = std::max(max(i), m_ver[j](i));
			}
		}
		
		

		const Scalar bbd = (max - min).norm();
		const Scalar epsilon = bbd * eps; //eps*bounding box diagnal
		
		
		std::vector<Vector3i> faces_new;

	
		resorting(m_ver, m_faces, faces_new);//resort the facets order
		//----
		

		

		halfspace_init(m_ver, faces_new, halfspace, cornerlist, epsilon);


		
		


		
		tree.init_envelope(cornerlist);
		
		

		//initializing types
		initFPU();

		
	}
	void FastEnvelope::resorting(const std::vector<Vector3> &V, const std::vector<Vector3i> &F, std::vector<Vector3i> &fnew) {
		std::vector<std::array<int,3>> ct;
		struct sortstruct {
			int order;
			Resorting::MortonCode64 morton;
		};
		std::vector<sortstruct> list;
		const int multi = 1000;
		ct.resize(F.size());
		list.resize(F.size());
		
		for (int i = 0; i < F.size(); i++) {
			ct[i][0] = int(((V[F[i][0]] + V[F[i][1]] + V[F[i][2]])*multi)[0]);
			ct[i][1] = int(((V[F[i][0]] + V[F[i][1]] + V[F[i][2]])*multi)[1]);
			ct[i][2] = int(((V[F[i][0]] + V[F[i][1]] + V[F[i][2]])*multi)[2]);
			list[i].morton = Resorting::MortonCode64(ct[i][0], ct[i][1], ct[i][2]);
			list[i].order = i;
		}
		const auto morton_compare = [](const sortstruct &a, const sortstruct &b)
		{
			return (a.morton < b.morton);
		};
		std::sort(list.begin(), list.end(), morton_compare);

		fnew.resize(F.size());
		for (int i = 0; i < F.size(); i++) {
			fnew[i] = F[list[i].order];
		}


	}
	bool FastEnvelope::is_outside(const std::array<Vector3, 3> &triangle) const
	{

		std::vector<unsigned int> querylist;
		tree.facet_in_envelope(triangle[0], triangle[1], triangle[2], querylist);
		const auto res = FastEnvelopeTestImplicit(triangle, querylist);
		return res;
	}
	bool FastEnvelope::is_outside(const Vector3 &point) const
	{

		std::vector<unsigned int> querylist;
		tree.facet_in_envelope(point, point, point, querylist);//TODO make a separate function for point
		bool out = point_out_prism(point, querylist, -1);
		if (out == true)
		{
			return 1;
		}
		return 0;
		
	}

	bool FastEnvelope::sample_triangle_outside(const std::array<Vector3, 3> &triangle, const int &pieces) const
	{

		const auto triangle_sample_normal = [](const std::array<Vector3, 3> &triangle, Vector3 &ps, const int &pieces, const int &nbr1, const int &nbr2)
		{
			int l1s = pieces - 1; //
			Vector3 p1 = triangle[0] + (triangle[1] - triangle[0]) * nbr1 / l1s, d = (triangle[2] - triangle[1]) / l1s;
			ps = p1 + d * nbr2;
		};

		const auto triangle_sample_normal_rational = [](const std::array<Vector3, 3> &triangle, Rational &ps0, Rational &ps1, Rational &ps2, const int &pieces, const int &nbr1, const int &nbr2)
		{
			int l1s = pieces - 1; //
			Rational t00(triangle[0][0]), t01(triangle[0][1]), t02(triangle[0][2]), t10(triangle[1][0]), t11(triangle[1][1]),
				t12(triangle[1][2]), t20(triangle[2][0]), t21(triangle[2][1]), t22(triangle[2][2]), nbr1r(nbr1), nbr2r(nbr2), l1sr(l1s);

			Rational p0 = t00 + (t10 - t00) * nbr1r / l1sr, d0 = (t20 - t10) / l1sr;
			Rational p1 = t01 + (t11 - t01) * nbr1r / l1sr, d1 = (t21 - t11) / l1sr;
			Rational p2 = t02 + (t12 - t02) * nbr1r / l1sr, d2 = (t22 - t12) / l1sr;
			ps0 = p0 + d0 * nbr2;
			ps1 = p1 + d1 * nbr2;
			ps2 = p2 + d2 * nbr2;
		};

		bool out;
		Vector3 point;
		std::vector<unsigned int> querylist;
		tree.facet_in_envelope(triangle[0], triangle[1], triangle[2], querylist);
		int jump = -1;
		if (querylist.size() == 0)
			return 1;

		int deg = algorithms::is_triangle_degenerated(triangle[0], triangle[1], triangle[2]);

		if (deg == DEGENERATED_POINT)
		{
			out = point_out_prism(triangle[0], querylist, jump);
			if (out == true)
			{

				return 1;
			}
			return 0;
		}
		if (deg == DEGENERATED_SEGMENT)
		{
			for (int i = 0; i < pieces; i++)
			{
				triangle_sample_segment(triangle, point, pieces, i);
				out = point_out_prism(point, querylist, jump);
				if (out == true)
				{

					return 1;
				}
			}
			return 0;
		}

		for (int i = 0; i < pieces; i++)
		{
			for (int j = 0; j <= i; j++)
			{
				triangle_sample_normal(triangle, point, pieces, i, j);
				out = point_out_prism(point, querylist, jump);
				if (out == true)
				{

					return 1;
				}
			}
		}

		return 0;
	}


	bool FastEnvelope::FastEnvelopeTestImplicit(const std::array<Vector3, 3> &triangle, const std::vector<unsigned int> &prismindex) const
	{
		if (prismindex.size() == 0)
		{
			return true;
		}
		

		int jump1, jump2;
		static const std::array<std::array<int, 2>, 3> triseg = {
	{{{0, 1}}, {{0, 2}}, {{1, 2}}}
		};


		std::vector<unsigned int> filted_intersection; filted_intersection.reserve(prismindex.size() / 3);
		std::vector<std::vector<int>>intersect_face; intersect_face.reserve(prismindex.size() / 3);
		bool out, cut;

		int inter, inter1, record1, record2,

			tti; //triangle-triangle intersection

		jump1 = -1;

		int check_id;

		
		for (int i = 0; i < 3; i++) {
			out = point_out_prism_return_local_id(triangle[i], prismindex, jump1, check_id);

			if (out) {
				
				return true;
			}
		}




		

		if (prismindex.size() == 1)
			return false;

		////////////////////degeneration fix

		
		int degeneration = algorithms::is_triangle_degenerated(triangle[0], triangle[1], triangle[2]);

		if (degeneration == DEGENERATED_POINT) //case 1 degenerate to a point
		{
			return false;
		}

		
		
		if (degeneration == DEGENERATED_SEGMENT)
		{
			std::vector<unsigned int > queue, idlist;
			queue.emplace_back(check_id);//queue contains the id in prismindex
			idlist.emplace_back(prismindex[check_id]);
			
			std::vector<int> cidl; cidl.reserve(8);
			for (int i = 0; i < queue.size(); i++) {

				jump1 = prismindex[queue[i]];

				for (int k = 0; k < 3; k++) {
					cut = is_seg_cut_polyhedra(jump1, triangle[triseg[k][0]], triangle[triseg[k][1]], cidl);
					if (cut&&cidl.size() == 0)continue;
					if (!cut) continue;
					for (int j = 0; j < cidl.size(); j++) {
						
						inter = Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id(triangle[triseg[k][0]], triangle[triseg[k][1]],
							halfspace[prismindex[queue[i]]][cidl[j]][0], halfspace[prismindex[queue[i]]][cidl[j]][1], halfspace[prismindex[queue[i]]][cidl[j]][2],
							idlist, jump1, check_id);

						if (inter == 1)
						{
							inter = Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id(triangle[triseg[k][0]], triangle[triseg[k][1]],
								halfspace[prismindex[queue[i]]][cidl[j]][0], halfspace[prismindex[queue[i]]][cidl[j]][1], halfspace[prismindex[queue[i]]][cidl[j]][2],
								prismindex, jump1, check_id);

							if (inter == 1) {

								return true;
							}
							if (inter == 0) {
								queue.emplace_back(check_id);
								idlist.emplace_back(prismindex[check_id]);
							}
						}
					}
				}
			}

			return false;
		}
		//
		////////////////////////////////degeneration fix over



		std::vector<int> cidl; cidl.reserve(8);
		for (int i = 0; i < prismindex.size(); i++) {
			tti = is_triangle_cut_envelope_polyhedra(prismindex[i],
				triangle[0], triangle[1], triangle[2], cidl);
			if (tti == 2) {

				return false;
			}
			else if (tti == 1 && cidl.size() > 0) {
				
				filted_intersection.emplace_back(prismindex[i]);
				intersect_face.emplace_back(cidl);


			}
		}


		if (filted_intersection.size() == 0) {

			return false;
		}


		//tree

		AABB localtree;
		std::vector<std::array<Vector3, 2>> localcorner;
		localcorner.resize(filted_intersection.size());

		for (int i = 0; i < filted_intersection.size(); i++) {
			localcorner[i] = cornerlist[filted_intersection[i]];
		}

		localtree.init_envelope(localcorner);

		//tree end


		std::vector<unsigned int > queue, idlist;
		std::vector<bool> coverlist;
		coverlist.resize(filted_intersection.size());
		for (int i = 0; i < coverlist.size(); i++) {
			coverlist[i] = false;
		}
		queue.emplace_back(0);//queue contains the id in filted_intersection
		idlist.emplace_back(filted_intersection[queue[0]]);// idlist contains the id in prismid//it is fine maybe it is not really intersected
		coverlist[queue[0]] = true;//when filted_intersection[i] is already in the cover list, coverlist[i]=true 
		
		std::vector<unsigned int> neighbours;//local id
		std::vector<unsigned int > list;
		std::vector<std::vector<int>> neighbour_facets, idlistorder;
		std::vector<bool> neighbour_cover;
		idlistorder.emplace_back(intersect_face[queue[0]]);







		

		for (int i = 0; i < queue.size(); i++) {

			jump1 = filted_intersection[queue[i]];

			for (int k = 0; k < 3; k++) {
				for (int j = 0; j < intersect_face[queue[i]].size(); j++) {
					tti = algorithms::seg_cut_plane(triangle[triseg[k][0]], triangle[triseg[k][1]],
						halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][0], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][1], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][2]);
					if (tti != FE_CUT_FACE) continue;
					inter = Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id_with_face_order(triangle[triseg[k][0]], triangle[triseg[k][1]],
						halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][0], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][1], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][2],
						idlist, idlistorder, jump1, check_id);

					if (inter == 1)
					{
						inter = Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id_with_face_order_jump_over(triangle[triseg[k][0]], triangle[triseg[k][1]],
							halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][0], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][1], halfspace[filted_intersection[queue[i]]][intersect_face[queue[i]][j]][2],
							filted_intersection, intersect_face, coverlist, jump1, check_id);
						assert(inter != 2);// the point must exist
						if (inter == 1) {

							return true;
						}
						if (inter == 0) {
							idlistorder.emplace_back(intersect_face[check_id]);
							queue.emplace_back(check_id);
							idlist.emplace_back(filted_intersection[check_id]);
							coverlist[check_id] = true;
						}
					}
				}
			}
		}



	
		//tpi part
		
		for (int i = 1; i < queue.size(); i++)
		{
			jump1 = filted_intersection[queue[i]];

			localtree.bbd_finding_in_envelope(cornerlist[jump1][0], cornerlist[jump1][1], list);
			neighbours.clear();
			neighbour_cover.clear();
			neighbour_facets.clear();

			neighbours.resize(list.size());
			neighbour_facets.resize(list.size());
			neighbour_cover.resize(list.size());
			for (int j = 0; j < list.size(); j++) {
				neighbours[j] = filted_intersection[list[j]];
				neighbour_facets[j] = intersect_face[list[j]];
				if (coverlist[list[j]] == true) neighbour_cover[j] = true;
				else neighbour_cover[j] = false;
			}


		
			for (int j = 0; j < i; j++) {
				jump2 = filted_intersection[queue[j]];
				if (!algorithms::box_box_intersection(cornerlist[jump1][0], cornerlist[jump1][1], cornerlist[jump2][0], cornerlist[jump2][1]))
					continue;
				for (int k = 0; k < intersect_face[queue[i]].size(); k++) {
					for (int h = 0; h < intersect_face[queue[j]].size(); h++) {

						cut = is_3_triangle_cut(triangle,
							halfspace[jump1][intersect_face[queue[i]][k]][0],
							halfspace[jump1][intersect_face[queue[i]][k]][1],
							halfspace[jump1][intersect_face[queue[i]][k]][2],

							halfspace[jump2][intersect_face[queue[j]][h]][0],
							halfspace[jump2][intersect_face[queue[j]][h]][1],
							halfspace[jump2][intersect_face[queue[j]][h]][2]);
						

						if (!cut) continue;
						
				
						cut = is_tpp_on_polyhedra(triangle,
							halfspace[jump1][intersect_face[queue[i]][k]][0],
							halfspace[jump1][intersect_face[queue[i]][k]][1],
							halfspace[jump1][intersect_face[queue[i]][k]][2],

							halfspace[jump2][intersect_face[queue[j]][h]][0],
							halfspace[jump2][intersect_face[queue[j]][h]][1],
							halfspace[jump2][intersect_face[queue[j]][h]][2], jump1, intersect_face[queue[i]][k]);
		
						if (!cut) continue;

					
						cut = is_tpp_on_polyhedra(triangle,
							halfspace[jump1][intersect_face[queue[i]][k]][0],
							halfspace[jump1][intersect_face[queue[i]][k]][1],
							halfspace[jump1][intersect_face[queue[i]][k]][2],

							halfspace[jump2][intersect_face[queue[j]][h]][0],
							halfspace[jump2][intersect_face[queue[j]][h]][1],
							halfspace[jump2][intersect_face[queue[j]][h]][2], jump2, intersect_face[queue[j]][h]);
				
						if (!cut) continue;
		
						

						inter = Implicit_Tri_Facet_Facet_interpoint_Out_Prism_return_local_id_with_face_order(triangle,
							halfspace[jump1][intersect_face[queue[i]][k]][0],
							halfspace[jump1][intersect_face[queue[i]][k]][1],
							halfspace[jump1][intersect_face[queue[i]][k]][2],

							halfspace[jump2][intersect_face[queue[j]][h]][0],
							halfspace[jump2][intersect_face[queue[j]][h]][1],
							halfspace[jump2][intersect_face[queue[j]][h]][2],
							idlist, idlistorder, jump1, jump2, check_id);

			
						if (inter == 1) {
		
							inter = Implicit_Tri_Facet_Facet_interpoint_Out_Prism_return_local_id_with_face_order_jump_over(triangle,
								halfspace[jump1][intersect_face[queue[i]][k]][0],
								halfspace[jump1][intersect_face[queue[i]][k]][1],
								halfspace[jump1][intersect_face[queue[i]][k]][2],

								halfspace[jump2][intersect_face[queue[j]][h]][0],
								halfspace[jump2][intersect_face[queue[j]][h]][1],
								halfspace[jump2][intersect_face[queue[j]][h]][2],
								neighbours, neighbour_facets, neighbour_cover, jump1, jump2, check_id);

				
							if (inter == 1) {

								return true;
							}
							if (inter == 0) {
								idlistorder.emplace_back(intersect_face[list[check_id]]);
								queue.emplace_back(list[check_id]);
								idlist.emplace_back(filted_intersection[list[check_id]]);
								coverlist[list[check_id]] = true;
							}
						}
					}
				}
			}
		}
		

		return false;


	}
	bool FastEnvelope::debugcode(const std::array<Vector3, 3> &triangle, const std::vector<unsigned int> &prismindex) const {
		return false;
	}

	

	

	
	int FastEnvelope::Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id(
		const Vector3 &segpoint0, const Vector3 &segpoint1, 
		const Vector3 &triangle0,const Vector3 &triangle1, const Vector3 &triangle2, 
		const std::vector<unsigned int> &prismindex, const int &jump, int &id) const {
		LPI_filtered_suppvars sf;
		bool precom = orient3D_LPI_prefilter( //
			segpoint0[0], segpoint0[1], segpoint0[2],
			segpoint1[0], segpoint1[1], segpoint1[2],
			triangle0[0], triangle0[1], triangle0[2],
			triangle1[0], triangle1[1], triangle1[2],
			triangle2[0], triangle2[1], triangle2[2],
			sf);
		if (precom == false) {
			int ori;


			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],

				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2], s);

			// time_multi += timer.getElapsedTimeInSec();
			if (premulti == false) return 2;
			for (int i = 0; i < prismindex.size(); i++)
			{
				if (prismindex[i] == jump)
				{
					continue;
				}
				

				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

					// timer.start();
					ori = orient3D_LPI_post_exact(s,
						segpoint0[0], segpoint0[1], segpoint0[2],

						halfspace[prismindex[i]][j][0][0],
						halfspace[prismindex[i]][j][0][1],
						halfspace[prismindex[i]][j][0][2],

						halfspace[prismindex[i]][j][1][0],
						halfspace[prismindex[i]][j][1][1],
						halfspace[prismindex[i]][j][1][2],

						halfspace[prismindex[i]][j][2][0],
						halfspace[prismindex[i]][j][2][1],
						halfspace[prismindex[i]][j][2][2]);
					if (ori == 1 || ori == 0)
					{
						break;
					}
				}
				if (ori == -1)
				{
					id = i;
					return IN_PRISM;
				}
			}


			return OUT_PRISM;
		}
		int tot;
		int ori;
		static INDEX index;
		static std::vector<INDEX> recompute;

		recompute.clear();
		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump) continue;

			index.FACES.clear();
			tot = 0;


			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

				ori =
					orient3D_LPI_postfilter(
						sf,
						segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}

			}
			if (tot == halfspace[prismindex[i]].size())
			{
				id = i;
				return IN_PRISM;
			}

			if (ori != 1)
			{
				assert(!index.FACES.empty());
				index.Pi = i;//local id
				recompute.emplace_back(index);
			}
		}

		if (!recompute.empty())
		{

			// timer.start();
			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],
				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2],
				s);
			// time_multi += timer.getElapsedTimeInSec();

			for (int k = 0; k < recompute.size(); k++)
			{
				int in1 = prismindex[recompute[k].Pi];

				for (int j = 0; j < recompute[k].FACES.size(); j++) {
					int in2 = recompute[k].FACES[j];

					ori = orient3D_LPI_post_exact(s, segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[in1][in2][0][0], halfspace[in1][in2][0][1], halfspace[in1][in2][0][2],
						halfspace[in1][in2][1][0], halfspace[in1][in2][1][1], halfspace[in1][in2][1][2],
						halfspace[in1][in2][2][0], halfspace[in1][in2][2][1], halfspace[in1][in2][2][2]);
					// time_multi += timer.getElapsedTimeInSec();
					if (ori == 1 || ori == 0)
						break;

				}

				if (ori == -1) {
					id = recompute[k].Pi;
					return IN_PRISM;
				}
			}
		}

		return OUT_PRISM;
	}

	int FastEnvelope::Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id_with_face_order(
		const Vector3 &segpoint0, const Vector3 &segpoint1, const Vector3 &triangle0,
		const Vector3 &triangle1, const Vector3 &triangle2, const std::vector<unsigned int> &prismindex,
		const std::vector<std::vector<int>>& intersect_face, const int &jump, int &id) const {
		LPI_filtered_suppvars sl;
		bool precom = orient3D_LPI_prefilter( //
			segpoint0[0], segpoint0[1], segpoint0[2],
			segpoint1[0], segpoint1[1], segpoint1[2],
			triangle0[0], triangle0[1], triangle0[2],
			triangle1[0], triangle1[1], triangle1[2],
			triangle2[0], triangle2[1], triangle2[2],
			sl);
		if (precom == false) {
			int tot, ori, fid;

			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],

				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2], s);

			
			if (premulti == false) return 2;
			for (int i = 0; i < prismindex.size(); i++)
			{
				if (prismindex[i] == jump)
				{
					continue;
				}
				tot = 0; fid = 0;

				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

					
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
					}
					else continue;
					ori = orient3D_LPI_post_exact(s,
						segpoint0[0], segpoint0[1], segpoint0[2],

						halfspace[prismindex[i]][j][0][0],
						halfspace[prismindex[i]][j][0][1],
						halfspace[prismindex[i]][j][0][2],

						halfspace[prismindex[i]][j][1][0],
						halfspace[prismindex[i]][j][1][1],
						halfspace[prismindex[i]][j][1][2],

						halfspace[prismindex[i]][j][2][0],
						halfspace[prismindex[i]][j][2][1],
						halfspace[prismindex[i]][j][2][2]);
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

					
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
						continue;
					}

					ori = orient3D_LPI_post_exact(s,
						segpoint0[0], segpoint0[1], segpoint0[2],

						halfspace[prismindex[i]][j][0][0],
						halfspace[prismindex[i]][j][0][1],
						halfspace[prismindex[i]][j][0][2],

						halfspace[prismindex[i]][j][1][0],
						halfspace[prismindex[i]][j][1][1],
						halfspace[prismindex[i]][j][1][2],

						halfspace[prismindex[i]][j][2][0],
						halfspace[prismindex[i]][j][2][1],
						halfspace[prismindex[i]][j][2][2]);
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				if (tot == halfspace[prismindex[i]].size())
				{
					id = i;
					return IN_PRISM;
				}
			}


			return OUT_PRISM;
		}
		int tot, fid;
		int ori;
		static INDEX index;
		static std::vector<INDEX> recompute;

		recompute.clear();
		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump) continue;

			index.FACES.clear();
			tot = 0; fid = 0;


			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
				}
				else continue;
				ori =
					orient3D_LPI_postfilter(
						sl,
						segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}

			}
			if (ori == 1) continue;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
					continue;
				}

				ori =
					orient3D_LPI_postfilter(
						sl,
						segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}

			}
			if (ori == 1) continue;
			if (tot == halfspace[prismindex[i]].size())
			{
				id = i;
				return IN_PRISM;
			}

			if (ori != 1)
			{
				assert(!index.FACES.empty());
				index.Pi = i;//local id
				recompute.emplace_back(index);
			}
		}

		if (!recompute.empty())
		{

			// timer.start();
			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],
				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2],
				s);
			// time_multi += timer.getElapsedTimeInSec();

			for (int k = 0; k < recompute.size(); k++)
			{
				int in1 = prismindex[recompute[k].Pi];

				for (int j = 0; j < recompute[k].FACES.size(); j++) {
					int in2 = recompute[k].FACES[j];

					ori = orient3D_LPI_post_exact(s, segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[in1][in2][0][0], halfspace[in1][in2][0][1], halfspace[in1][in2][0][2],
						halfspace[in1][in2][1][0], halfspace[in1][in2][1][1], halfspace[in1][in2][1][2],
						halfspace[in1][in2][2][0], halfspace[in1][in2][2][1], halfspace[in1][in2][2][2]);
					// time_multi += timer.getElapsedTimeInSec();
					if (ori == 1 || ori == 0)
						break;

				}

				if (ori == -1) {
					id = recompute[k].Pi;
					return IN_PRISM;
				}
			}
		}

		return OUT_PRISM;
	}
	int FastEnvelope::Implicit_Seg_Facet_interpoint_Out_Prism_return_local_id_with_face_order_jump_over(
		const Vector3 &segpoint0, const Vector3 &segpoint1, const Vector3 &triangle0,
		const Vector3 &triangle1, const Vector3 &triangle2, const std::vector<unsigned int> &prismindex,
		const std::vector<std::vector<int>>& intersect_face,const std::vector<bool>& coverlist, const int &jump, int &id) const {
		LPI_filtered_suppvars sl;
		bool precom = orient3D_LPI_prefilter( //
			segpoint0[0], segpoint0[1], segpoint0[2],
			segpoint1[0], segpoint1[1], segpoint1[2],
			triangle0[0], triangle0[1], triangle0[2],
			triangle1[0], triangle1[1], triangle1[2],
			triangle2[0], triangle2[1], triangle2[2],
			sl);
		if (precom == false) {
			int tot, ori, fid;

			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],

				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2], s);


			if (premulti == false) return 2;
			for (int i = 0; i < prismindex.size(); i++)
			{
				if (prismindex[i] == jump)
				{
					continue;
				}
				if (coverlist[i] == true) continue;
				tot = 0; fid = 0;

				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {


					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
					}
					else continue;
					ori = orient3D_LPI_post_exact(s,
						segpoint0[0], segpoint0[1], segpoint0[2],

						halfspace[prismindex[i]][j][0][0],
						halfspace[prismindex[i]][j][0][1],
						halfspace[prismindex[i]][j][0][2],

						halfspace[prismindex[i]][j][1][0],
						halfspace[prismindex[i]][j][1][1],
						halfspace[prismindex[i]][j][1][2],

						halfspace[prismindex[i]][j][2][0],
						halfspace[prismindex[i]][j][2][1],
						halfspace[prismindex[i]][j][2][2]);
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {


					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
						continue;
					}

					ori = orient3D_LPI_post_exact(s,
						segpoint0[0], segpoint0[1], segpoint0[2],

						halfspace[prismindex[i]][j][0][0],
						halfspace[prismindex[i]][j][0][1],
						halfspace[prismindex[i]][j][0][2],

						halfspace[prismindex[i]][j][1][0],
						halfspace[prismindex[i]][j][1][1],
						halfspace[prismindex[i]][j][1][2],

						halfspace[prismindex[i]][j][2][0],
						halfspace[prismindex[i]][j][2][1],
						halfspace[prismindex[i]][j][2][2]);
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				if (tot == halfspace[prismindex[i]].size())
				{
					id = i;
					return IN_PRISM;
				}
			}


			return OUT_PRISM;
		}
		int tot, fid;
		int ori;
		static INDEX index;
		static std::vector<INDEX> recompute;

		recompute.clear();
		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump) continue;
			if (coverlist[i] == true) continue;
			index.FACES.clear();
			tot = 0; fid = 0;


			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
				}
				else continue;
				ori =
					orient3D_LPI_postfilter(
						sl,
						segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}

			}
			if (ori == 1) continue;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
					continue;
				}

				ori =
					orient3D_LPI_postfilter(
						sl,
						segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}

			}
			if (ori == 1) continue;
			if (tot == halfspace[prismindex[i]].size())
			{
				id = i;
				return IN_PRISM;
			}

			if (ori != 1)
			{
				assert(!index.FACES.empty());
				index.Pi = i;//local id
				recompute.emplace_back(index);
			}
		}

		if (!recompute.empty())
		{

			// timer.start();
			LPI_exact_suppvars s;
			bool premulti = orient3D_LPI_pre_exact(
				segpoint0[0], segpoint0[1], segpoint0[2],
				segpoint1[0], segpoint1[1], segpoint1[2],
				triangle0[0], triangle0[1], triangle0[2],
				triangle1[0], triangle1[1], triangle1[2],
				triangle2[0], triangle2[1], triangle2[2],
				s);
			// time_multi += timer.getElapsedTimeInSec();

			for (int k = 0; k < recompute.size(); k++)
			{
				int in1 = prismindex[recompute[k].Pi];

				for (int j = 0; j < recompute[k].FACES.size(); j++) {
					int in2 = recompute[k].FACES[j];

					ori = orient3D_LPI_post_exact(s, segpoint0[0], segpoint0[1], segpoint0[2],
						halfspace[in1][in2][0][0], halfspace[in1][in2][0][1], halfspace[in1][in2][0][2],
						halfspace[in1][in2][1][0], halfspace[in1][in2][1][1], halfspace[in1][in2][1][2],
						halfspace[in1][in2][2][0], halfspace[in1][in2][2][1], halfspace[in1][in2][2][2]);
					// time_multi += timer.getElapsedTimeInSec();
					if (ori == 1 || ori == 0)
						break;

				}

				if (ori == -1) {
					id = recompute[k].Pi;
					return IN_PRISM;
				}
			}
		}

		return OUT_PRISM;
	}


	

	int FastEnvelope::Implicit_Tri_Facet_Facet_interpoint_Out_Prism_return_local_id_with_face_order(
		const std::array<Vector3, 3> &triangle,
		const Vector3 &facet10, const Vector3 &facet11, const Vector3 &facet12, const Vector3 &facet20, const Vector3 &facet21, const Vector3 &facet22,
		const std::vector<unsigned int> &prismindex, const std::vector<std::vector<int>>&intersect_face, const int &jump1, const int &jump2,
		int &id) const {
	
		TPI_exact_suppvars s;
		TPI_filtered_suppvars st;

		int tot, ori, fid;
		bool pre = orient3D_TPI_prefilter(triangle[0][0], triangle[0][1], triangle[0][2],
			triangle[1][0], triangle[1][1], triangle[1][2], triangle[2][0], triangle[2][1], triangle[2][2],
			facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
			facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
			st);

		if (pre == false) {
			bool premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2], triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return 2;
			for (int i = 0; i < prismindex.size(); i++)
			{

				if (prismindex[i] == jump1 || prismindex[i] == jump2)	continue;
				if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump1][0], cornerlist[jump1][1])) continue;
				if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump2][0], cornerlist[jump2][1])) continue;
				
				tot = 0;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
					}
					else continue;
				
					ori = orient3D_TPI_post_exact(s,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);
		
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
						continue;
					}

			
					ori = orient3D_TPI_post_exact(s,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				if (tot == halfspace[prismindex[i]].size())
				{
					id = i;
					return IN_PRISM;
				}

			}

			return OUT_PRISM;
		}

		static INDEX index;
		static std::vector<INDEX> recompute;
		recompute.clear();


		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump1 || prismindex[i] == jump2)
				continue;
			if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump1][0], cornerlist[jump1][1])) continue;
			if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump2][0], cornerlist[jump2][1])) continue;
			
			index.FACES.clear();
			tot = 0;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {


				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
				}
				else continue;

				ori =
					orient3D_TPI_postfilter(
						st,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);


				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}
			}
			if (ori == 1) continue;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
					continue;
				}


				ori =
					orient3D_TPI_postfilter(
						st,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);


				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}
			}
			if (ori == 1) continue;
			if (tot == halfspace[prismindex[i]].size())
			{
				id = i;
				return IN_PRISM;
			}

			if (ori != 1)
			{
				index.Pi = i;
				recompute.emplace_back(index);

			}
		}

		if (recompute.size() > 0)
		{
			// timer.start();
			bool premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],

				facet10[0], facet10[1], facet10[2],
				facet11[0], facet11[1], facet11[2],
				facet12[0], facet12[1], facet12[2],

				facet20[0], facet20[1], facet20[2],
				facet21[0], facet21[1], facet21[2],
				facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return 2;
			for (int k = 0; k < recompute.size(); k++)
			{
				int in1 = prismindex[recompute[k].Pi];

				for (int j = 0; j < recompute[k].FACES.size(); j++) {
					int in2 = recompute[k].FACES[j];

					ori = orient3D_TPI_post_exact(s,
						halfspace[in1][in2][0][0], halfspace[in1][in2][0][1], halfspace[in1][in2][0][2],
						halfspace[in1][in2][1][0], halfspace[in1][in2][1][1], halfspace[in1][in2][1][2],
						halfspace[in1][in2][2][0], halfspace[in1][in2][2][1], halfspace[in1][in2][2][2]
					);

					if (ori == 1 || ori == 0)	break;
				}
				if (ori == -1) {
			
					id = recompute[k].Pi;
					return IN_PRISM;
				}
			}
		}
		return OUT_PRISM;

	}



	int FastEnvelope::Implicit_Tri_Facet_Facet_interpoint_Out_Prism_return_local_id_with_face_order_jump_over(
		const std::array<Vector3, 3> &triangle,
		const Vector3 &facet10, const Vector3 &facet11, const Vector3 &facet12, const Vector3 &facet20, const Vector3 &facet21, const Vector3 &facet22,
		const std::vector<unsigned int> &prismindex, const std::vector<std::vector<int>>&intersect_face,const std::vector<bool>& coverlist, const int &jump1, const int &jump2,
		int &id) const {

		TPI_exact_suppvars s;
		TPI_filtered_suppvars st;
		
		int tot, ori, fid;
		bool pre = orient3D_TPI_prefilter(triangle[0][0], triangle[0][1], triangle[0][2],
			triangle[1][0], triangle[1][1], triangle[1][2], triangle[2][0], triangle[2][1], triangle[2][2],
			facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
			facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
			st);

		if (pre == false) {
			bool premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2], triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return 2;
			for (int i = 0; i < prismindex.size(); i++)
			{

				if (prismindex[i] == jump1 || prismindex[i] == jump2)	continue;
				if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump1][0], cornerlist[jump1][1])) continue;
				if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump2][0], cornerlist[jump2][1])) continue;
				if (coverlist[i] == true) continue;
				tot = 0;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
					}
					else continue;
		
					ori = orient3D_TPI_post_exact(s,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);
	
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				fid = 0;
				for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {
					if (intersect_face[i][fid] == j)
					{
						if (fid + 1 < intersect_face[i].size()) fid++;
						continue;
					}

			
					ori = orient3D_TPI_post_exact(s,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);
		
					if (ori == 1 || ori == 0)
					{
						break;
					}

					if (ori == -1)
					{
						tot++;
					}
				}
				if (ori == 1 || ori == 0) continue;
				if (tot == halfspace[prismindex[i]].size())
				{
					id = i;
					return IN_PRISM;
				}

			}

			return OUT_PRISM;
		}

		static INDEX index;
		static std::vector<INDEX> recompute;
		recompute.clear();


		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump1 || prismindex[i] == jump2)
				continue;
			if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump1][0], cornerlist[jump1][1])) continue;
			if (!algorithms::box_box_intersection(cornerlist[prismindex[i]][0], cornerlist[prismindex[i]][1], cornerlist[jump2][0], cornerlist[jump2][1])) continue;
			if (coverlist[i] == true) continue;
			index.FACES.clear();
			tot = 0;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {


				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
				}
				else continue;
		
				ori =
					orient3D_TPI_postfilter(
						st,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);
				
	
				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}
			}
			if (ori == 1) continue;
			fid = 0;
			for (int j = 0; j < halfspace[prismindex[i]].size(); j++) {

				if (intersect_face[i][fid] == j)
				{
					if (fid + 1 < intersect_face[i].size()) fid++;
					continue;
				}


				ori =
					orient3D_TPI_postfilter(
						st,
						halfspace[prismindex[i]][j][0][0], halfspace[prismindex[i]][j][0][1], halfspace[prismindex[i]][j][0][2],
						halfspace[prismindex[i]][j][1][0], halfspace[prismindex[i]][j][1][1], halfspace[prismindex[i]][j][1][2],
						halfspace[prismindex[i]][j][2][0], halfspace[prismindex[i]][j][2][1], halfspace[prismindex[i]][j][2][2]);

		
				if (ori == 1)
				{
					break;
				}
				if (ori == 0)
				{
					index.FACES.emplace_back(j);
				}

				else if (ori == -1)
				{
					tot++;
				}
			}
			if (ori == 1) continue;
			if (tot == halfspace[prismindex[i]].size())
			{
				id = i;
				return IN_PRISM;
			}

			if (ori != 1)
			{
				index.Pi = i;
				recompute.emplace_back(index);

			}
		}
	
		if (recompute.size() > 0)
		{
			// timer.start();
			bool premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],

				facet10[0], facet10[1], facet10[2],
				facet11[0], facet11[1], facet11[2],
				facet12[0], facet12[1], facet12[2],

				facet20[0], facet20[1], facet20[2],
				facet21[0], facet21[1], facet21[2],
				facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return 2;
			for (int k = 0; k < recompute.size(); k++)
			{
				int in1 = prismindex[recompute[k].Pi];

				for (int j = 0; j < recompute[k].FACES.size(); j++) {
					int in2 = recompute[k].FACES[j];

					ori = orient3D_TPI_post_exact(s,
						halfspace[in1][in2][0][0], halfspace[in1][in2][0][1], halfspace[in1][in2][0][2],
						halfspace[in1][in2][1][0], halfspace[in1][in2][1][1], halfspace[in1][in2][1][2],
						halfspace[in1][in2][2][0], halfspace[in1][in2][2][1], halfspace[in1][in2][2][2]
					);

					if (ori == 1 || ori == 0)	break;
				}
				if (ori == -1) {
					
					id = recompute[k].Pi;
					return IN_PRISM;
				}
			}
		}

		return OUT_PRISM;

	}

	bool FastEnvelope::is_3_triangle_cut_pure_multiprecision(const std::array<Vector3, 3> &triangle, TPI_exact_suppvars &s)
	{
		int o1, o2, o3;
		Vector3 n = (triangle[0] - triangle[1]).cross(triangle[0] - triangle[2]) + triangle[0];

		if (Predicates::orient_3d(n, triangle[0], triangle[1], triangle[2]) == 0)
		{
			logger().debug("Degeneration happens");
			n = { {Vector3(rand(), rand(), rand())} };
		}
		static Scalar
			t00,
			t01, t02,
			t10, t11, t12,
			t20, t21, t22;

		t00 = triangle[0][0];
		t01 = triangle[0][1];
		t02 = triangle[0][2];
		t10 = triangle[1][0];
		t11 = triangle[1][1];
		t12 = triangle[1][2];
		t20 = triangle[2][0];
		t21 = triangle[2][1];
		t22 = triangle[2][2];
		
		o1 = orient3D_TPI_post_exact(
			s,
			n[0], n[1], n[2],
			t00, t01, t02,
			t10, t11, t12);
		
		if (o1 == 0)
			return false;
		
		o2 = orient3D_TPI_post_exact(
			s,
			n[0], n[1], n[2],
			t10, t11, t12,
			t20, t21, t22);
		
		if (o2 == 0 || o1 + o2 == 0)
			return false;

		o3 = orient3D_TPI_post_exact(
			s,
			n[0], n[1], n[2],
			t20, t21, t22,
			t00, t01, t02);

		if (o3 == 0 || o1 + o3 == 0 || o2 + o3 == 0)
			return false;
		return true;
	}


	bool FastEnvelope::is_3_triangle_cut(
		const std::array<Vector3, 3> &triangle,
		const Vector3 &facet10, const Vector3 &facet11, const Vector3 &facet12, const Vector3 &facet20, const Vector3 &facet21, const Vector3 &facet22)
	{
		TPI_filtered_suppvars st;
		TPI_exact_suppvars s;
		static Scalar
			t00, t01, t02,
			t10, t11, t12,
			t20, t21, t22,

			f100, f101, f102,
			f110, f111, f112,
			f120, f121, f122,

			f200, f201, f202,
			f210, f211, f212,
			f220, f221, f222;
		bool pre =
			orient3D_TPI_prefilter(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				st);
		if (pre == false) {
			bool premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return false;
			return is_3_triangle_cut_pure_multiprecision(triangle, s);
		}

		Vector3 n = (triangle[0] - triangle[1]).cross(triangle[0] - triangle[2]) + triangle[0];
		if (Predicates::orient_3d(n, triangle[0], triangle[1], triangle[2]) == 0)
		{
			logger().debug("Degeneration happens");
			n = { {Vector3(rand(), rand(), rand())} };
		}


		bool premulti = false;
		int o1 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			triangle[0][0], triangle[0][1], triangle[0][2],
			triangle[1][0], triangle[1][1], triangle[1][2]);
		if (o1 == 0)
		{

			t00 = (triangle[0][0]);
			t01 = (triangle[0][1]);
			t02 = (triangle[0][2]);
			t10 = (triangle[1][0]);
			t11 = (triangle[1][1]);
			t12 = (triangle[1][2]);
			t20 = (triangle[2][0]);
			t21 = (triangle[2][1]);
			t22 = (triangle[2][2]);

			f100 = (facet10[0]);
			f101 = (facet10[1]);
			f102 = (facet10[2]);
			f110 = (facet11[0]);
			f111 = (facet11[1]);
			f112 = (facet11[2]);
			f120 = (facet12[0]);
			f121 = (facet12[1]);
			f122 = (facet12[2]);

			f200 = (facet20[0]);
			f201 = (facet20[1]);
			f202 = (facet20[2]);
			f210 = (facet21[0]);
			f211 = (facet21[1]);
			f212 = (facet21[2]);
			f220 = (facet22[0]);
			f221 = (facet22[1]);
			f222 = (facet22[2]);

		
			premulti = orient3D_TPI_pre_exact(
				t00, t01, t02, t10, t11, t12, t20, t21, t22,
				f100, f101, f102, f110, f111, f112, f120, f121, f122,
				f200, f201, f202, f210, f211, f212, f220, f221, f222,
				s);

			o1 = orient3D_TPI_post_exact(
				s,
				n[0], n[1], n[2],
				t00, t01, t02,
				t10, t11, t12);
		}

		if (o1 == 0)
			return false;

		int o2 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			triangle[1][0], triangle[1][1], triangle[1][2],
			triangle[2][0], triangle[2][1], triangle[2][2]);
		if (o2 == 0)
		{
			if (premulti == false)
			{
				t00 = (triangle[0][0]);
				t01 = (triangle[0][1]);
				t02 = (triangle[0][2]);
				t10 = (triangle[1][0]);
				t11 = (triangle[1][1]);
				t12 = (triangle[1][2]);
				t20 = (triangle[2][0]);
				t21 = (triangle[2][1]);
				t22 = (triangle[2][2]);

				f100 = (facet10[0]);
				f101 = (facet10[1]);
				f102 = (facet10[2]);
				f110 = (facet11[0]);
				f111 = (facet11[1]);
				f112 = (facet11[2]);
				f120 = (facet12[0]);
				f121 = (facet12[1]);
				f122 = (facet12[2]);

				f200 = (facet20[0]);
				f201 = (facet20[1]);
				f202 = (facet20[2]);
				f210 = (facet21[0]);
				f211 = (facet21[1]);
				f212 = (facet21[2]);
				f220 = (facet22[0]);
				f221 = (facet22[1]);
				f222 = (facet22[2]);

				// timer.start();
				premulti = orient3D_TPI_pre_exact(
					t00, t01, t02, t10, t11, t12, t20, t21, t22,
					f100, f101, f102, f110, f111, f112, f120, f121, f122,
					f200, f201, f202, f210, f211, f212, f220, f221, f222,
					s);
			}
			o2 = orient3D_TPI_post_exact(
				s,
				n[0], n[1], n[2],
				t10, t11, t12,
				t20, t21, t22);

		}
		if (o2 == 0 || o1 + o2 == 0)
			return false;

		int o3 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			triangle[2][0], triangle[2][1], triangle[2][2],
			triangle[0][0], triangle[0][1], triangle[0][2]);
		if (o3 == 0)
		{
			if (premulti == false)
			{
				t00 = (triangle[0][0]);
				t01 = (triangle[0][1]);
				t02 = (triangle[0][2]);
				t10 = (triangle[1][0]);
				t11 = (triangle[1][1]);
				t12 = (triangle[1][2]);
				t20 = (triangle[2][0]);
				t21 = (triangle[2][1]);
				t22 = (triangle[2][2]);

				f100 = (facet10[0]);
				f101 = (facet10[1]);
				f102 = (facet10[2]);
				f110 = (facet11[0]);
				f111 = (facet11[1]);
				f112 = (facet11[2]);
				f120 = (facet12[0]);
				f121 = (facet12[1]);
				f122 = (facet12[2]);

				f200 = (facet20[0]);
				f201 = (facet20[1]);
				f202 = (facet20[2]);
				f210 = (facet21[0]);
				f211 = (facet21[1]);
				f212 = (facet21[2]);
				f220 = (facet22[0]);
				f221 = (facet22[1]);
				f222 = (facet22[2]);

				premulti = orient3D_TPI_pre_exact(
					t00, t01, t02, t10, t11, t12, t20, t21, t22,
					f100, f101, f102, f110, f111, f112, f120, f121, f122,
					f200, f201, f202, f210, f211, f212, f220, f221, f222,
					s);
			}
			o3 = orient3D_TPI_post_exact(
				s,
				n[0], n[1], n[2],
				t20, t21, t22,
				t00, t01, t02);

		}
		if (o3 == 0 || o1 + o3 == 0 || o2 + o3 == 0)
			return false;

		return true;
	}


	int FastEnvelope::is_3_triangle_cut_float_fast(
		const Vector3 &tri0, const Vector3 &tri1, const Vector3 &tri2,
		const Vector3 &facet10, const Vector3 &facet11, const Vector3 &facet12,
		const Vector3 &facet20, const Vector3 &facet21, const Vector3 &facet22)
	{

		Vector3 n = (tri0 - tri1).cross(tri0 - tri2) + tri0;

		if (Predicates::orient_3d(n, tri0, tri1, tri2) == 0)
		{
			logger().debug("Degeneration happens !");

			n = { {Vector3(rand(), rand(), rand())} };
		}
		TPI_filtered_suppvars st;
		bool pre =
			orient3D_TPI_prefilter(
				tri0[0], tri0[1], tri0[2],
				tri1[0], tri1[1], tri1[2],
				tri2[0], tri2[1], tri2[2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				st);

		if (pre == false)
			return 2; // means we dont know
		int o1 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			tri0[0], tri0[1], tri0[2],
			tri1[0], tri1[1], tri1[2]);
		int o2 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			tri1[0], tri1[1], tri1[2],
			tri2[0], tri2[1], tri2[2]);
		if (o1 * o2 == -1)
			return 0;
		int o3 = orient3D_TPI_postfilter(st, n[0], n[1], n[2],
			tri2[0], tri2[1], tri2[2],
			tri0[0], tri0[1], tri0[2]);
		if (o1 * o3 == -1 || o2 * o3 == -1)
			return 0;
		if (o1 * o2 * o3 == 0)
			return 2; // means we dont know
		return 1;
	}

	

	int FastEnvelope::is_triangle_cut_envelope_polyhedra(const int &cindex,
		const Vector3 &tri0, const Vector3 &tri1, const Vector3 &tri2, std::vector<int> &cid) const
	{

		cid.clear();
		cid.reserve(3);
		std::vector<bool> cut;
		cut.resize(halfspace[cindex].size());
		for (int i = 0; i < halfspace[cindex].size(); i++)
		{
			cut[i] = false;
		}
		std::vector<int> o1, o2, o3, cutp;
		o1.resize(halfspace[cindex].size());
		o2.resize(halfspace[cindex].size());
		o3.resize(halfspace[cindex].size());
		int  ori = 0, ct1 = 0, ct2 = 0, ct3 = 0;


		for (int i = 0; i < halfspace[cindex].size(); i++)
		{

			o1[i] = Predicates::orient_3d(tri0, halfspace[cindex][i][0], halfspace[cindex][i][1], halfspace[cindex][i][2]);
			o2[i] = Predicates::orient_3d(tri1, halfspace[cindex][i][0], halfspace[cindex][i][1], halfspace[cindex][i][2]);
			o3[i] = Predicates::orient_3d(tri2, halfspace[cindex][i][0], halfspace[cindex][i][1], halfspace[cindex][i][2]);
			if (o1[i] + o2[i] + o3[i] >= 3)
			{
				return 0;
			}
			if (o1[i] == -1) ct1++;
			if (o2[i] == -1) ct2++;
			if (o3[i] == -1) ct3++;
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 1)
			{
				return 0;
			}
			if (o1[i] == 1 && o2[i] == 0 && o3[i] == 0)
			{
				return 0;
			}
			if (o1[i] == 0 && o2[i] == 1 && o3[i] == 0)
			{
				return 0;
			}
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 0)
			{
				return 0;
			}

			if (o1[i] * o2[i] == -1 || o1[i] * o3[i] == -1 || o3[i] * o2[i] == -1)
				cutp.emplace_back(i);
		}
		if (cutp.size() == 0) {
			if (ct1 == halfspace[cindex].size()|| ct2 == halfspace[cindex].size()|| ct3 == halfspace[cindex].size()) {
				return 2;
			}
		}

		if (cutp.size() == 0)
		{
			return 0;
		}

		LPI_filtered_suppvars sl;
		std::array<Scalar, 2> seg00, seg01, seg02, seg10, seg11, seg12;
		for (int i = 0; i < cutp.size(); i++)
		{
			int temp = 0;
			if (o1[cutp[i]] * o2[cutp[i]] == -1) {
				seg00[temp] = tri0[0];
				seg01[temp] = tri0[1];
				seg02[temp] = tri0[2];
				seg10[temp] = tri1[0];
				seg11[temp] = tri1[1];
				seg12[temp] = tri1[2];
				temp++;
			}
			if (o1[cutp[i]] * o3[cutp[i]] == -1) {
				seg00[temp] = tri0[0];
				seg01[temp] = tri0[1];
				seg02[temp] = tri0[2];
				seg10[temp] = tri2[0];
				seg11[temp] = tri2[1];
				seg12[temp] = tri2[2];
				temp++;
			}
			if (o2[cutp[i]] * o3[cutp[i]] == -1) {
				seg00[temp] = tri1[0];
				seg01[temp] = tri1[1];
				seg02[temp] = tri1[2];
				seg10[temp] = tri2[0];
				seg11[temp] = tri2[1];
				seg12[temp] = tri2[2];
				temp++;
			}

			for (int k = 0; k < 2; k++)
			{

				bool precom = orient3D_LPI_prefilter( // it is boolean maybe need considering
					seg00[k], seg01[k], seg02[k],
					seg10[k], seg11[k], seg12[k],
					halfspace[cindex][cutp[i]][0][0], halfspace[cindex][cutp[i]][0][1], halfspace[cindex][cutp[i]][0][2],
					halfspace[cindex][cutp[i]][1][0], halfspace[cindex][cutp[i]][1][1], halfspace[cindex][cutp[i]][1][2],
					halfspace[cindex][cutp[i]][2][0], halfspace[cindex][cutp[i]][2][1], halfspace[cindex][cutp[i]][2][2],
					sl);
				if (precom == false)
				{
					cut[cutp[i]] = true;
					break;
				}
				for (int j = 0; j < cutp.size(); j++)
				{
					if (i == j)
						continue;
					ori =
						orient3D_LPI_postfilter(
							sl,
							seg00[k], seg01[k], seg02[k],
							halfspace[cindex][cutp[j]][0][0], halfspace[cindex][cutp[j]][0][1], halfspace[cindex][cutp[j]][0][2],
							halfspace[cindex][cutp[j]][1][0], halfspace[cindex][cutp[j]][1][1], halfspace[cindex][cutp[j]][1][2],
							halfspace[cindex][cutp[j]][2][0], halfspace[cindex][cutp[j]][2][1], halfspace[cindex][cutp[j]][2][2]);

					if (ori == 1)
						break;
				}
				if (ori != 1)
				{
					cut[cutp[i]] = true;
					break;
				}
			}

			ori = 0;// initialize the orientation to avoid the j loop doesn't happen because cutp.size()==1
		}

		if (cutp.size() <= 2)
		{
			for (int i = 0; i < halfspace[cindex].size(); i++)
			{
				if (cut[i] == true)
					cid.emplace_back(i);
			}
			return 1;
		}
		// triangle-facet-facet intersection
		TPI_filtered_suppvars st;

		for (int i = 0; i < cutp.size(); i++)
		{
			for (int j = i + 1; j < cutp.size(); j++)// two facets and the triangle generate a point
			{
				if (cut[cutp[i]] == true && cut[cutp[j]] == true)
					continue;

				bool neib = is_two_facets_neighbouring(cindex, cutp[i], cutp[j]);
				if (neib == false) continue;


				int inter = is_3_triangle_cut_float_fast(
					tri0, tri1, tri2,
					halfspace[cindex][cutp[i]][0],
					halfspace[cindex][cutp[i]][1],
					halfspace[cindex][cutp[i]][2],
					halfspace[cindex][cutp[j]][0],
					halfspace[cindex][cutp[j]][1],
					halfspace[cindex][cutp[j]][2]);
				if (inter == 2)
				{ //we dont know if point exist or if inside of triangle
					cut[cutp[i]] = true;
					cut[cutp[j]] = true;
					continue;
				}
				if (inter == 0)
					continue; // sure not inside

				bool pre =
					orient3D_TPI_prefilter(
						tri0[0], tri0[1], tri0[2],
						tri1[0], tri1[1], tri1[2],
						tri2[0], tri2[1], tri2[2],
						halfspace[cindex][cutp[i]][0][0], halfspace[cindex][cutp[i]][0][1], halfspace[cindex][cutp[i]][0][2],
						halfspace[cindex][cutp[i]][1][0], halfspace[cindex][cutp[i]][1][1], halfspace[cindex][cutp[i]][1][2],
						halfspace[cindex][cutp[i]][2][0], halfspace[cindex][cutp[i]][2][1], halfspace[cindex][cutp[i]][2][2],
						halfspace[cindex][cutp[j]][0][0], halfspace[cindex][cutp[j]][0][1], halfspace[cindex][cutp[j]][0][2],
						halfspace[cindex][cutp[j]][1][0], halfspace[cindex][cutp[j]][1][1], halfspace[cindex][cutp[j]][1][2],
						halfspace[cindex][cutp[j]][2][0], halfspace[cindex][cutp[j]][2][1], halfspace[cindex][cutp[j]][2][2],
						st);

				for (int k = 0; k < cutp.size(); k++)
				{

					if (k == i || k == j)
						continue;

					ori =
						orient3D_TPI_postfilter(st,
							halfspace[cindex][cutp[k]][0][0], halfspace[cindex][cutp[k]][0][1], halfspace[cindex][cutp[k]][0][2],
							halfspace[cindex][cutp[k]][1][0], halfspace[cindex][cutp[k]][1][1], halfspace[cindex][cutp[k]][1][2],
							halfspace[cindex][cutp[k]][2][0], halfspace[cindex][cutp[k]][2][1], halfspace[cindex][cutp[k]][2][2]);

					if (ori == 1)
						break;
				}

				if (ori != 1)
				{
					cut[cutp[i]] = true;
					cut[cutp[j]] = true;
				}
			}
		}

		for (int i = 0; i < halfspace[cindex].size(); i++)
		{
			if (cut[i] == true)
				cid.emplace_back(i);
		}

		return 1;
	}
	bool FastEnvelope::is_tpp_on_polyhedra(
		const std::array<Vector3, 3> &triangle,
		const Vector3 &facet10, const Vector3 &facet11, const Vector3 &facet12, const Vector3 &facet20, const Vector3 &facet21, const Vector3 &facet22,
		const int &prismid, const int &faceid)const {
		int ori;
		TPI_filtered_suppvars st;
		TPI_exact_suppvars s;
		bool premulti = false;
		bool pre =
			orient3D_TPI_prefilter(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2],
				facet11[0], facet11[1], facet11[2],
				facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2],
				facet21[0], facet21[1], facet21[2],
				facet22[0], facet22[1], facet22[2],
				st);
		if (pre == true) {
			for (int i = 0; i < halfspace[prismid].size(); i++) {
				/*bool neib = is_two_facets_neighbouring(prismid, i, faceid);// this works only when the polyhedron is convax and no two neighbour facets are coplanar
				if (neib == false) continue;*/
				if (i == faceid) continue;
				ori =
					orient3D_TPI_postfilter(st,
						halfspace[prismid][i][0][0], halfspace[prismid][i][0][1], halfspace[prismid][i][0][2],
						halfspace[prismid][i][1][0], halfspace[prismid][i][1][1], halfspace[prismid][i][1][2],
						halfspace[prismid][i][2][0], halfspace[prismid][i][2][1], halfspace[prismid][i][2][2]);

				if (ori == 0) {
					if (premulti == false) {
						premulti = orient3D_TPI_pre_exact(
							triangle[0][0], triangle[0][1], triangle[0][2],
							triangle[1][0], triangle[1][1], triangle[1][2],
							triangle[2][0], triangle[2][1], triangle[2][2],

							facet10[0], facet10[1], facet10[2],
							facet11[0], facet11[1], facet11[2],
							facet12[0], facet12[1], facet12[2],

							facet20[0], facet20[1], facet20[2],
							facet21[0], facet21[1], facet21[2],
							facet22[0], facet22[1], facet22[2],
							s);
						if (premulti == false) return false;
					}
					ori = orient3D_TPI_post_exact(s,
						halfspace[prismid][i][0][0], halfspace[prismid][i][0][1], halfspace[prismid][i][0][2],
						halfspace[prismid][i][1][0], halfspace[prismid][i][1][1], halfspace[prismid][i][1][2],
						halfspace[prismid][i][2][0], halfspace[prismid][i][2][1], halfspace[prismid][i][2][2]);
				}
				
				if (ori == 1) return false;

			}

		}
		else {
			premulti = orient3D_TPI_pre_exact(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],

				facet10[0], facet10[1], facet10[2],
				facet11[0], facet11[1], facet11[2],
				facet12[0], facet12[1], facet12[2],

				facet20[0], facet20[1], facet20[2],
				facet21[0], facet21[1], facet21[2],
				facet22[0], facet22[1], facet22[2],
				s);
			if (premulti == false) return false;
			for (int i = 0; i < halfspace[prismid].size(); i++) {
				/*bool neib = is_two_facets_neighbouring(prismid, i, faceid);// this works only when the polyhedron is convax and no two neighbour facets are coplanar
				if (neib == false) continue;*/
				if (i == faceid) continue;
				ori = orient3D_TPI_post_exact(s,
					halfspace[prismid][i][0][0], halfspace[prismid][i][0][1], halfspace[prismid][i][0][2],
					halfspace[prismid][i][1][0], halfspace[prismid][i][1][1], halfspace[prismid][i][1][2],
					halfspace[prismid][i][2][0], halfspace[prismid][i][2][1], halfspace[prismid][i][2][2]);
				
				if (ori == 1) return false;

			}
		}
		return true;
	}

	bool FastEnvelope::is_seg_cut_polyhedra(const int &cindex,
		const Vector3 &seg0, const Vector3 &seg1, std::vector<int> &cid) const
	{


		cid.clear();
		std::vector<bool> cut;
		cut.resize(halfspace[cindex].size());
		for (int i = 0; i < halfspace[cindex].size(); i++)
		{
			cut[i] = false;
		}
		std::vector<int> o1, o2;
		o1.resize(halfspace[cindex].size());
		o2.resize(halfspace[cindex].size());
		int ori = 0, count = 0;
		std::vector<int> cutp;

		for (int i = 0; i < halfspace[cindex].size(); i++)
		{


			o1[i] = Predicates::orient_3d(seg0, halfspace[cindex][i][0], halfspace[cindex][i][1], halfspace[cindex][i][2]);
			o2[i] = Predicates::orient_3d(seg1, halfspace[cindex][i][0], halfspace[cindex][i][1], halfspace[cindex][i][2]);

			if (o1[i] + o2[i] >= 1)
			{
				return false;
			}

			if (o1[i] == 0 && o2[i] == 0)
			{
				return false;
			}

			if (o1[i] * o2[i] == -1)
				cutp.emplace_back(i);
			if (o1[i] == -1) count++;
		}
		if (cutp.size() == 0 && count == halfspace[cindex].size()) return true;
		if (cutp.size() == 0)
		{
			return false;
		}

		LPI_filtered_suppvars sf;
		for (int i = 0; i < cutp.size(); i++)
		{

			bool precom = orient3D_LPI_prefilter( // it is boolean maybe need considering
				seg0[0], seg0[1], seg0[2],
				seg1[0], seg1[1], seg1[2],
				halfspace[cindex][cutp[i]][0][0], halfspace[cindex][cutp[i]][0][1], halfspace[cindex][cutp[i]][0][2],
				halfspace[cindex][cutp[i]][1][0], halfspace[cindex][cutp[i]][1][1], halfspace[cindex][cutp[i]][1][2],
				halfspace[cindex][cutp[i]][2][0], halfspace[cindex][cutp[i]][2][1], halfspace[cindex][cutp[i]][2][2],
				sf);
			if (precom == false)
			{
				cut[cutp[i]] = true;
				continue;
			}
			for (int j = 0; j < cutp.size(); j++)
			{
				if (i == j)
					continue;

				ori =
					orient3D_LPI_postfilter(
						sf,
						seg0[0], seg0[1], seg0[2],
						halfspace[cindex][cutp[j]][0][0], halfspace[cindex][cutp[j]][0][1], halfspace[cindex][cutp[j]][0][2],
						halfspace[cindex][cutp[j]][1][0], halfspace[cindex][cutp[j]][1][1], halfspace[cindex][cutp[j]][1][2],
						halfspace[cindex][cutp[j]][2][0], halfspace[cindex][cutp[j]][2][1], halfspace[cindex][cutp[j]][2][2]);

				if (ori == 1)
					break;
			}
			if (ori != 1)
			{
				cut[cutp[i]] = true;
			}
		}

		for (int i = 0; i < halfspace[cindex].size(); i++)
		{
			if (cut[i] == true)
				cid.emplace_back(i);
		}

		return true;
	}
	bool FastEnvelope::point_out_prism(const Vector3 &point, const std::vector<unsigned int> &prismindex, const int &jump) const
	{

		int ori;

		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump)
				continue;

			for (int j = 0; j < halfspace[prismindex[i]].size(); j++)
			{

				ori = Predicates::orient_3d(halfspace[prismindex[i]][j][0], halfspace[prismindex[i]][j][1], halfspace[prismindex[i]][j][2], point);
				if (ori == -1 || ori == 0)
				{
					break;
				}
				if (j == halfspace[prismindex[i]].size() - 1)
				{

					return false;
				}
			}

		}

		return true;
	}
	bool FastEnvelope::point_out_prism_return_local_id(const Vector3 &point, const std::vector<unsigned int> &prismindex, const int &jump, int &id) const
	{

		int ori;

		for (int i = 0; i < prismindex.size(); i++)
		{
			if (prismindex[i] == jump)
				continue;

			for (int j = 0; j < halfspace[prismindex[i]].size(); j++)
			{

				ori = Predicates::orient_3d(halfspace[prismindex[i]][j][0], halfspace[prismindex[i]][j][1], halfspace[prismindex[i]][j][2], point);
				if (ori == -1 || ori == 0)
				{
					break;
				}

			}
			if (ori == 1)
			{
				id = i;
				return false;
			}

		}

		return true;
	}
	
	void FastEnvelope::halfspace_init(const std::vector<Vector3> &m_ver, const std::vector<Vector3i> &m_faces,
		std::vector<std::vector<std::array<Vector3, 3>>>& halfspace, std::vector<std::array<Vector3, 2>>& cornerlist, const Scalar &epsilon) {

		const auto dot_sign = [](const Vector3 &a, const Vector3 &b)
		{
			Scalar t = a.dot(b);
			if (t > SCALAR_ZERO)
				return 1;
			if (t < -1 * SCALAR_ZERO)
				return -1;
			Rational a0(a[0]), a1(a[1]), a2(a[2]), b0(b[0]), b1(b[1]), b2(b[2]), dot;
			dot = a0 * b0 + a1 * b1 + a2 * b2;
			if (dot.get_sign() > 0)
				return 1;
			if (dot.get_sign() < 0)
				return -1;
			return 0;
		};
		const auto get_bb_corners_12 = [](const std::array<Vector3, 12> &vertices) {//TODO why use this one
			std::array<Vector3, 2> corners;
			corners[0] = vertices[0];
			corners[1] = vertices[0];

			for (size_t j = 0; j < 12; j++) {
				for (int i = 0; i < 3; i++) {
					corners[0][i] = std::min(corners[0][i], vertices[j][i]);
					corners[1][i] = std::max(corners[1][i], vertices[j][i]);
				}
			}

			//const Scalar dis = (max - min).minCoeff() * 3;//TODO  change to 1e-5 or sth
			const Scalar dis = 1e-6;
			for (int j = 0; j < 3; j++) {
				corners[0][j] -= dis;
				corners[1][j] += dis;
			}
			return corners;

		};
		const auto get_bb_corners_8 = [](const std::array<Vector3, 8> &vertices) {//TODO why use this one
			std::array<Vector3, 2> corners;
			corners[0] = vertices[0];
			corners[1] = vertices[0];

			for (size_t j = 0; j < 8; j++) {
				for (int i = 0; i < 3; i++) {
					corners[0][i] = std::min(corners[0][i], vertices[j][i]);
					corners[1][i] = std::max(corners[1][i], vertices[j][i]);
				}
			}

			//const Scalar dis = (max - min).minCoeff() * 3;//TODO  change to 1e-5 or sth
			const Scalar dis = 1e-6;
			for (int j = 0; j < 3; j++) {
				corners[0][j] -= dis;
				corners[1][j] += dis;
			}
			return corners;
		};
		static const fastEnvelope::Vector3 origin = fastEnvelope::Vector3(0, 0, 0);
	
		halfspace.resize(m_faces.size());
		cornerlist.resize(m_faces.size());
		
		Vector3 AB, AC, BC, normal, vector1, ABn, min, max;
		std::array<Vector3, 6> polygon;
		std::array<Vector3, 12> polygonoff;
		std::array<Vector3, 8> box;
		static const std::array<Vector3, 8> boxorder = {
			{
				{1, 1, 1},
				{-1, 1, 1},
				{-1, -1, 1},
				{1, -1, 1},
				{1, 1, -1},
				{-1, 1, -1},
				{-1, -1, -1},
				{1, -1, -1},
			} };
		static const int p_face[8][3] = { {0, 1, 3}, {7, 6, 9}, {1, 0, 7}, {2, 1, 7}, {3, 2, 8}, {3, 9, 10}, {5, 4, 11}, {0, 5, 6} }; //prism triangle index. all with orientation.
		static const int c_face[6][3] = { {0, 1, 2}, {4, 7, 6}, {0, 3, 4}, {1, 0, 4}, {1, 5, 2}, {2, 6, 3} };
	//	static const std::array<std::vector<int>, 8> p_facepoint = {
	//	{{0,1,2,3,4,5},
	//{8,7,6,11,10,9},
	//{7,1,0,6},
	//{2,1,7,8},
	//{3,2,8,9},
	//{4,3,9,10},
	//{4,10,11,5},
	//{6,0,5,11}}
	//	};

	//	static const std::array<std::array<int, 4>, 6> c_facepoint = {
	//			{
	//				{0,1,2,3},
	//		{4,7,6,5},
	//		{4,0,3,7},
	//		{1,0,4,5},
	//		{2,1,5,6},
	//		{3,2,6,7}
	//			}
	//	};

		Scalar tolerance = epsilon / sqrt(3);
		Scalar de;

		for (int i = 0; i < m_faces.size(); i++)
		{
			AB = m_ver[m_faces[i][1]] - m_ver[m_faces[i][0]];
			AC = m_ver[m_faces[i][2]] - m_ver[m_faces[i][0]];
			BC = m_ver[m_faces[i][2]] - m_ver[m_faces[i][1]];
			de = algorithms::is_triangle_degenerated(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], m_ver[m_faces[i][2]]);

			if (de == DEGENERATED_POINT)
			{
				logger().debug("Envelope Triangle Degeneration- Point");
				for (int j = 0; j < 8; j++)
				{
					box[j] = m_ver[m_faces[i][0]] + boxorder[j] * tolerance;
				}
				halfspace[i].resize(6);
				for (int j = 0; j < 6; j++) {
					halfspace[i][j][0] = box[c_face[j][0]];
					halfspace[i][j][1] = box[c_face[j][1]];
					halfspace[i][j][2] = box[c_face[j][2]];
				}
				cornerlist[i] = (get_bb_corners_8(box));

				
				continue;
			}
			if (de == DEGENERATED_SEGMENT)
			{
				logger().debug("Envelope Triangle Degeneration- Segment");
				Scalar length1 = AB.norm(), length2 = AC.norm(), length3 = BC.norm();
				if (length1 >= length2 && length1 >= length3)
				{
					seg_cube(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], tolerance, box);

				}
				if (length2 >= length1 && length2 >= length3)
				{
					seg_cube(m_ver[m_faces[i][0]], m_ver[m_faces[i][2]], tolerance, box);

				}
				if (length3 >= length1 && length3 >= length2)
				{
					seg_cube(m_ver[m_faces[i][1]], m_ver[m_faces[i][2]], tolerance, box);
				}
				halfspace[i].resize(6);
				for (int j = 0; j < 6; j++) {
					halfspace[i][j][0] = box[c_face[j][0]];
					halfspace[i][j][1] = box[c_face[j][1]];
					halfspace[i][j][2] = box[c_face[j][2]];
				}
				cornerlist[i] = (get_bb_corners_8(box));


				continue;
			}
			if (de == NERLY_DEGENERATED)
			{
				logger().debug("Envelope Triangle Degeneration- Nearly");

				normal = algorithms::accurate_normal_vector(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], m_ver[m_faces[i][0]], m_ver[m_faces[i][2]]);

				vector1 = algorithms::accurate_normal_vector(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], origin, normal);

			}
			else
			{
				normal = AB.cross(AC).normalized();
				vector1 = AB.cross(normal).normalized();
			}

			ABn = AB.normalized();
			polygon[0] = m_ver[m_faces[i][0]] + (vector1 - ABn) * tolerance;
			polygon[1] = m_ver[m_faces[i][1]] + (vector1 + ABn) * tolerance;
			if (dot_sign(AB, BC) < 0)
			{
				polygon[2] = m_ver[m_faces[i][1]] + (-vector1 + ABn) * tolerance;
				polygon[3] = m_ver[m_faces[i][2]] + (-vector1 + ABn) * tolerance;
				polygon[4] = m_ver[m_faces[i][2]] + (-vector1 - ABn) * tolerance;
				if (dot_sign(AB, AC) < 0)
				{
					polygon[5] = m_ver[m_faces[i][2]] + (vector1 - ABn) * tolerance;
				}
				else
				{
					polygon[5] = m_ver[m_faces[i][0]] + (-vector1 - ABn) * tolerance;
				}
			}
			else
			{
				polygon[2] = m_ver[m_faces[i][2]] + (vector1 + ABn) * tolerance;
				polygon[3] = m_ver[m_faces[i][2]] + (-vector1 + ABn) * tolerance;
				polygon[4] = m_ver[m_faces[i][2]] + (-vector1 - ABn) * tolerance;
				polygon[5] = m_ver[m_faces[i][0]] + (-vector1 - ABn) * tolerance;
			}

			for (int j = 0; j < 6; j++)
			{
				polygonoff[j] = polygon[j] + normal * tolerance;
			}
			for (int j = 6; j < 12; j++)
			{
				polygonoff[j] = polygon[j - 6] - normal * tolerance;
			}
			halfspace[i].resize(8);
			for (int j = 0; j < 8; j++) {
				halfspace[i][j][0] = polygonoff[p_face[j][0]];
				halfspace[i][j][1] = polygonoff[p_face[j][1]];
				halfspace[i][j][2] = polygonoff[p_face[j][2]];
			}
			cornerlist[i] = (get_bb_corners_12(polygonoff));

		}

		
	}

	void FastEnvelope::seg_cube(const Vector3 &p1, const Vector3 &p2, const Scalar &width, std::array<Vector3, 8> &envbox)
	{
		Vector3 v1, v2, v = p2 - p1; //p12
		if (v[0] != 0)
		{
			v1 = Vector3((0 - v[1] - v[2]) / v[0], 1, 1);
		}
		else
		{
			if (v[1] != 0)
			{
				v1 = Vector3(1, (0 - v[2]) / v[1], 1);
			}
			else
			{
				v1 = Vector3(1, 1, 0);
			}
		}
		v2 = v.cross(v1);
		v = v.normalized();
		v1 = v1.normalized();
		v2 = v2.normalized();
		envbox[0] = p2 + width * (v + v1 + v2);
		envbox[1] = p2 + width * (v - v1 + v2);
		envbox[2] = p2 + width * (v - v1 - v2);
		envbox[3] = p2 + width * (v + v1 - v2); //right hand out direction
		envbox[4] = p1 + width * (-v + v1 + v2);
		envbox[5] = p1 + width * (-v - v1 + v2);
		envbox[6] = p1 + width * (-v - v1 - v2);
		envbox[7] = p1 + width * (-v + v1 - v2); //right hand in direction
	}

	
	bool FastEnvelope::is_two_facets_neighbouring(const int & pid, const int &i, const int &j)const {
		static const int prism_map[64][2] = {
	{-1, -1},
	{-1, -1},
	{0, 1},
	{1, 2},
	{2, 3},
	{3, 4},
	{4, 5},
	{0, 5},
	{-1, -1},
	{-1, -1},
	{6, 7},
	{7, 8},
	{8, 9},
	{9, 10},
	{10, 11},
	{6, 11},
	{0, 1},
	{6, 7},
	{-1, -1},
	{1, 7},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{0, 6},
	{1, 2},
	{7, 8},
	{1, 7},
	{-1, -1},
	{2, 8},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{2, 3},
	{8, 9},
	{-1, -1},
	{2, 8},
	{-1, -1},
	{3, 9},
	{-1, -1},
	{-1, -1},
	{3, 4},
	{9, 10},
	{-1, -1},
	{-1, -1},
	{3, 9},
	{-1, -1},
	{4, 10},
	{-1, -1},
	{4, 5},
	{10, 11},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{4, 10},
	{-1, -1},
	{5, 11},
	{0, 5},
	{6, 11},
	{0, 6},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{5, 11},
	{-1, -1} };
		static const int cubic_map[36][2] = {
			{-1, -1},
		 {-1, -1},
		 {0, 3},
		 {0, 1},
		 {1, 2},
		 {2, 3},
		 {-1, -1},
		 {-1, -1},
		 {4, 7},
		 {4, 5},
		 {5, 6},
		 {6, 7},
		 {0, 3},
		 {4, 7},
		 {-1, -1},
		 {0, 4},
		 {-1, -1},
		 {3, 7},
		 {0, 1},
		 {4, 5},
		 {0, 4},
		 {-1, -1},
		 {1, 5},
		 {-1, -1},
		 {1, 2},
		 {5, 6},
		 {-1, -1},
		 {1, 5},
		 {-1, -1},
		 {2, 6},
		 {2, 3},
		 {6, 7},
		 {3, 7},
		 {-1, -1},
		 {2, 6},
		 {-1, -1}
		};
		if (halfspace[pid].size() == 8) {
			int id = i * 8 + j;
			if (prism_map[id][0] == -1) return false;
		}
		if (halfspace[pid].size() == 6) {
			int id = i * 6 + j;
			if (cubic_map[id][0] == -1) return false;
		}
		return true;
	}

}
