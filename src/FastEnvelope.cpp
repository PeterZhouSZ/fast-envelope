#include <fastenvelope/FastEnvelope.h>
#include <fastenvelope/Parameters.h>
#include <fastenvelope/Predicates.hpp>
#include <fstream>
#include <istream>
#include <igl/Timer.h>
#include <fastenvelope/ip_filtered.h>
#include <arbitraryprecision/fprecision.h>
#include <fastenvelope/Rational.hpp>
#include <igl/Timer.h>


static const int p_face[8][3] = { {0,1,3},{7,6,9},{1,0,7},{2,1,7},{3,2,8},{3,9,10},{5,4,11},{0,5,6} };//prism triangle index. all with orientation.
static const int c_face[6][3] = { {0,1,2},{4,7,6},{0,3,4},{1,0,4},{1,5,2},{2,6,3} };

static const std::array<std::vector<int>, 8> p_facepoint = {
		{
			{0,1,2,3,4,5},
	{8,7,6,11,10,9},
	{7,1,0,6},
	{2,1,7,8},
	{3,2,8,9},
	{4,3,9,10},
	{4,10,11,5},
	{6,0,5,11}

		}
};
static const std::array<std::array<int, 4>, 6> c_facepoint = {
		{
			{0,1,2,3},
	{4,7,6,5},
	{4,0,3,7},
	{1,0,4,5},
	{2,1,5,6},
	{3,2,6,7}


		}
};
static const int p_facenumber = 8;
static const int c_facenumber = 6;

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
 {-1, -1}
};

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


static const   std::function<int(double)> check_double = [](double v) {
	if (fabs(v) < 1e-10)
		return -2;

	if (v > 0)
		return 1;

	if (v < 0)
		return -1;

	return 0;
};

static const   std::function<int(fastEnvelope::Rational)> check_Rational = [](fastEnvelope::Rational v) {
	//todo: CHECK
	return v.get_sign();
	// if (v > 0)
	// 	return 1;

	// if (v < 0)
	// 	return -1;

	// return 0;
};

static const   std::function<int(fastEnvelope::Multiprecision)> check_Multiprecision = [](fastEnvelope::Multiprecision v) {
	//todo: CHECK
	return v.get_sign();
	// if (v > 0)
	// 	return 1;

	// if (v < 0)
	// 	return -1;

	// return 0;
};




static const std::array<std::array<int, 2>, 3> triseg = {

	{{{0,1}},{{0,2}},{{1,2}}}

};



namespace fastEnvelope {
	//using namespace std;

	void FastEnvelope::print_number() {
		//std::cout << "lpi filter number " << filternumberlpi << " lpi total number " << totalnumberlpi << " percentage " << float(filternumberlpi )/ float(totalnumberlpi) << std::endl;
		//std::cout << "tpi filter number " << filternumbertpi << " tpi total number " << totalnumbertpi << " percentage " << float(filternumbertpi) / float(totalnumbertpi) << std::endl;
		//std::cout << "triangle_intersection filter number " << filternumber1 << " tpi total number " << totalnumber1 << " percentage " << float(filternumber1) / float(totalnumber1) << std::endl;
		//std::cout << "triangle_intersection filter number lpi -2 " << filternumberlpi2 << " percentage " << float(filternumberlpi2) / float(totalnumberlpi) << std::endl;
		//std::cout << "triangle_intersection filter number tpi -2 " << filternumbertpi2 << " percentage " << float(filternumbertpi2) / float(totalnumbertpi+ totalnumber1) << std::endl;
		/*std::cout << "lpi 1 " << float(after11) / float(after11 + after12 + after10) << " lpi -1 " << after12 / float(after11 + after12 + after10) << " lpi 0 " << after10 / float(after11 + after12 + after10) << " tot  " << after11 + after12 + after10 << std::endl;
		std::cout << "tpi 1 " << after21 / float(after21 + after22 + after20) << " tpi -1 " << after22 / float(after21 + after22 + after20) << " tpi 0 " << after20 / float(after21 + after22 + after20) << " tot  " << after21 + after22 + after20 << std::endl;
		std::cout << "go1 " << go1 << " go2 " << go2 << std::endl;*/
		//std::cout << "same " << float(diff1) / float(diff1 + diff2 + diff3) << " diff " << float(diff2) / float(diff1 + diff2 + diff3) << " wrong " << float(diff3) / float(diff1 + diff2 + diff3) << std::endl;
	//	std::cout << "cut tri number original " << ct1 << " conservative " << ct2  <<" rate "<<float(ct1)/float(ct2)<< std::endl;
		//std::cout << "total " <<diff1+diff2+diff3 << "   " << ct1 << "  " << ct2 << std::endl;

	}

	FastEnvelope::FastEnvelope(const std::vector<Vector3>& m_ver, const std::vector<Vector3i>& m_faces, const Scalar eps, const int spac)
	{
		
		//Multiprecision::set_precision(256);

		get_bb_corners(m_ver, min, max);
		Scalar bbd = (max - min).norm();
		Scalar epsilon = bbd * eps; //eps*bounding box diagnal
		BoxGeneration(m_ver, m_faces, envprism, envcubic, epsilon);
		//build a  hash function
		prism_size = envprism.size();


		const Scalar boxlength = std::min(std::min(max[0] - min[0], max[1] - min[1]), max[2] - min[2]) / spac;
		subx = (max[0] - min[0]) / boxlength, suby = (max[1] - min[1]) / boxlength, subz = (max[2] - min[2]) / boxlength;


		CornerList_prism(envprism, cornerlist);
		std::vector<std::array<Vector3, 2>> cubiconors;
		CornerList_cubic(envcubic, cubiconors);
		cornerlist.insert(cornerlist.end(), cubiconors.begin(), cubiconors.end());
		std::vector<int> intercell;
		int ct = 0, prismsize = envprism.size();

		prismmap.reserve(spac*spac*spac / 10);
		for (int i = 0; i < cornerlist.size(); i++) {
			BoxFindCells(cornerlist[i][0], cornerlist[i][1], min, max, subx, suby, subz, intercell);
			for (int j = 0; j < intercell.size(); j++) {
				prismmap[intercell[j]].emplace_back(i);
			}
		}
		std::cout << "map size " << prismmap.size() << std::endl;
		std::cout << "prism size " << prism_size << std::endl;
		std::cout << "cubic size " << envcubic.size() << std::endl;
	}
	bool FastEnvelope::is_outside(const std::array<Vector3, 3> &triangle) const {
		Vector3 tmin, tmax;
		std::vector<int> inumber;
		std::vector<int> intercell;

		get_tri_corners(triangle[0],triangle[1],triangle[2], tmin, tmax);
		BoxFindCells(tmin, tmax, min, max, subx, suby, subz, intercell);

		for (int j = 0; j < intercell.size(); j++) {
			auto search = prismmap.find(intercell[j]);
			if (search != prismmap.end()) {
				inumber.insert(inumber.end(), search->second.begin(), search->second.end());
			}
		}
		sort(inumber.begin(), inumber.end());
		inumber.erase(unique(inumber.begin(), inumber.end()), inumber.end());


		return FastEnvelopeTestImplicit(triangle, inumber);
	}

	void FastEnvelope::print_prisms(const std::array<Vector3, 3> &triangle) const {

		Vector3 tmin, tmax;
		std::vector<int> inumber;
		std::vector<int> intercell;
		std::vector<std::array<Vector3, 12>> interenvprism;
		get_tri_corners(triangle[0], triangle[1], triangle[2], tmin, tmax);
		BoxFindCells(tmin, tmax, min, max, subx, suby, subz, intercell);
		inumber.clear();
		for (int j = 0; j < intercell.size(); j++) {
			auto search = prismmap.find(intercell[j]);
			if (search != prismmap.end()) {
				inumber.insert(inumber.end(), search->second.begin(), search->second.end());
			}
		}
		sort(inumber.begin(), inumber.end());
		inumber.erase(unique(inumber.begin(), inumber.end()), inumber.end());
		interenvprism.reserve(inumber.size());
		for (int j = 0; j < inumber.size(); j++) {
			interenvprism.emplace_back(envprism[inumber[j]]);
		}

		/*std::ofstream fout;
		fout.open("D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\visualprism.txt");
		for (int i = 0; i < interenvprism.size(); i++) {
			for (int j = 0; j < 12; j++) {

				fout << std::setprecision(17) << interenvprism[i][j][0] << " " << interenvprism[i][j][1] << " " << interenvprism[i][j][2] << std::endl;

			}
		}

		fout.close();*/
	}
	bool FastEnvelope::sample_triangle_outside(const std::array<Vector3, 3> &triangle, const int& pieces) const {


		bool out;
		Vector3 tmin, tmax, point;
		std::vector<int> inumber;
		std::vector<int> intercell;

		get_tri_corners(triangle[0], triangle[1], triangle[2], tmin, tmax);
		BoxFindCells(tmin, tmax, min, max, subx, suby, subz, intercell);
		inumber.clear();
		for (int j = 0; j < intercell.size(); j++) {
			auto search = prismmap.find(intercell[j]);
			if (search != prismmap.end()) {
				inumber.insert(inumber.end(), search->second.begin(), search->second.end());
			}
		}
		sort(inumber.begin(), inumber.end());
		inumber.erase(unique(inumber.begin(), inumber.end()), inumber.end());
		int deg = is_triangle_degenerated(triangle[0], triangle[1], triangle[2]);

		int jump = -1;
		if (deg == DEGENERATED_POINT) {
			out = point_out_prism(triangle[0], inumber, jump);
			if (out == true) {

				return 1;

			}
			return 0;
		}
		if (deg == DEGENERATED_SEGMENT) {
			for (int i = 0; i < pieces; i++) {
				triangle_sample_segment(triangle, point, pieces, i);
				out = point_out_prism(point, inumber, jump);
				if (out == true) {

					return 1;

				}

			}
			return 0;
		}


		for (int i = 0; i < pieces; i++) {
			for (int j = 0; j <= i; j++) {
				triangle_sample_normal(triangle, point, pieces, i, j);
				out = point_out_prism(point, inumber, jump);
				if (out == true) {

					return 1;

				}


			}

		}
		return 0;
	}

	void FastEnvelope::triangle_sample_segment(const std::array<Vector3, 3> &triangle, Vector3& ps, const int &pieces, const int & nbr) {

		int t = pieces-1;
		if (triangle[1] - triangle[0] == Vector3(0, 0, 0)) {

			ps = (triangle[0] + (triangle[2] - triangle[0])*nbr / t);

			return;
		}
		if (triangle[2] - triangle[0] == Vector3(0, 0, 0)) {


			ps = (triangle[0] + (triangle[1] - triangle[0])*nbr / t);

			return;
		}
		if (triangle[2] - triangle[1] == Vector3(0, 0, 0)) {

			ps = (triangle[0] + (triangle[1] - triangle[0])*nbr / t);

			return;
		}

		Scalar d1 = (triangle[1] - triangle[0]).norm(), d2 = (triangle[2] - triangle[0]).norm(), d3 = (triangle[1] - triangle[2]).norm();
		if (d1 >= d2 && d1 >= d3) {
			ps = (triangle[0] + (triangle[1] - triangle[0])*nbr / t);

			return;
		}
		if (d2 >= d1 && d2 >= d3) {
			ps = (triangle[0] + (triangle[2] - triangle[0])*nbr / t);

			return;
		}
		if (d3 >= d1 && d3 >= d2) {
			ps = (triangle[1] + (triangle[2] - triangle[1])*nbr / t);

			return;
		}
	}
	void  FastEnvelope::triangle_sample_point(const std::array<Vector3, 3> &triangle, Vector3& ps) {
		ps = triangle[0];
	}
	void FastEnvelope::triangle_sample_normal(const std::array<Vector3, 3> &triangle,Vector3& ps, const int &pieces, const int & nbr1, const int &nbr2) {
		int l1s = pieces - 1;//
		Vector3 p1 = triangle[0] + (triangle[1] - triangle[0])*nbr1 / l1s, d = (triangle[2] - triangle[1]) / l1s;
		ps = p1 + d * nbr2;

	}

	bool FastEnvelope::FastEnvelopeTestImplicit(const std::array<Vector3, 3> &triangle, const std::vector<int>& prismindex)const

	{

		static const std::function<int(fastEnvelope::Multiprecision)> checker = check_Multiprecision;
		static const std::function<int(fastEnvelope::Multiprecision)> checker1 = check_Multiprecision;
		if (prismindex.size() == 0) {

			return 1;

		}

		int jump1, jump2;

		std::vector<std::array<int,2>> inter_ijk_list;//list of intersected triangle

		bool out,cut;

		int inter, inter1, record1, record2,

			tti;//triangle-triangle intersection


		jump1 = -1;
		for (int i = 0; i < 3; i++) {

			out = point_out_prism(triangle[i], prismindex, jump1);

			if (out == true) {

				return 1;

			}

		}

		if (prismindex.size() == 1) return 0;



		////////////////////degeneration fix

		int degeneration = is_triangle_degenerated(triangle[0], triangle[1], triangle[2]);

		if (degeneration == DEGENERATED_POINT) {//case 1 degenerate to a point

			return 0;

		}//case 1 degenerate to a point

		if (degeneration == DEGENERATED_SEGMENT) {

			for (int we = 0; we < 3; we++) {//case 2 degenerated as a segment, at most test 2 segments,but still we need to test 3, because

											// of the endpoint-triangle intersection will be ignored

											// the segment is {triangle[triseg[we][0]], triangle[triseg[we][1]]}
				
				
				
				for (int i = 0; i < prismindex.size(); i++) {
					jump1 = prismindex[i];
					if (prismindex[i] < prism_size) {
						std::vector<int>cid;
						cut = is_seg_cut_prism(jump1, triangle[triseg[we][0]], triangle[triseg[we][1]], cid);
						if (cut == true) {
							for (int j = 0; j < cid.size(); j++) {
								
								inter = Implicit_Seg_Facet_interpoint_Out_Prism_multi_precision(triangle[triseg[we][0]], triangle[triseg[we][1]],
									envprism[prismindex[i]][p_face[cid[j]][0]], envprism[prismindex[i]][p_face[cid[j]][1]], envprism[prismindex[i]][p_face[cid[j]][2]],
									prismindex, jump1, checker);//rational
								if (inter == 1) {

									return 1;

								}
							}
						}
					}
					else {
						int cindex = prismindex[i] - prism_size;
						std::vector<int>cid;
						cut = is_seg_cut_cube(cindex, triangle[triseg[we][0]], triangle[triseg[we][1]], cid);
						if (cut == true) {
							for (int j = 0; j < cid.size(); j++) {
							
								inter = Implicit_Seg_Facet_interpoint_Out_Prism_multi_precision(triangle[triseg[we][0]], triangle[triseg[we][1]],
									envcubic[cindex][c_face[cid[j]][0]], envcubic[cindex][c_face[cid[j]][1]], envcubic[cindex][c_face[cid[j]][2]],
									prismindex, jump1, checker);//rational

								
								if (inter == 1) {

									return 1;

								}
							}
						}
						
						
					}


				}

			}//case 2 case 2 degenerated as a segment

			return 0;

		}

		////////////////////////////////degeneration fix over


		for (int i = 0; i < prismindex.size(); i++) {
			jump1 = prismindex[i];
			
			if (prismindex[i] < prism_size) {
				
				std::vector<int> cidl;
				cut = is_triangle_cut_prism(prismindex[i],
					triangle[0], triangle[1], triangle[2], cidl);
				for (int j = 0; j < cidl.size(); j++) {
					for (int k = 0; k < 3; k++) {

						inter = Implicit_Seg_Facet_interpoint_Out_Prism_multi_precision(
							triangle[triseg[k][0]], triangle[triseg[k][1]],
							envprism[prismindex[i]][p_face[cidl[j]][0]],
							envprism[prismindex[i]][p_face[cidl[j]][1]],
							envprism[prismindex[i]][p_face[cidl[j]][2]],
							prismindex, jump1, checker);
						if (inter == 1) {

							return 1;

						}



					}
					inter_ijk_list.push_back({ {prismindex[i],cidl[j]} });
				}
				
			}
			else {
				int cindex = prismindex[i] - prism_size;
				std::vector<int> cidl;
				cut = is_triangle_cut_cube(cindex,
					triangle[0], triangle[1], triangle[2], cidl);
				for (int j = 0; j < cidl.size(); j++) {
					for (int k = 0; k < 3; k++) {
						tti = seg_cut_plane(triangle[triseg[k][0]], triangle[triseg[k][1]],
							envcubic[cindex][c_face[cidl[j]][0]],
							envcubic[cindex][c_face[cidl[j]][1]],
							envcubic[cindex][c_face[cidl[j]][2]]);
						if (tti == CUT_FACE) {
							inter = Implicit_Seg_Facet_interpoint_Out_Prism_multi_precision(
								triangle[triseg[k][0]], triangle[triseg[k][1]],
								envcubic[cindex][c_face[cidl[j]][0]],
								envcubic[cindex][c_face[cidl[j]][1]],
								envcubic[cindex][c_face[cidl[j]][2]],
								prismindex, jump1, checker);
							if (inter == 1) {

								return 1;

							}

						}
					}
					inter_ijk_list.push_back({ {prismindex[i],cidl[j]} });
				}
				
			}


		}



		int listsize = inter_ijk_list.size();


	
		int id, id0 = 0;
		for (int i = 0; i < listsize; i++) {
			jump1 = inter_ijk_list[i][0];
			for (int j = i + 1; j < listsize; j++) {

				//check triangle{ { envprism[list[i][0]][p_triangle[list[i][1]][list[i][2]][0]], ...[1],...[2] } } and triangle{ { envprism[list[j][0]][p_triangle[list[j][1]][list[j][2]][0]], ...[1],...[2] } }

				//and T
				if (inter_ijk_list[i][0] == inter_ijk_list[j][0]) {
					if (inter_ijk_list[i][0] < prism_size) {
						id = inter_ijk_list[i][1] * 8 + inter_ijk_list[j][1];
						id0 = prism_map[id][0];
						
					}
					else {
						id = inter_ijk_list[i][1] * 6 + inter_ijk_list[j][1];

						id0 = cubic_map[id][0];
					}
					if (id0 == -1) continue;
				}
				
			
					//find prism_map[list[i][1]*8+list[j][1]][0],prism_map[list[i][1]*8+list[j][1]][1]


				


					Vector3 t00, t01, t02, t10, t11, t12;
					int n1, n2;
					if (inter_ijk_list[i][0] < prism_size) {
						n1 = p_facenumber;
					}
					else {
						n1 = c_facenumber;
					}
					if (inter_ijk_list[j][0] < prism_size) {
						n2 = p_facenumber;
					}
					else {
						n2 = c_facenumber;
					}




					jump2 = inter_ijk_list[j][0];

					if (n1 == p_facenumber) {
						t00 = envprism[inter_ijk_list[i][0]][p_face[inter_ijk_list[i][1]][0]];
						t01 = envprism[inter_ijk_list[i][0]][p_face[inter_ijk_list[i][1]][1]];
						t02 = envprism[inter_ijk_list[i][0]][p_face[inter_ijk_list[i][1]][2]];
					}
					else {
						t00 = envcubic[inter_ijk_list[i][0] - prism_size][c_face[inter_ijk_list[i][1]][0]];
						t01 = envcubic[inter_ijk_list[i][0] - prism_size][c_face[inter_ijk_list[i][1]][1]];
						t02 = envcubic[inter_ijk_list[i][0] - prism_size][c_face[inter_ijk_list[i][1]][2]];
					}
					if (n2 == p_facenumber) {
						t10 = envprism[inter_ijk_list[j][0]][p_face[inter_ijk_list[j][1]][0]];
						t11 = envprism[inter_ijk_list[j][0]][p_face[inter_ijk_list[j][1]][1]];
						t12 = envprism[inter_ijk_list[j][0]][p_face[inter_ijk_list[j][1]][2]];
					}
					else {
						t10 = envcubic[inter_ijk_list[j][0] - prism_size][c_face[inter_ijk_list[j][1]][0]];
						t11 = envcubic[inter_ijk_list[j][0] - prism_size][c_face[inter_ijk_list[j][1]][1]];
						t12 = envcubic[inter_ijk_list[j][0] - prism_size][c_face[inter_ijk_list[j][1]][2]];
					}

					int inter2 = Implicit_Tri_Facet_Facet_interpoint_Out_Prism_multi_precision(triangle,

						t00, t01, t02, t10, t11, t12,

						prismindex, jump1, jump2, checker);

					


					if (inter2 == 1) {


						return 1;//out

					}

				


			}

		}





		return 0;

	}




	struct INDEX {
		int Pi;
		std::vector<int> FACES;
	};
	template<typename T>
	int FastEnvelope::Implicit_Seg_Facet_interpoint_Out_Prism_multi_precision(const Vector3& segpoint0, const Vector3& segpoint1, const Vector3& triangle0,
		const Vector3& triangle1, const Vector3& triangle2, const std::vector<int>& prismindex, const int& jump, const std::function<int(T)>& checker) const {
		int  ori,ori1;
		int inter = seg_cut_plane(segpoint0, segpoint1, triangle0, triangle1, triangle2);

		if (inter == CUT_COPLANAR) {// we can not add "CUT_EMPTY" to this, because we use tri-tri intersection, not tri-facet intersection
									//so even if seg cut tri or next tri, seg_cut_tri may returns cut_empty
			return NOT_INTERSECTED;//not intersected
		}

		int tot;
		Scalar a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;
		bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
			segpoint0[0], segpoint0[1], segpoint0[2],
			segpoint1[0], segpoint1[1], segpoint1[2],
			triangle0[0], triangle0[1], triangle0[2],
			triangle1[0], triangle1[1], triangle1[2],
			triangle2[0], triangle2[1], triangle2[2],
			a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);

		if (precom == false) {
			static T
				s00, s01, s02, s10, s11, s12,
				t00, t01, t02,
				t10, t11, t12,
				t20, t21, t22,
				a11r, a12r, a13r, dr,

				e00, e01, e02,
				e10, e11, e12,
				e20, e21, e22;


			s00 = segpoint0[0]; s01 = segpoint0[1]; s02 = segpoint0[2]; s10 = segpoint1[0]; s11 = segpoint1[1]; s12 = segpoint1[2];
			t00 = triangle0[0]; t01 = triangle0[1]; t02 = triangle0[2];
			t10 = triangle1[0]; t11 = triangle1[1]; t12 = triangle1[2];
			t20 = triangle2[0]; t21 = triangle2[1]; t22 = triangle2[2];

			bool premulti = orient3D_LPI_prefilter_multiprecision(s00, s01, s02, s10, s11, s12,
				t00, t01, t02, t10, t11, t12, t20, t21, t22, a11r, a12r, a13r, dr, checker);
			for (int i = 0; i < prismindex.size(); i++) {
				if (prismindex[i] == jump) {

					continue;
				}

				if (prismindex[i] < prism_size) {

					tot = 0;
					for (int j = 0; j < p_facenumber; j++) {


						e00 = envprism[prismindex[i]][p_face[j][0]][0]; e01 = envprism[prismindex[i]][p_face[j][0]][1]; e02 = envprism[prismindex[i]][p_face[j][0]][2];
						e10 = envprism[prismindex[i]][p_face[j][1]][0]; e11 = envprism[prismindex[i]][p_face[j][1]][1]; e12 = envprism[prismindex[i]][p_face[j][1]][2];
						e20 = envprism[prismindex[i]][p_face[j][2]][0]; e21 = envprism[prismindex[i]][p_face[j][2]][1]; e22 = envprism[prismindex[i]][p_face[j][2]][2];
						ori = orient3D_LPI_postfilter_multiprecision(a11r, a12r, a13r, dr, s00, s01, s02,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);
						
						if (ori == 1 || ori == 0) {
							break;
						}

						if (ori == -1) {
							tot++;
						}

					}
					if (tot == p_facenumber) {

						return IN_PRISM;
					}
				}
				else {
					tot = 0;
					for (int j = 0; j < c_facenumber; j++) {


						e00 = envcubic[prismindex[i]-prism_size][c_face[j][0]][0]; e01 = envcubic[prismindex[i]-prism_size][c_face[j][0]][1]; e02 = envcubic[prismindex[i]-prism_size][c_face[j][0]][2];
						e10 = envcubic[prismindex[i]-prism_size][c_face[j][1]][0]; e11 = envcubic[prismindex[i]-prism_size][c_face[j][1]][1]; e12 = envcubic[prismindex[i]-prism_size][c_face[j][1]][2];
						e20 = envcubic[prismindex[i]-prism_size][c_face[j][2]][0]; e21 = envcubic[prismindex[i]-prism_size][c_face[j][2]][1]; e22 = envcubic[prismindex[i]-prism_size][c_face[j][2]][2];
						ori = orient3D_LPI_postfilter_multiprecision(a11r, a12r, a13r, dr, s00, s01, s02,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);


						if (ori == 1 || ori == 0) {
							break;
						}

						if (ori == -1) {
							tot++;
						}

					}
					if (tot == c_facenumber) {

						return IN_PRISM;
					}
				}


			}
			return OUT_PRISM;
		}


		INDEX index;
		std::vector<INDEX> recompute;
		for (int i = 0; i < prismindex.size(); i++) {
			if (prismindex[i] == jump) {

				continue;
			}
			if (prismindex[i] < prism_size) {
				index.FACES.clear();
				tot = 0;
				for (int j = 0; j < p_facenumber; j++) {
					//ftimer2.start();
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							segpoint0[0], segpoint0[1], segpoint0[2],
							envprism[prismindex[i]][p_face[j][0]][0], envprism[prismindex[i]][p_face[j][0]][1], envprism[prismindex[i]][p_face[j][0]][2],
							envprism[prismindex[i]][p_face[j][1]][0], envprism[prismindex[i]][p_face[j][1]][1], envprism[prismindex[i]][p_face[j][1]][2],
							envprism[prismindex[i]][p_face[j][2]][0], envprism[prismindex[i]][p_face[j][2]][1], envprism[prismindex[i]][p_face[j][2]][2]);


					if (ori == 1) {
						break;
					}
					if (ori == 0) {
						index.FACES.emplace_back(j);
					}

					else if (ori == -1) {
						tot++;
					}

				}
				if (tot == p_facenumber) {

					return IN_PRISM;
				}

				if (ori != 1) {
					assert(!index.FACES.empty());
					index.Pi = prismindex[i];
					recompute.emplace_back(index);
				}
			}
			else {
				index.FACES.clear();
				tot = 0;
				for (int j = 0; j < c_facenumber; j++) {
					//ftimer2.start();
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							segpoint0[0], segpoint0[1], segpoint0[2],
							envcubic[prismindex[i]-prism_size][c_face[j][0]][0], envcubic[prismindex[i]-prism_size][c_face[j][0]][1], envcubic[prismindex[i]-prism_size][c_face[j][0]][2],
							envcubic[prismindex[i]-prism_size][c_face[j][1]][0], envcubic[prismindex[i]-prism_size][c_face[j][1]][1], envcubic[prismindex[i]-prism_size][c_face[j][1]][2],
							envcubic[prismindex[i]-prism_size][c_face[j][2]][0], envcubic[prismindex[i]-prism_size][c_face[j][2]][1], envcubic[prismindex[i]-prism_size][c_face[j][2]][2]);

					if (ori == 1) {
						break;
					}
					if (ori == 0) {
						index.FACES.emplace_back(j);
					}

					else if (ori == -1) {
						tot++;
					}

				}
				if (tot == c_facenumber) {

					return IN_PRISM;
				}

				if (ori != 1) {
					assert(!index.FACES.empty());
					index.Pi = prismindex[i];
					recompute.emplace_back(index);
				}
			}

		}

		if (!recompute.empty()) {
			static T
				s00, s01, s02, s10, s11, s12,
				t00, t01, t02,
				t10, t11, t12,
				t20, t21, t22,
				a11r, a12r, a13r, dr,

				e00, e01, e02,
				e10, e11, e12,
				e20, e21, e22;
			s00 = segpoint0[0]; s01 = segpoint0[1]; s02 = segpoint0[2]; s10 = segpoint1[0]; s11 = segpoint1[1]; s12 = segpoint1[2];
			t00 = triangle0[0]; t01 = triangle0[1]; t02 = triangle0[2];
			t10 = triangle1[0]; t11 = triangle1[1]; t12 = triangle1[2];
			t20 = triangle2[0]; t21 = triangle2[1]; t22 = triangle2[2];
			bool premulti = orient3D_LPI_prefilter_multiprecision(s00, s01, s02, s10, s11, s12,
				t00, t01, t02, t10, t11, t12, t20, t21, t22, a11r, a12r, a13r, dr, checker);


			for (int k = 0; k < recompute.size(); k++) {
				int in1 = recompute[k].Pi;
				if (in1 < prism_size) {
					for (int j = 0; j < recompute[k].FACES.size(); j++) {
						int in2 = recompute[k].FACES[j];


						e00 = envprism[in1][p_face[in2][0]][0]; e01 = envprism[in1][p_face[in2][0]][1]; e02 = envprism[in1][p_face[in2][0]][2];
						e10 = envprism[in1][p_face[in2][1]][0]; e11 = envprism[in1][p_face[in2][1]][1]; e12 = envprism[in1][p_face[in2][1]][2];
						e20 = envprism[in1][p_face[in2][2]][0]; e21 = envprism[in1][p_face[in2][2]][1]; e22 = envprism[in1][p_face[in2][2]][2];

						ori = orient3D_LPI_postfilter_multiprecision(a11r, a12r, a13r, dr, s00, s01, s02,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);

						if (ori == 1 || ori == 0) break;
					}

					if (ori == -1) return IN_PRISM;
				}
				else {
					for (int j = 0; j < recompute[k].FACES.size(); j++) {
						int in2 = recompute[k].FACES[j];


						e00 = envcubic[in1-prism_size][c_face[in2][0]][0]; e01 = envcubic[in1-prism_size][c_face[in2][0]][1]; e02 = envcubic[in1-prism_size][c_face[in2][0]][2];
						e10 = envcubic[in1-prism_size][c_face[in2][1]][0]; e11 = envcubic[in1-prism_size][c_face[in2][1]][1]; e12 = envcubic[in1-prism_size][c_face[in2][1]][2];
						e20 = envcubic[in1-prism_size][c_face[in2][2]][0]; e21 = envcubic[in1-prism_size][c_face[in2][2]][1]; e22 = envcubic[in1-prism_size][c_face[in2][2]][2];

						ori = orient3D_LPI_postfilter_multiprecision(a11r, a12r, a13r, dr, s00, s01, s02,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);



						if (ori == 1 || ori == 0) break;
					}

					if (ori == -1) return IN_PRISM;
				}

			}


		}

		return OUT_PRISM;
	}
	

template<typename T>
	int FastEnvelope::Implicit_Tri_Facet_Facet_interpoint_Out_Prism_multi_precision(const std::array<Vector3, 3>& triangle,
		const Vector3& facet10, const Vector3& facet11, const Vector3& facet12, const Vector3& facet20, const Vector3& facet21, const Vector3& facet22,
		const std::vector<int>& prismindex, const int& jump1, const int &jump2, const std::function<int(T)>& checker) const {
		int ori;
		int tot;
		bool in = is_3_triangle_cut(triangle, facet10, facet11, facet12, facet20, facet21, facet22,checker);

		if (in == 0) {
			return NOT_INTERSECTED;
		}

		Scalar n1, n2, n3, d, max1, max2, max3, max4, max5, max6, max7;
		bool precom = ip_filtered::orient3D_TPI_prefilter(
			triangle[0][0], triangle[0][1], triangle[0][2],
			triangle[1][0], triangle[1][1], triangle[1][2],
			triangle[2][0], triangle[2][1], triangle[2][2],
			facet10[0], facet10[1], facet10[2],
			facet11[0], facet11[1], facet11[2],
			facet12[0], facet12[1], facet12[2],
			facet20[0], facet20[1], facet20[2],
			facet21[0], facet21[1], facet21[2],
			facet22[0], facet22[1], facet22[2], d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7);

		if (precom == false) {
			static T
				t00, t01, t02,
				t10, t11, t12,
				t20, t21, t22,

				f100, f101, f102,
				f110, f111, f112,
				f120, f121, f122,

				f200, f201, f202,
				f210, f211, f212,
				f220, f221, f222,
				dr, n1r, n2r, n3r,

				e00, e01, e02,
				e10, e11, e12,
				e20, e21, e22;;
			t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
			t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
			t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

			f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
			f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
			f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

			f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
			f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
			f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);
			bool premulti = orient3D_TPI_prefilter_multiprecision(t00, t01, t02, t10, t11, t12, t20, t21, t22,
				f100, f101, f102, f110, f111, f112, f120, f121, f122,
				f200, f201, f202, f210, f211, f212, f220, f221, f222,
				dr, n1r, n2r, n3r, checker);

			for (int i = 0; i < prismindex.size(); i++) {
				if (prismindex[i] == jump1 || prismindex[i] == jump2) continue;
				if (prismindex[i] < prism_size) {
					tot = 0;
					for (int j = 0; j < p_facenumber; j++) {

						e00 = (envprism[prismindex[i]][p_face[j][0]][0]); e01 = (envprism[prismindex[i]][p_face[j][0]][1]); e02 = (envprism[prismindex[i]][p_face[j][0]][2]);
						e10 = (envprism[prismindex[i]][p_face[j][1]][0]); e11 = (envprism[prismindex[i]][p_face[j][1]][1]); e12 = (envprism[prismindex[i]][p_face[j][1]][2]);
						e20 = (envprism[prismindex[i]][p_face[j][2]][0]); e21 = (envprism[prismindex[i]][p_face[j][2]][1]); e22 = (envprism[prismindex[i]][p_face[j][2]][2]);
						ori = orient3D_TPI_postfilter_multiprecision(dr, n1r, n2r, n3r, e00, e01, e02, e10, e11, e12, e20, e21, e22, checker);

						if (ori == 1 || ori == 0) {
							break;
						}

						if (ori == -1) {
							tot++;
						}

					}
					if (tot == p_facenumber) {

						return IN_PRISM;
					}
				}
				else {
					tot = 0;
					for (int j = 0; j < c_facenumber; j++) {

						e00 = (envcubic[prismindex[i] - prism_size][c_face[j][0]][0]); e01 = (envcubic[prismindex[i]- prism_size][c_face[j][0]][1]); e02 = (envcubic[prismindex[i]- prism_size][c_face[j][0]][2]);
						e10 = (envcubic[prismindex[i] - prism_size][c_face[j][1]][0]); e11 = (envcubic[prismindex[i]- prism_size][c_face[j][1]][1]); e12 = (envcubic[prismindex[i]- prism_size][c_face[j][1]][2]);
						e20 = (envcubic[prismindex[i] - prism_size][c_face[j][2]][0]); e21 = (envcubic[prismindex[i]- prism_size][c_face[j][2]][1]); e22 = (envcubic[prismindex[i]- prism_size][c_face[j][2]][2]);
						ori = orient3D_TPI_postfilter_multiprecision(dr, n1r, n2r, n3r, e00, e01, e02, e10, e11, e12, e20, e21, e22, checker);

						if (ori == 1 || ori == 0) {
							break;
						}

						if (ori == -1) {
							tot++;
						}

					}
					if (tot == c_facenumber) {

						return IN_PRISM;
					}
				}


			}
			return OUT_PRISM;
		}

		INDEX index;
		std::vector<INDEX> recompute;
		for (int i = 0; i < prismindex.size(); i++) {
			if (prismindex[i] == jump1 || prismindex[i] == jump2) continue;
			if (prismindex[i] < prism_size) {
				index.FACES.clear();
				tot = 0;
				for (int j = 0; j < p_facenumber; j++) {
					//ftimer2.start();
					ori = ip_filtered::
						orient3D_TPI_postfilter(
							d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7,
							envprism[prismindex[i]][p_face[j][0]][0], envprism[prismindex[i]][p_face[j][0]][1], envprism[prismindex[i]][p_face[j][0]][2],
							envprism[prismindex[i]][p_face[j][1]][0], envprism[prismindex[i]][p_face[j][1]][1], envprism[prismindex[i]][p_face[j][1]][2],
							envprism[prismindex[i]][p_face[j][2]][0], envprism[prismindex[i]][p_face[j][2]][1], envprism[prismindex[i]][p_face[j][2]][2]);


					if (ori == 1) {
						break;
					}
					if (ori == 0) {
						index.FACES.emplace_back(j);
					}

					else if (ori == -1) {
						tot++;
					}

				}
				if (tot == p_facenumber) {

					return IN_PRISM;
				}

				if (ori != 1) {
					index.Pi = prismindex[i];
					recompute.emplace_back(index);
				}
			}
			else {
				index.FACES.clear();
				tot = 0;
				for (int j = 0; j < c_facenumber; j++) {
					//ftimer2.start();
					ori = ip_filtered::
						orient3D_TPI_postfilter(
							d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7,
							envcubic[prismindex[i] - prism_size][c_face[j][0]][0], envcubic[prismindex[i]- prism_size][c_face[j][0]][1], envcubic[prismindex[i]- prism_size][c_face[j][0]][2],
							envcubic[prismindex[i] - prism_size][c_face[j][1]][0], envcubic[prismindex[i]- prism_size][c_face[j][1]][1], envcubic[prismindex[i]- prism_size][c_face[j][1]][2],
							envcubic[prismindex[i] - prism_size][c_face[j][2]][0], envcubic[prismindex[i]- prism_size][c_face[j][2]][1], envcubic[prismindex[i]- prism_size][c_face[j][2]][2]);


					if (ori == 1) {
						break;
					}
					if (ori == 0) {
						index.FACES.emplace_back(j);
					}

					else if (ori == -1) {
						tot++;
					}

				}
				if (tot == c_facenumber) {

					return IN_PRISM;
				}

				if (ori != 1) {
					index.Pi = prismindex[i];
					recompute.emplace_back(index);
				}
			}


		}

		if (recompute.size() > 0) {
			static T
				t00, t01, t02,
				t10, t11, t12,
				t20, t21, t22,

				f100, f101, f102,
				f110, f111, f112,
				f120, f121, f122,

				f200, f201, f202,
				f210, f211, f212,
				f220, f221, f222,
				dr, n1r, n2r, n3r,

				e00, e01, e02,
				e10, e11, e12,
				e20, e21, e22;;
			t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
			t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
			t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

			f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
			f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
			f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

			f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
			f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
			f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);
			bool premulti = orient3D_TPI_prefilter_multiprecision(t00, t01, t02, t10, t11, t12, t20, t21, t22,
				f100, f101, f102, f110, f111, f112, f120, f121, f122,
				f200, f201, f202, f210, f211, f212, f220, f221, f222,
				dr, n1r, n2r, n3r, checker);



			for (int k = 0; k < recompute.size(); k++) {
				int in1 = recompute[k].Pi;

				if (in1 < prism_size) {
					for (int j = 0; j < recompute[k].FACES.size(); j++) {
						int in2 = recompute[k].FACES[j];

						e00 = (envprism[in1][p_face[in2][0]][0]); e01 = (envprism[in1][p_face[in2][0]][1]); e02 = (envprism[in1][p_face[in2][0]][2]);
						e10 = (envprism[in1][p_face[in2][1]][0]); e11 = (envprism[in1][p_face[in2][1]][1]); e12 = (envprism[in1][p_face[in2][1]][2]);
						e20 = (envprism[in1][p_face[in2][2]][0]); e21 = (envprism[in1][p_face[in2][2]][1]); e22 = (envprism[in1][p_face[in2][2]][2]);
						ori = orient3D_TPI_postfilter_multiprecision(dr, n1r, n2r, n3r,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);



						if (ori == 1 || ori == 0) break;
					}

					if (ori == -1) return IN_PRISM;
				}
				else {
					for (int j = 0; j < recompute[k].FACES.size(); j++) {
						int in2 = recompute[k].FACES[j];


						e00 = (envcubic[in1 - prism_size][c_face[in2][0]][0]); e01 = (envcubic[in1 - prism_size][c_face[in2][0]][1]); e02 = (envcubic[in1 - prism_size][c_face[in2][0]][2]);
						e10 = (envcubic[in1 - prism_size][c_face[in2][1]][0]); e11 = (envcubic[in1 - prism_size][c_face[in2][1]][1]); e12 = (envcubic[in1 - prism_size][c_face[in2][1]][2]);
						e20 = (envcubic[in1 - prism_size][c_face[in2][2]][0]); e21 = (envcubic[in1 - prism_size][c_face[in2][2]][1]); e22 = (envcubic[in1 - prism_size][c_face[in2][2]][2]);
						ori = orient3D_TPI_postfilter_multiprecision(dr, n1r, n2r, n3r,
							e00, e01, e02, e10, e11, e12,
							e20, e21, e22, checker);



						//if (ori == -2) std::cout << "impossible thing happens in lpi" << std::endl;
						if (ori == 1 || ori == 0) break;
					}

					if (ori == -1) return IN_PRISM;
				}

			}

		}



		return OUT_PRISM;
	}


#include<ctime>
	template<typename T>

	bool FastEnvelope::is_3_triangle_cut(const std::array<Vector3, 3>& triangle,
		const Vector3& facet10, const Vector3& facet11, const Vector3& facet12, const Vector3& facet20, const Vector3& facet21, const Vector3& facet22, const std::function<int(T)> &checker) {
		//make this guy static
		Vector3 n = (triangle[0] - triangle[1]).cross(triangle[0] - triangle[2]) + triangle[0];

		if (Predicates::orient_3d(n, triangle[0], triangle[1], triangle[2]) == 0) {
			std::cout << "Degeneration happens" << std::endl;
			//move this guy in constructor and use fixed seed
			srand(int(time(0)));
			n = { {Vector3(rand(),rand(),rand()) } };
		}
		Scalar d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7;
		bool pre = ip_filtered::
			orient3D_TPI_prefilter(
				triangle[0][0], triangle[0][1], triangle[0][2],
				triangle[1][0], triangle[1][1], triangle[1][2],
				triangle[2][0], triangle[2][1], triangle[2][2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7);

		if (pre == false) {
			static T
				t00, t01, t02,
				t10, t11, t12,
				t20, t21, t22,

				f100, f101, f102,
				f110, f111, f112,
				f120, f121, f122,

				f200, f201, f202,
				f210, f211, f212,
				f220, f221, f222,

				nr0, nr1, nr2,

				dr, n1r, n2r, n3r;

			t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
			t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
			t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

			f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
			f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
			f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

			f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
			f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
			f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);

			nr0 = (n[0]); nr1 = (n[1]); nr2 = (n[2]);
			bool premulti = orient3D_TPI_prefilter_multiprecision(
				t00, t01, t02, t10, t11, t12, t20, t21, t22,
				f100, f101, f102, f110, f111, f112, f120, f121, f122,
				f200, f201, f202, f210, f211, f212, f220, f221, f222,
				dr, n1r, n2r, n3r, checker);
			if (premulti == false) return false;

			int o1 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t00, t01, t02,
				t10, t11, t12, checker);
			/*if (o1 == 1) after21++;
			if (o1 == -1) after22++;
			if (o1 == 0) after20++;*/
			if (o1 == 0) return false;

			int o2 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t10, t11, t12,
				t20, t21, t22, checker);
			/*if (o2 == 1) after21++;
			if (o2 == -1) after22++;
			if (o2 == 0) after20++;*/
			if (o2 == 0 || o1 + o2 == 0) return false;

			int o3 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t20, t21, t22,
				t00, t01, t02, checker);
			/*if (o3 == 1) after21++;
			if (o3 == -1) after22++;
			if (o3 == 0) after20++;*/
			if (o3 == 0 || o1 + o3 == 0) {
				return false;
			}

			return true;
		}

		static T
			t00, t01, t02,
			t10, t11, t12,
			t20, t21, t22,

			f100, f101, f102,
			f110, f111, f112,
			f120, f121, f122,

			f200, f201, f202,
			f210, f211, f212,
			f220, f221, f222,

			nr0, nr1, nr2,

			dr, n1r, n2r, n3r;

		bool premulti = false;
		int o1 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			triangle[0][0], triangle[0][1], triangle[0][2],
			triangle[1][0], triangle[1][1], triangle[1][2]);
		if (o1 == 0) {


			t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
			t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
			t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

			f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
			f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
			f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

			f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
			f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
			f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);

			nr0 = (n[0]); nr1 = (n[1]); nr2 = (n[2]);

			premulti = orient3D_TPI_prefilter_multiprecision(
				t00, t01, t02, t10, t11, t12, t20, t21, t22,
				f100, f101, f102, f110, f111, f112, f120, f121, f122,
				f200, f201, f202, f210, f211, f212, f220, f221, f222,
				dr, n1r, n2r, n3r, checker);
			o1 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t00, t01, t02,
				t10, t11, t12, checker);
			/*if (o1 == 1) after21++;
			if (o1 == -1) after22++;
			if (o1 == 0) after20++;*/
		}

		if (o1 == 0) return false;

		int o2 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			triangle[1][0], triangle[1][1], triangle[1][2],
			triangle[2][0], triangle[2][1], triangle[2][2]);
		if (o2 == 0) {
			if (premulti == false) {
				t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
				t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
				t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

				f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
				f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
				f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

				f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
				f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
				f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);

				nr0 = (n[0]); nr1 = (n[1]); nr2 = (n[2]);
				premulti = orient3D_TPI_prefilter_multiprecision(
					t00, t01, t02, t10, t11, t12, t20, t21, t22,
					f100, f101, f102, f110, f111, f112, f120, f121, f122,
					f200, f201, f202, f210, f211, f212, f220, f221, f222,
					dr, n1r, n2r, n3r, checker);
			}
			o2 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t10, t11, t12,
				t20, t21, t22, checker);
			/*if (o2 == 1) after21++;
			if (o2 == -1) after22++;
			if (o2 == 0) after20++;*/
		}
		if (o2 == 0 || o1 + o2 == 0) return false;

		int o3 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			triangle[2][0], triangle[2][1], triangle[2][2],
			triangle[0][0], triangle[0][1], triangle[0][2]);
		if (o3 == 0) {
			if (premulti == false) {
				t00 = (triangle[0][0]); t01 = (triangle[0][1]); t02 = (triangle[0][2]);
				t10 = (triangle[1][0]); t11 = (triangle[1][1]); t12 = (triangle[1][2]);
				t20 = (triangle[2][0]); t21 = (triangle[2][1]); t22 = (triangle[2][2]);

				f100 = (facet10[0]); f101 = (facet10[1]); f102 = (facet10[2]);
				f110 = (facet11[0]); f111 = (facet11[1]); f112 = (facet11[2]);
				f120 = (facet12[0]); f121 = (facet12[1]); f122 = (facet12[2]);

				f200 = (facet20[0]); f201 = (facet20[1]); f202 = (facet20[2]);
				f210 = (facet21[0]); f211 = (facet21[1]); f212 = (facet21[2]);
				f220 = (facet22[0]); f221 = (facet22[1]); f222 = (facet22[2]);

				nr0 = (n[0]); nr1 = (n[1]); nr2 = (n[2]);
				premulti = orient3D_TPI_prefilter_multiprecision(
					t00, t01, t02, t10, t11, t12, t20, t21, t22,
					f100, f101, f102, f110, f111, f112, f120, f121, f122,
					f200, f201, f202, f210, f211, f212, f220, f221, f222,
					dr, n1r, n2r, n3r, checker);
			}
			o3 = orient3D_TPI_postfilter_multiprecision(
				dr, n1r, n2r, n3r,
				nr0, nr1, nr2,
				t20, t21, t22,
				t00, t01, t02, checker);
			/*if (o3 == 1) after21++;
			if (o3 == -1) after22++;
			if (o3 == 0) after20++;*/
		}
		if (o3 == 0 || o1 + o3 == 0) return false;

		return true;
	}
	

	int FastEnvelope::is_3_triangle_cut_float(
		const Vector3& tri0, const Vector3& tri1, const Vector3& tri2,
		const Vector3& facet10, const Vector3& facet11, const Vector3& facet12,
		const Vector3& facet20, const Vector3& facet21, const Vector3& facet22) {
		
		Vector3 n = (tri0 - tri1).cross(tri0 - tri2) + tri0;

		if (Predicates::orient_3d(n, tri0, tri1, tri2) == 0) {
			std::cout << "Degeneration happens !" << std::endl;
			
			srand(int(time(0)));
			n = { {Vector3(rand(),rand(),rand()) } };
		}
		Scalar d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7;
		bool pre = ip_filtered::
			orient3D_TPI_prefilter(
				tri0[0], tri0[1], tri0[2],
				tri1[0], tri1[1], tri1[2],
				tri2[0], tri2[1], tri2[2],
				facet10[0], facet10[1], facet10[2], facet11[0], facet11[1], facet11[2], facet12[0], facet12[1], facet12[2],
				facet20[0], facet20[1], facet20[2], facet21[0], facet21[1], facet21[2], facet22[0], facet22[1], facet22[2],
				d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7);

		if (pre == false) return 2;// means we dont know
		int o1 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			tri0[0], tri0[1], tri0[2],
			tri1[0], tri1[1], tri1[2]);
		int o2 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			tri1[0], tri1[1], tri1[2],
			tri2[0], tri2[1], tri2[2]);
		if (o1*o2 == -1) return 0;
		int o3 = ip_filtered::orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7, n[0], n[1], n[2],
			tri2[0], tri2[1], tri2[2],
			tri0[0], tri0[1], tri0[2]);
		if (o1*o3 == -1 || o2 * o3 == -1) return 0;
		if (o1*o2*o3 == 0) return 2;// means we dont know
		return 1;
	
	}




	
	int FastEnvelope::seg_cut_plane(const Vector3 & seg0, const Vector3 &seg1, const Vector3&t0, const Vector3&t1, const Vector3 &t2) {
		int o1, o2;
		o1 = Predicates::orient_3d(seg0, t0, t1, t2);
		o2 = Predicates::orient_3d(seg1, t0, t1, t2);
		int op = o1 * o2;
		if (op >= 0) {
			return CUT_COPLANAR;//in fact, coplanar and not on this plane
		}
		return CUT_FACE;
	}

	
	bool FastEnvelope::is_triangle_cut_prism(const int&pindex,
		const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, std::vector<int> &cid)const{
		
		
		bool cut[8];
		for (int i = 0; i < 8; i++) {
			cut[i] = false;
		}
		int o1[8], o2[8], o3[8],ori=0;
		std::vector<int> cutp;

		for (int i = 0; i < 8; i++) {

			o1[i] = Predicates::orient_3d(tri0, envprism[pindex][p_face[i][0]], envprism[pindex][p_face[i][1]], envprism[pindex][p_face[i][2]]);
			o2[i] = Predicates::orient_3d(tri1, envprism[pindex][p_face[i][0]], envprism[pindex][p_face[i][1]], envprism[pindex][p_face[i][2]]);
			o3[i] = Predicates::orient_3d(tri2, envprism[pindex][p_face[i][0]], envprism[pindex][p_face[i][1]], envprism[pindex][p_face[i][2]]);
			if (o1[i] + o2[i] + o3[i] >= 3) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 1) {
				return false;
			}
			if (o1[i] == 1 && o2[i] == 0 && o3[i] == 0) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 1 && o3[i] == 0) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 0) {
				return false;
			}
			

			if (o1[i] * o2[i] == -1 || o1[i] * o3[i] == -1 || o3[i] * o2[i] == -1) cutp.push_back(i);
		}
		if (cutp.size() ==0) {
			return false;
		}
		
		Scalar a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;
		for (int i = 0; i < cutp.size(); i++) {
			if (o1[cutp[i]] * o2[cutp[i]] == -1) {
				
				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri0[0], tri0[1], tri0[2],
					tri1[0], tri1[1], tri1[2],
					
					envprism[pindex][p_face[cutp[i]][0]][0], envprism[pindex][p_face[cutp[i]][0]][1], envprism[pindex][p_face[cutp[i]][0]][2],
					envprism[pindex][p_face[cutp[i]][1]][0], envprism[pindex][p_face[cutp[i]][1]][1], envprism[pindex][p_face[cutp[i]][1]][2], 
					envprism[pindex][p_face[cutp[i]][2]][0], envprism[pindex][p_face[cutp[i]][2]][1], envprism[pindex][p_face[cutp[i]][2]][2], 
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori= ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri0[0], tri0[1], tri0[2],
							envprism[pindex][p_face[cutp[j]][0]][0], envprism[pindex][p_face[cutp[j]][0]][1], envprism[pindex][p_face[cutp[j]][0]][2], 
							envprism[pindex][p_face[cutp[j]][1]][0], envprism[pindex][p_face[cutp[j]][1]][1], envprism[pindex][p_face[cutp[j]][1]][2],
							envprism[pindex][p_face[cutp[j]][2]][0], envprism[pindex][p_face[cutp[j]][2]][1], envprism[pindex][p_face[cutp[j]][2]][2]);
					
					if (ori == 1) break;
					
				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}

			}
			if (cut[cutp[i]] == true) continue;
			ori = 0;
			if (o1[cutp[i]] * o3[cutp[i]] == -1) {

				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri0[0], tri0[1], tri0[2],
					tri2[0], tri2[1], tri2[2],
					envprism[pindex][p_face[cutp[i]][0]][0], envprism[pindex][p_face[cutp[i]][0]][1], envprism[pindex][p_face[cutp[i]][0]][2],
					envprism[pindex][p_face[cutp[i]][1]][0], envprism[pindex][p_face[cutp[i]][1]][1], envprism[pindex][p_face[cutp[i]][1]][2],
					envprism[pindex][p_face[cutp[i]][2]][0], envprism[pindex][p_face[cutp[i]][2]][1], envprism[pindex][p_face[cutp[i]][2]][2],
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri0[0], tri0[1], tri0[2],
							envprism[pindex][p_face[cutp[j]][0]][0], envprism[pindex][p_face[cutp[j]][0]][1], envprism[pindex][p_face[cutp[j]][0]][2],
							envprism[pindex][p_face[cutp[j]][1]][0], envprism[pindex][p_face[cutp[j]][1]][1], envprism[pindex][p_face[cutp[j]][1]][2],
							envprism[pindex][p_face[cutp[j]][2]][0], envprism[pindex][p_face[cutp[j]][2]][1], envprism[pindex][p_face[cutp[j]][2]][2]);

					if (ori == 1) break;
					
				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}
			}


			if (cut[cutp[i]] == true) continue;
			ori = 0;
			if (o2[cutp[i]] * o3[cutp[i]] == -1) {

				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri1[0], tri1[1], tri1[2],
					tri2[0], tri2[1], tri2[2],
					envprism[pindex][p_face[cutp[i]][0]][0], envprism[pindex][p_face[cutp[i]][0]][1], envprism[pindex][p_face[cutp[i]][0]][2],
					envprism[pindex][p_face[cutp[i]][1]][0], envprism[pindex][p_face[cutp[i]][1]][1], envprism[pindex][p_face[cutp[i]][1]][2],
					envprism[pindex][p_face[cutp[i]][2]][0], envprism[pindex][p_face[cutp[i]][2]][1], envprism[pindex][p_face[cutp[i]][2]][2],
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri1[0], tri1[1], tri1[2],
							envprism[pindex][p_face[cutp[j]][0]][0], envprism[pindex][p_face[cutp[j]][0]][1], envprism[pindex][p_face[cutp[j]][0]][2],
							envprism[pindex][p_face[cutp[j]][1]][0], envprism[pindex][p_face[cutp[j]][1]][1], envprism[pindex][p_face[cutp[j]][1]][2],
							envprism[pindex][p_face[cutp[j]][2]][0], envprism[pindex][p_face[cutp[j]][2]][1], envprism[pindex][p_face[cutp[j]][2]][2]);

					if (ori == 1) break;
					
				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}
			}

		}

		if (cutp.size() <= 2) {
			for (int i = 0; i < 8; i++) {
				if (cut[i] == true) cid.push_back(i);
			}
			return true;
		}
		// triangle-facet-facet intersection
		Scalar  n1, n2, n3, max3, max4, max6, max7;
		for (int i = 0; i < cutp.size(); i++) {
			for (int j = i + 1; j < cutp.size(); j++) {
				if (cut[cutp[i]] == true && cut[cutp[j]] == true) continue;
				
				int id = cutp[i] * 8 + cutp[j];
				int id0 = prism_map[id][0];
				if (id0 == -1) continue;
				int inter = is_3_triangle_cut_float(
					tri0, tri1, tri2,
					envprism[pindex][p_face[cutp[i]][0]],
					envprism[pindex][p_face[cutp[i]][1]],
					envprism[pindex][p_face[cutp[i]][2]],
					envprism[pindex][p_face[cutp[j]][0]],
					envprism[pindex][p_face[cutp[j]][1]],
					envprism[pindex][p_face[cutp[j]][2]]);
				if (inter == 2) {//we dont know if point exist or if inside of triangle
					cut[cutp[i]] == true;
					cut[cutp[j]] == true;
					continue;
				}
				if (inter == 0) continue;// sure not inside
				
				bool pre = ip_filtered::
					orient3D_TPI_prefilter(
						tri0[0], tri0[1], tri0[2],
						tri1[0], tri1[1], tri1[2],
						tri2[0], tri2[1], tri2[2],
						envprism[pindex][p_face[cutp[i]][0]][0], envprism[pindex][p_face[cutp[i]][0]][1], envprism[pindex][p_face[cutp[i]][0]][2],
						envprism[pindex][p_face[cutp[i]][1]][0], envprism[pindex][p_face[cutp[i]][1]][1], envprism[pindex][p_face[cutp[i]][1]][2],
						envprism[pindex][p_face[cutp[i]][2]][0], envprism[pindex][p_face[cutp[i]][2]][1], envprism[pindex][p_face[cutp[i]][2]][2],
						envprism[pindex][p_face[cutp[j]][0]][0], envprism[pindex][p_face[cutp[j]][0]][1], envprism[pindex][p_face[cutp[j]][0]][2],
						envprism[pindex][p_face[cutp[j]][1]][0], envprism[pindex][p_face[cutp[j]][1]][1], envprism[pindex][p_face[cutp[j]][1]][2],
						envprism[pindex][p_face[cutp[j]][2]][0], envprism[pindex][p_face[cutp[j]][2]][1], envprism[pindex][p_face[cutp[j]][2]][2],
						d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7);
				
				for (int k = 0; k < cutp.size(); k++) {

					if (k == i || k == j) continue;
					
					ori = ip_filtered::
						orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7,
							envprism[pindex][p_face[cutp[k]][0]][0], envprism[pindex][p_face[cutp[k]][0]][1], envprism[pindex][p_face[cutp[k]][0]][2],
							envprism[pindex][p_face[cutp[k]][1]][0], envprism[pindex][p_face[cutp[k]][1]][1], envprism[pindex][p_face[cutp[k]][1]][2],
							envprism[pindex][p_face[cutp[k]][2]][0], envprism[pindex][p_face[cutp[k]][2]][1], envprism[pindex][p_face[cutp[k]][2]][2]);
					
					if (ori == 1) break;
					
				}

				if (ori != 1) {
					cut[cutp[i]] = true;
					cut[cutp[j]] = true;
				}
			}
		}

		for (int i = 0; i < 8; i++) {
			if (cut[i] == true) cid.push_back(i);
		}

		return true;

	}

	bool FastEnvelope::is_seg_cut_prism(const int&pindex,
		const Vector3& seg0, const Vector3& seg1, std::vector<int> &cid)const {
		bool cut[8];
		for (int i = 0; i < 8; i++) {
			cut[i] = false;
		}
		int o1[8], o2[8], ori = 0;
		std::vector<int> cutp;

		for (int i = 0; i < 8; i++) {

			o1[i] = Predicates::orient_3d(seg0, envprism[pindex][p_face[i][0]], envprism[pindex][p_face[i][1]], envprism[pindex][p_face[i][2]]);
			o2[i] = Predicates::orient_3d(seg1, envprism[pindex][p_face[i][0]], envprism[pindex][p_face[i][1]], envprism[pindex][p_face[i][2]]);

			if (o1[i] + o2[i] >= 1) {
				return false;
			}

			if (o1[i] == 0 && o2[i] == 0) {
				return false;
			}

			if (o1[i] * o2[i] == -1) cutp.push_back(i);
		}
		if (cutp.size() == 0) {
			return false;
		}

		Scalar a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;
		for (int i = 0; i < cutp.size(); i++) {


			bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
				seg0[0], seg0[1], seg0[2],
				seg1[0], seg1[1], seg1[2],
				envprism[pindex][p_face[cutp[i]][0]][0], envprism[pindex][p_face[cutp[i]][0]][1], envprism[pindex][p_face[cutp[i]][0]][2],
				envprism[pindex][p_face[cutp[i]][1]][0], envprism[pindex][p_face[cutp[i]][1]][1], envprism[pindex][p_face[cutp[i]][1]][2],
				envprism[pindex][p_face[cutp[i]][2]][0], envprism[pindex][p_face[cutp[i]][2]][1], envprism[pindex][p_face[cutp[i]][2]][2], 
				a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
			if (precom == false) {
				cut[cutp[i]] = true;
				continue;
			}
			for (int j = 0; j < cutp.size(); j++) {
				if (i == j) continue;
				ori = ip_filtered::
					orient3D_LPI_postfilter(
						a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
						seg0[0], seg0[1], seg0[2],
						envprism[pindex][p_face[cutp[j]][0]][0], envprism[pindex][p_face[cutp[j]][0]][1], envprism[pindex][p_face[cutp[j]][0]][2],
						envprism[pindex][p_face[cutp[j]][1]][0], envprism[pindex][p_face[cutp[j]][1]][1], envprism[pindex][p_face[cutp[j]][1]][2],
						envprism[pindex][p_face[cutp[j]][2]][0], envprism[pindex][p_face[cutp[j]][2]][1], envprism[pindex][p_face[cutp[j]][2]][2]);

				if (ori == 1) break;

			}
			if (ori != 1) {
				cut[cutp[i]] = true;
			}

		}

		for (int i = 0; i < 8; i++) {
			if (cut[i] == true) cid.push_back(i);
		}

		return true;
	}
	bool FastEnvelope::is_triangle_cut_cube(const int&cindex,
		const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, std::vector<int> &cid)const {

		bool cut[6];
		for (int i = 0; i < 6; i++) {
			cut[i] = false;
		}
		int o1[6], o2[6], o3[6], ori = 0;
		std::vector<int> cutp;

		for (int i = 0; i < 6; i++) {

			o1[i] = Predicates::orient_3d(tri0, envcubic[cindex][c_face[i][0]], envcubic[cindex][c_face[i][1]], envcubic[cindex][c_face[i][2]]);
			o2[i] = Predicates::orient_3d(tri1, envcubic[cindex][c_face[i][0]], envcubic[cindex][c_face[i][1]], envcubic[cindex][c_face[i][2]]);
			o3[i] = Predicates::orient_3d(tri2, envcubic[cindex][c_face[i][0]], envcubic[cindex][c_face[i][1]], envcubic[cindex][c_face[i][2]]);
			if (o1[i] + o2[i] + o3[i] >= 3) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 1) {
				return false;
			}
			if (o1[i] == 1 && o2[i] == 0 && o3[i] == 0) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 1 && o3[i] == 0) {
				return false;
			}
			if (o1[i] == 0 && o2[i] == 0 && o3[i] == 0) {
				return false;
			}


			if (o1[i] * o2[i] == -1 || o1[i] * o3[i] == -1 || o3[i] * o2[i] == -1) cutp.push_back(i);
		}
		if (cutp.size() == 0) {
			return false;
		}

		Scalar a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;
		for (int i = 0; i < cutp.size(); i++) {
			if (o1[cutp[i]] * o2[cutp[i]] == -1) {

				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri0[0], tri0[1], tri0[2],
					tri1[0], tri1[1], tri1[2],

					envcubic[cindex][c_face[cutp[i]][0]][0], envcubic[cindex][c_face[cutp[i]][0]][1], envcubic[cindex][c_face[cutp[i]][0]][2],
					envcubic[cindex][c_face[cutp[i]][1]][0], envcubic[cindex][c_face[cutp[i]][1]][1], envcubic[cindex][c_face[cutp[i]][1]][2],
					envcubic[cindex][c_face[cutp[i]][2]][0], envcubic[cindex][c_face[cutp[i]][2]][1], envcubic[cindex][c_face[cutp[i]][2]][2],
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri0[0], tri0[1], tri0[2],
							envcubic[cindex][c_face[cutp[j]][0]][0], envcubic[cindex][c_face[cutp[j]][0]][1], envcubic[cindex][c_face[cutp[j]][0]][2],
							envcubic[cindex][c_face[cutp[j]][1]][0], envcubic[cindex][c_face[cutp[j]][1]][1], envcubic[cindex][c_face[cutp[j]][1]][2],
							envcubic[cindex][c_face[cutp[j]][2]][0], envcubic[cindex][c_face[cutp[j]][2]][1], envcubic[cindex][c_face[cutp[j]][2]][2]
						);

					if (ori == 1) break;

				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}

			}
			if (cut[cutp[i]] == true) continue;
			ori = 0;
			if (o1[cutp[i]] * o3[cutp[i]] == -1) {

				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri0[0], tri0[1], tri0[2],
					tri2[0], tri2[1], tri2[2],
					envcubic[cindex][c_face[cutp[i]][0]][0], envcubic[cindex][c_face[cutp[i]][0]][1], envcubic[cindex][c_face[cutp[i]][0]][2],
					envcubic[cindex][c_face[cutp[i]][1]][0], envcubic[cindex][c_face[cutp[i]][1]][1], envcubic[cindex][c_face[cutp[i]][1]][2],
					envcubic[cindex][c_face[cutp[i]][2]][0], envcubic[cindex][c_face[cutp[i]][2]][1], envcubic[cindex][c_face[cutp[i]][2]][2],
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri0[0], tri0[1], tri0[2],
							envcubic[cindex][c_face[cutp[j]][0]][0], envcubic[cindex][c_face[cutp[j]][0]][1], envcubic[cindex][c_face[cutp[j]][0]][2],
							envcubic[cindex][c_face[cutp[j]][1]][0], envcubic[cindex][c_face[cutp[j]][1]][1], envcubic[cindex][c_face[cutp[j]][1]][2],
							envcubic[cindex][c_face[cutp[j]][2]][0], envcubic[cindex][c_face[cutp[j]][2]][1], envcubic[cindex][c_face[cutp[j]][2]][2]);

					if (ori == 1) break;

				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}
			}


			if (cut[cutp[i]] == true) continue;
			ori = 0;
			if (o2[cutp[i]] * o3[cutp[i]] == -1) {

				bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
					tri1[0], tri1[1], tri1[2],
					tri2[0], tri2[1], tri2[2],
					envcubic[cindex][c_face[cutp[i]][0]][0], envcubic[cindex][c_face[cutp[i]][0]][1], envcubic[cindex][c_face[cutp[i]][0]][2],
					envcubic[cindex][c_face[cutp[i]][1]][0], envcubic[cindex][c_face[cutp[i]][1]][1], envcubic[cindex][c_face[cutp[i]][1]][2],
					envcubic[cindex][c_face[cutp[i]][2]][0], envcubic[cindex][c_face[cutp[i]][2]][1], envcubic[cindex][c_face[cutp[i]][2]][2],
					a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
				if (precom == false) {
					cut[cutp[i]] = true;
					continue;
				}
				for (int j = 0; j < cutp.size(); j++) {
					if (i == j) continue;
					ori = ip_filtered::
						orient3D_LPI_postfilter(
							a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
							tri1[0], tri1[1], tri1[2],
							envcubic[cindex][c_face[cutp[j]][0]][0], envcubic[cindex][c_face[cutp[j]][0]][1], envcubic[cindex][c_face[cutp[j]][0]][2],
							envcubic[cindex][c_face[cutp[j]][1]][0], envcubic[cindex][c_face[cutp[j]][1]][1], envcubic[cindex][c_face[cutp[j]][1]][2],
							envcubic[cindex][c_face[cutp[j]][2]][0], envcubic[cindex][c_face[cutp[j]][2]][1], envcubic[cindex][c_face[cutp[j]][2]][2]);

					if (ori == 1) break;

				}
				if (ori != 1) {
					cut[cutp[i]] = true;
				}
			}

		}

		if (cutp.size() <= 2) {
			for (int i = 0; i < 6; i++) {
				if (cut[i] == true) cid.push_back(i);
			}
			return true;
		}
		// triangle-facet-facet intersection
		Scalar  n1, n2, n3, max3, max4, max6, max7;
		for (int i = 0; i < cutp.size(); i++) {
			for (int j = i + 1; j < cutp.size(); j++) {
				if (cut[cutp[i]] == true && cut[cutp[j]] == true) continue;

				int id = cutp[i] * 6 + cutp[j];
				int id0 = prism_map[id][0];
				if (id0 == -1) continue;
				int inter = is_3_triangle_cut_float(
					tri0, tri1, tri2,
					envcubic[cindex][c_face[cutp[i]][0]],
					envcubic[cindex][c_face[cutp[i]][1]],
					envcubic[cindex][c_face[cutp[i]][2]],
					envcubic[cindex][c_face[cutp[j]][0]],
					envcubic[cindex][c_face[cutp[j]][1]],
					envcubic[cindex][c_face[cutp[j]][2]]);
				if (inter == 2) {//we dont know if point exist or if inside of triangle
					cut[cutp[i]] == true;
					cut[cutp[j]] == true;
					continue;
				}
				if (inter == 0) continue;// sure not inside

				bool pre = ip_filtered::
					orient3D_TPI_prefilter(
						tri0[0], tri0[1], tri0[2],
						tri1[0], tri1[1], tri1[2],
						tri2[0], tri2[1], tri2[2],
						envcubic[cindex][c_face[cutp[i]][0]][0], envcubic[cindex][c_face[cutp[i]][0]][1], envcubic[cindex][c_face[cutp[i]][0]][2],
						envcubic[cindex][c_face[cutp[i]][1]][0], envcubic[cindex][c_face[cutp[i]][1]][1], envcubic[cindex][c_face[cutp[i]][1]][2],
						envcubic[cindex][c_face[cutp[i]][2]][0], envcubic[cindex][c_face[cutp[i]][2]][1], envcubic[cindex][c_face[cutp[i]][2]][2],
						envcubic[cindex][c_face[cutp[j]][0]][0], envcubic[cindex][c_face[cutp[j]][0]][1], envcubic[cindex][c_face[cutp[j]][0]][2],
						envcubic[cindex][c_face[cutp[j]][1]][0], envcubic[cindex][c_face[cutp[j]][1]][1], envcubic[cindex][c_face[cutp[j]][1]][2],
						envcubic[cindex][c_face[cutp[j]][2]][0], envcubic[cindex][c_face[cutp[j]][2]][1], envcubic[cindex][c_face[cutp[j]][2]][2],
						d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7);

				for (int k = 0; k < cutp.size(); k++) {

					if (k == i || k == j) continue;

					ori = ip_filtered::
						orient3D_TPI_postfilter(d, n1, n2, n3, max1, max2, max3, max4, max5, max6, max7,
							envcubic[cindex][c_face[cutp[k]][0]][0], envcubic[cindex][c_face[cutp[k]][0]][1], envcubic[cindex][c_face[cutp[k]][0]][2],
							envcubic[cindex][c_face[cutp[k]][1]][0], envcubic[cindex][c_face[cutp[k]][1]][1], envcubic[cindex][c_face[cutp[k]][1]][2],
							envcubic[cindex][c_face[cutp[k]][2]][0], envcubic[cindex][c_face[cutp[k]][2]][1], envcubic[cindex][c_face[cutp[k]][2]][2]);

					if (ori == 1) break;

				}

				if (ori != 1) {
					cut[cutp[i]] = true;
					cut[cutp[j]] = true;
				}
			}
		}

		for (int i = 0; i < 6; i++) {
			if (cut[i] == true) cid.push_back(i);
		}

		return true;
	}
	bool FastEnvelope::is_seg_cut_cube(const int&cindex,
		const Vector3& seg0, const Vector3& seg1, std::vector<int> &cid) const{
		bool cut[6];
		for (int i = 0; i < 6; i++) {
			cut[i] = false;
		}
		int o1[6], o2[6], ori = 0;
		std::vector<int> cutp;

		for (int i = 0; i < 6; i++) {

			o1[i] = Predicates::orient_3d(seg0, envcubic[cindex][c_face[i][0]], envcubic[cindex][c_face[i][1]], envcubic[cindex][c_face[i][2]]);
			o2[i] = Predicates::orient_3d(seg1, envcubic[cindex][c_face[i][0]], envcubic[cindex][c_face[i][1]], envcubic[cindex][c_face[i][2]]);

			if (o1[i] + o2[i] >= 1) {
				return false;
			}

			if (o1[i] == 0 && o2[i] == 0) {
				return false;
			}

			if (o1[i] * o2[i] == -1) cutp.push_back(i);
		}
		if (cutp.size() == 0) {
			return false;
		}

		Scalar a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5;
		for (int i = 0; i < cutp.size(); i++) {


			bool precom = ip_filtered::orient3D_LPI_prefilter(// it is boolean maybe need considering
				seg0[0], seg0[1], seg0[2],
				seg1[0], seg1[1], seg1[2],
				envcubic[cindex][c_face[cutp[i]][0]][0], envcubic[cindex][c_face[cutp[i]][0]][1], envcubic[cindex][c_face[cutp[i]][0]][2],
				envcubic[cindex][c_face[cutp[i]][1]][0], envcubic[cindex][c_face[cutp[i]][1]][1], envcubic[cindex][c_face[cutp[i]][1]][2],
				envcubic[cindex][c_face[cutp[i]][2]][0], envcubic[cindex][c_face[cutp[i]][2]][1], envcubic[cindex][c_face[cutp[i]][2]][2],
				a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5);
			if (precom == false) {
				cut[cutp[i]] = true;
				continue;
			}
			for (int j = 0; j < cutp.size(); j++) {
				if (i == j) continue;
				ori = ip_filtered::
					orient3D_LPI_postfilter(
						a11, a12, a13, d, fa11, fa12, fa13, max1, max2, max5,
						seg0[0], seg0[1], seg0[2],
						envcubic[cindex][c_face[cutp[j]][0]][0], envcubic[cindex][c_face[cutp[j]][0]][1], envcubic[cindex][c_face[cutp[j]][0]][2],
						envcubic[cindex][c_face[cutp[j]][1]][0], envcubic[cindex][c_face[cutp[j]][1]][1], envcubic[cindex][c_face[cutp[j]][1]][2],
						envcubic[cindex][c_face[cutp[j]][2]][0], envcubic[cindex][c_face[cutp[j]][2]][1], envcubic[cindex][c_face[cutp[j]][2]][2]);

				if (ori == 1) break;

			}
			if (ori != 1) {
				cut[cutp[i]] = true;
			}

		}

		for (int i = 0; i < 6; i++) {
			if (cut[i] == true) cid.push_back(i);
		}

		return true;
	}

	bool FastEnvelope::is_triangle_cut_bounding_box(
		const Vector3& tri0, const Vector3& tri1, const Vector3& tri2, const Vector3 &bmin, const Vector3 &bmax) {
		Vector3 tmin, tmax;
		get_tri_corners(tri0, tri1, tri2, tmin, tmax);
		return box_box_intersection(tmin, tmax, bmin, bmax);
	}


	bool FastEnvelope::point_out_prism(const Vector3 & point, const std::vector<int>& prismindex, const int& jump)const
	{

		int  ori;
		int psize = envprism.size();
		for (int i = 0; i < prismindex.size(); i++) {
			if (prismindex[i] == jump) continue;
			if (prismindex[i] < psize) {
				for (int j = 0; j < p_facenumber; j++) {

					ori = Predicates::orient_3d(envprism[prismindex[i]][p_face[j][0]], envprism[prismindex[i]][p_face[j][1]], envprism[prismindex[i]][p_face[j][2]], point);
					if (ori == -1 || ori == 0) {
						break;
					}
					if (j == 7) {

						return false;
					}
				}
			}
			else {
				int boxid = prismindex[i] - psize;
				for (int j = 0; j < c_facenumber; j++) {

					ori = Predicates::orient_3d(envcubic[boxid][c_face[j][0]], envcubic[boxid][c_face[j][1]], envcubic[boxid][c_face[j][2]], point);

					if (ori == -1 || ori == 0) {
						break;
					}
					if (j == 5) {

						return false;
					}
				}
			}

		}

		return true;
	}

	int FastEnvelope::is_triangle_degenerated(const Vector3& triangle0, const Vector3& triangle1, const Vector3& triangle2) {

		Vector3 a = triangle0 - triangle1, b = triangle0 - triangle2;
		Vector3 normal = a.cross(b);
		Scalar nbr = normal.norm();

		if (nbr > SCALAR_ZERO) {
			return NOT_DEGENERATED;
		}
		int ori;
		std::array < Vector2, 3> p;
		for (int j = 0; j < 3; j++) {

			p[0] = to_2d(triangle0, j);
			p[1] = to_2d(triangle1, j);
			p[2] = to_2d(triangle2, j);

			ori = Predicates::orient_2d(p[0], p[1], p[2]);
			if (ori != 0) {
				return NERLY_DEGENERATED;
			}
		}

		if (triangle0[0] != triangle1[0] || triangle0[1] != triangle1[1] || triangle0[2] != triangle1[2]) {
			return DEGENERATED_SEGMENT;
		}
		if (triangle0[0] != triangle2[0] || triangle0[1] != triangle2[1] || triangle0[2] != triangle2[2]) {
			return DEGENERATED_SEGMENT;
		}
		return DEGENERATED_POINT;

	}
	void FastEnvelope::BoxGeneration(const std::vector<Vector3>& m_ver, const std::vector<Vector3i>& m_faces, std::vector<std::array<Vector3, 12>>& envprism, std::vector<std::array<Vector3, 8>>& envbox, const Scalar& epsilon)
	{
		envprism.reserve(m_faces.size());
		Vector3 AB, AC, BC, normal, vector1, ABn;
		Parameters pram;
		std::array<Vector3, 6> polygon;
		std::array<Vector3, 12> polygonoff;
		std::array<Vector3, 8> box;
		std::array<Vector3, 8> boxorder = {
			{
				{1,1,1},
		{-1,1,1},
		{-1,-1,1},
		{1,-1,1},
		{1,1,-1},
		{-1,1,-1},
		{-1,-1,-1},
		{1,-1,-1},
		}
		};

		Scalar
			tolerance = epsilon / sqrt(3),

			de;

		for (int i = 0; i < m_faces.size(); i++) {
			AB = m_ver[m_faces[i][1]] - m_ver[m_faces[i][0]];
			AC = m_ver[m_faces[i][2]] - m_ver[m_faces[i][0]];
			BC = m_ver[m_faces[i][2]] - m_ver[m_faces[i][1]];
			de = is_triangle_degenerated(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], m_ver[m_faces[i][2]]);



			if (de == DEGENERATED_POINT) {
				std::cout << "Envelope Triangle Degeneration- Point" << std::endl;
				for (int j = 0; j < 8; j++) {
					box[j] = m_ver[m_faces[i][0]] + boxorder[j] * tolerance;
				}
				envbox.push_back(box);
				continue;
			}
			if (de == DEGENERATED_SEGMENT) {
				std::cout << "Envelope Triangle Degeneration- Segment" << std::endl;
				Scalar length1 = AB.norm(), length2 = AC.norm(), length3 = BC.norm();
				if (length1 >= length2 && length1 >= length3) {
					seg_cube(m_ver[m_faces[i][0]], m_ver[m_faces[i][1]], tolerance, box);
					envbox.push_back(box);
				}
				if (length2 >= length1 && length2 >= length3) {
					seg_cube(m_ver[m_faces[i][0]], m_ver[m_faces[i][2]], tolerance, box);
					envbox.push_back(box);
				}
				if (length3 >= length1 && length3 >= length2) {
					seg_cube(m_ver[m_faces[i][1]], m_ver[m_faces[i][2]], tolerance, box);
					envbox.push_back(box);
				}
				continue;
			}
			if (de == NERLY_DEGENERATED) {
				std::cout << "Envelope Triangle Degeneration- Nearly" << std::endl;

				normal = accurate_normal_vector(AB, AC);
				vector1= accurate_normal_vector(AB, normal);
				
			}
			else {
				normal = AB.cross(AC).normalized();
				vector1 = AB.cross(normal).normalized();
			}

			ABn = AB.normalized();
			polygon[0] = m_ver[m_faces[i][0]] + (vector1 - ABn) * tolerance;
			polygon[1] = m_ver[m_faces[i][1]] + (vector1 + ABn) * tolerance;
			if (AB.dot(BC) < 0) {
				polygon[2] = m_ver[m_faces[i][1]] + (-vector1 + ABn) * tolerance;
				polygon[3] = m_ver[m_faces[i][2]] + (-vector1 + ABn) * tolerance;
				polygon[4] = m_ver[m_faces[i][2]] + (-vector1 - ABn) * tolerance;
				if (AB.dot(AC) < 0) {
					polygon[5] = m_ver[m_faces[i][2]] + (vector1 - ABn) * tolerance;
				}
				else {
					polygon[5] = m_ver[m_faces[i][0]] + (-vector1 - ABn) * tolerance;
				}
			}
			else {
				polygon[2] = m_ver[m_faces[i][2]] + (vector1 + ABn) * tolerance;
				polygon[3] = m_ver[m_faces[i][2]] + (-vector1 + ABn) * tolerance;
				polygon[4] = m_ver[m_faces[i][2]] + (-vector1 - ABn) * tolerance;
				polygon[5] = m_ver[m_faces[i][0]] + (-vector1 - ABn) * tolerance;
			}

			for (int j = 0; j < 6; j++) {
				polygonoff[j] = polygon[j] + normal * tolerance;
			}
			for (int j = 6; j < 12; j++) {
				polygonoff[j] = polygon[j - 6] - normal * tolerance;
			}
			envprism.emplace_back(polygonoff);

		}



	}
	void FastEnvelope::seg_cube(const Vector3 &p1, const Vector3 &p2, const Scalar& width, std::array<Vector3, 8>& envbox) {
		Vector3 v1, v2, v = p2 - p1;//p12
		if (v[0] != 0) {
			v1 = Vector3((0 - v[1] - v[2]) / v[0], 1, 1);
		}
		else {
			if (v[1] != 0) {
				v1 = Vector3(1, (0 - v[2]) / v[1], 1);

			}
			else {
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
		envbox[3] = p2 + width * (v + v1 - v2);//right hand out direction
		envbox[4] = p1 + width * (-v + v1 + v2);
		envbox[5] = p1 + width * (-v - v1 + v2);
		envbox[6] = p1 + width * (-v - v1 - v2);
		envbox[7] = p1 + width * (-v + v1 - v2);//right hand in direction
	}

	Vector3 FastEnvelope::accurate_normal_vector(const Vector3 & p, const Vector3 & q) {

		const Multiprecision ax = p[0];
		const Multiprecision ay = p[1];
		const Multiprecision az = p[2];

		const Multiprecision bx = q[0];
		const Multiprecision by = q[1];
		const Multiprecision bz = q[2];

		Multiprecision x = ay * bz - az * by;
		Multiprecision y = az * bx - ax * bz;
		Multiprecision z = ax * by - ay * bx;
		Multiprecision ssum = x * x + y * y + z * z;
		const Multiprecision length = ssum.sqrt(ssum);
		x = x / length; y = y / length; z = z / length;

		Scalar fx = x.to_double(), fy = y.to_double(), fz = z.to_double();
		return Vector3(fx, fy, fz);

	}




}












