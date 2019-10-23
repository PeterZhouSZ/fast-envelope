﻿#include <fastenvelope/FastEnvelope.h>
#include <fastenvelope/MeshIO.hpp>
#include <fastenvelope/Multiprecision.hpp>
#include <fastenvelope/Predicates.hpp>
#include <fastenvelope/Types.hpp>
#include <fastenvelope/AABBWrapper.h>
#include <fastenvelope/Rational.hpp>

#include <igl/Timer.h>


#include <geogram/basic/command_line.h>

#include <geogram/basic/command_line_args.h>

//#include<fastenvelope/EnvelopeTest.h>
#include <unordered_map>

#include <stdlib.h>

#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <istream>
#include <iostream>
#include <array>
#include <ctime>
#include <cstdlib>
#include<igl/writeOBJ.h>

using namespace fastEnvelope;
using namespace std;



void tri_tri_cutting_try() {

	Vector3 p1, p2, p3, q1, q2, q3;

	p1 = { 1,0,0 };

	p2 = { 0,1,0 };

	p3 = { 0,0,0 };


	q1 = { 0,0,1 };

	q2 = { -0.5,-0.5,-0.5 };

	q3 = { -0.5,-0.5,-0.5 };


	//FastEnvelope::test_tri_tri_cut( q1, q2, q3,p1, p2, p3);
	//FastEnvelope::test_tri_tri_cut( p1, p2, p3, q1, q2, q3 );

}
void get_bb_corners(const std::vector<Vector3> &vertices, Vector3 &min, Vector3 &max) {//TODO why use this one
	min = vertices.front();
	max = vertices.front();

	for (size_t j = 0; j < vertices.size(); j++) {
		for (int i = 0; i < 3; i++) {
			min(i) = std::min(min(i), vertices[j](i));
			max(i) = std::max(max(i), vertices[j](i));
		}
	}
}
void unordered_map_try() {
	std::vector<int> a, b;
	a.push_back(0);
	a.push_back(2);
	b.push_back(7);
	std::unordered_map<int, std::vector<int>> letter;
	letter[1] = a;
	letter[3] = b;
	letter[4].push_back(1);
	auto search = letter.find(4);
	if (search == letter.end()) {
		std::cout << "no find" << std::endl;
	}
	std::cout << "Found or not " << search->first << " size " << search->second.size() << '\n';
}

void get_triangle_corners(const std::array<Vector3, 3> &triangle, Vector3 &min, Vector3 &max) {
	min[0] = std::min(std::min(triangle[0][0], triangle[1][0]), triangle[2][0]);
	min[1] = std::min(std::min(triangle[0][1], triangle[1][1]), triangle[2][1]);
	min[2] = std::min(std::min(triangle[0][2], triangle[1][2]), triangle[2][2]);
	max[0] = std::max(std::max(triangle[0][0], triangle[1][0]), triangle[2][0]);
	max[1] = std::max(std::max(triangle[0][1], triangle[1][1]), triangle[2][1]);
	max[2] = std::max(std::max(triangle[0][2], triangle[1][2]), triangle[2][2]);

}






void testOrientation() {
	std::array<Vector3, 3> tri = { { Vector3(0,0,0),Vector3(1,0,0),Vector3(0,1,0) } };// right hand law
	Vector3 p = { 0,0,1 };
	int ori = Predicates::orient_3d(p, tri[0], tri[1], tri[2]);
	std::cout << "orientation test : " << ori << std::endl;
}
void sample_trianglex(const std::array<Vector3, 3>& vs, std::vector<GEO::vec3>& ps, Scalar sampling_dist) {
	Scalar sqrt3_2 = std::sqrt(3) / 2;

	std::array<Scalar, 3> ls;
	for (int i = 0; i < 3; i++) {
		ls[i] = (vs[i] - vs[(i + 1) % 3]).squaredNorm();
	}
	auto min_max = std::minmax_element(ls.begin(), ls.end());
	int min_i = min_max.first - ls.begin();
	int max_i = min_max.second - ls.begin();
	Scalar N = sqrt(ls[max_i]) / sampling_dist;
	if (N <= 1) {
		for (int i = 0; i < 3; i++)
			ps.push_back(GEO::vec3(vs[i][0], vs[i][1], vs[i][2]));
		return;
	}
	if (N == int(N))
		N -= 1;

	GEO::vec3 v0(vs[max_i][0], vs[max_i][1], vs[max_i][2]);
	GEO::vec3 v1(vs[(max_i + 1) % 3][0], vs[(max_i + 1) % 3][1], vs[(max_i + 1) % 3][2]);
	GEO::vec3 v2(vs[(max_i + 2) % 3][0], vs[(max_i + 2) % 3][1], vs[(max_i + 2) % 3][2]);

	GEO::vec3 n_v0v1 = GEO::normalize(v1 - v0);
	for (int n = 0; n <= N; n++) {
		ps.push_back(v0 + n_v0v1 * sampling_dist * n);
	}
	ps.push_back(v1);

	Scalar h = GEO::distance(GEO::dot((v2 - v0), (v1 - v0)) * (v1 - v0) / ls[max_i] + v0, v2);
	int M = h / (sqrt3_2 * sampling_dist);
	if (M < 1) {
		ps.push_back(v2);
		return;
	}

	GEO::vec3 n_v0v2 = GEO::normalize(v2 - v0);
	GEO::vec3 n_v1v2 = GEO::normalize(v2 - v1);
	Scalar tan_v0, tan_v1, sin_v0, sin_v1;
	sin_v0 = GEO::length(GEO::cross((v2 - v0), (v1 - v0))) / (GEO::distance(v0, v2) * GEO::distance(v0, v1));
	tan_v0 = GEO::length(GEO::cross((v2 - v0), (v1 - v0))) / GEO::dot((v2 - v0), (v1 - v0));
	tan_v1 = GEO::length(GEO::cross((v2 - v1), (v0 - v1))) / GEO::dot((v2 - v1), (v0 - v1));
	sin_v1 = GEO::length(GEO::cross((v2 - v1), (v0 - v1))) / (GEO::distance(v1, v2) * GEO::distance(v0, v1));

	for (int m = 1; m <= M; m++) {
		int n = sqrt3_2 / tan_v0 * m + 0.5;
		int n1 = sqrt3_2 / tan_v0 * m;
		if (m % 2 == 0 && n == n1) {
			n += 1;
		}
		GEO::vec3 v0_m = v0 + m * sqrt3_2 * sampling_dist / sin_v0 * n_v0v2;
		GEO::vec3 v1_m = v1 + m * sqrt3_2 * sampling_dist / sin_v1 * n_v1v2;
		if (GEO::distance(v0_m, v1_m) <= sampling_dist)
			break;

		Scalar delta_d = ((n + (m % 2) / 2.0) - m * sqrt3_2 / tan_v0) * sampling_dist;
		GEO::vec3 v = v0_m + delta_d * n_v0v1;
		int N1 = GEO::distance(v, v1_m) / sampling_dist;
		//        ps.push_back(v0_m);
		for (int i = 0; i <= N1; i++) {
			ps.push_back(v + i * n_v0v1 * sampling_dist);
		}
		//        ps.push_back(v1_m);
	}
	ps.push_back(v2);

	//sample edges
	N = sqrt(ls[(max_i + 1) % 3]) / sampling_dist;
	if (N > 1) {
		if (N == int(N))
			N -= 1;
		GEO::vec3 n_v1v2 = GEO::normalize(v2 - v1);
		for (int n = 1; n <= N; n++) {
			ps.push_back(v1 + n_v1v2 * sampling_dist * n);
		}
	}

	N = sqrt(ls[(max_i + 2) % 3]) / sampling_dist;
	if (N > 1) {
		if (N == int(N))
			N -= 1;
		GEO::vec3 n_v2v0 = GEO::normalize(v0 - v2);
		for (int n = 1; n <= N; n++) {
			ps.push_back(v2 + n_v2v0 * sampling_dist * n);
		}
	}
}


bool is_out_function(const std::array<Vector3, 3>& triangle, const Scalar& dd, AABBWrapper& sf_tree) {
	std::vector<GEO::vec3> ps;
	sample_trianglex(triangle, ps, dd);//dd is used for sapmling
	return sf_tree.is_out_sf_envelope(ps, pow(dd*(1 - (1 / sqrt(3))), 2));

}







void calculation() {
	Eigen::Matrix<Scalar, 4, 4> biga, bigb;
	Eigen::Matrix<Scalar, 3, 3> a;
	Eigen::Matrix<Scalar, 3, 4> b;
	Eigen::Matrix<Scalar, 3, 1> c0;
	Eigen::Matrix<Scalar, 1, 3> r0;
	Eigen::Matrix<Scalar, 4, 6> bigc;
	a << 1, 2, 3,
		4, 5, 6,
		7, 8, 9;
	c0 << 0,
		0,
		0;
	r0 << 0, 0, 0;
	biga << a, c0,
		r0, 1;
	b << 1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12;
	bigb << b,
		1, 1, 1, 1;
	//std::cout << biga*bigb<<"\n" << std::endl;
	//std::cout << a*b<< "\n" << std::endl;

	a.setZero();
	std::array<Vector3, 3> triangle0;
	triangle0[0] = { {Vector3(0,0,1)} };
	triangle0[1] = { {Vector3(0,1,1)} };
	triangle0[2] = { {Vector3(7,0,1)} };
	a.transpose() << (triangle0[0] - triangle0[1]).cross(triangle0[0] - triangle0[2]),
		(triangle0[0] - triangle0[1]).cross(triangle0[0] - triangle0[2]),
		(triangle0[0] - triangle0[1]).cross(triangle0[0] - triangle0[2]);
	bigc << a, a,
		1, 1, 1, 1, 1, 1;
	std::cout << a << "\n" << std::endl;
	std::cout << bigc << "\n" << std::endl;
}

void test_ttt() {
	std::array<Vector3, 3> triangle, facet1, facet2, facet3;
	Eigen::Matrix<Scalar, 3, 3> A, AT, ATA;
	Eigen::Matrix<Scalar, 3, 1> B;
	Eigen::Matrix<Scalar, 4, 4> C;
	triangle = { { Vector3(0,0,0),Vector3(1,0,0),Vector3(0,1,0)} };
	facet1 = { { Vector3(0,0,0),Vector3(0,0,1),Vector3(1,0,0)} };
	facet2 = { { Vector3(0,0,0),Vector3(0,0,1),Vector3(0,1,0)} };
	facet3 = { { Vector3(0,0,0),Vector3(1,0,0),Vector3(0,1,0)} };
	AT << (triangle[0] - triangle[1]).cross(triangle[0] - triangle[2]),
		(facet1[0] - facet1[1]).cross(facet1[0] - facet1[2]),
		(facet2[0] - facet2[1]).cross(facet2[0] - facet2[2]);
	B << triangle[0], facet1[0], facet2[0];
	A = AT.transpose();
	B = A * B;
	ATA = AT * A;
	std::cout << A << std::endl;
	// if (FastEnvelope::determinant(A) == 0) { //mmnnmnmmnnnnnnnnnnnnnnnnnnnnnnmnmmmnmnmmmnmnmmmmmmnmn
	// 	std::cout << "A singular" << std::endl;
	// }
	int ori = -1; // FastEnvelope::orient_3triangles(A, AT, ATA, B, facet3); //FIXME maybe
	std::cout << ori << std::endl;
}






std::vector<std::array<Vector3, 3>> read_CSV_triangle(const string inputFileName, vector<int>& inenvelope) {


	std::vector<std::array<Vector3, 3>> triangle;

	ifstream infile;
	infile.open(inputFileName);
	if (!infile.is_open())
	{
		cout << "Path Wrong!!!!" << endl;
		return triangle;
	}

	int l = 0;
	while (infile) // there is input overload classfile
	{
		l++;
		string s;
		if (!getline(infile, s)) break;
		if (s[0] != '#') {
			istringstream ss(s);
			array<double, 10> record;
			int c = 0;
			while (ss) {
				string line;
				if (!getline(ss, line, ','))
					break;
				try {
					record[c] = stof(line);
					c++;
				}
				catch (const std::invalid_argument e) {
					cout << "NaN found in file " << inputFileName << " line " << l
						<< endl;
					e.what();
				}
			}

			triangle.push_back({ {Vector3(record[0],record[1],record[2]),Vector3(record[3],record[4],record[5]),
				Vector3(record[6],record[7],record[8])} });
			inenvelope.push_back(record[9]);
		}
	}
	if (!infile.eof()) {
		cerr << "Could not read file " << inputFileName << "\n";
	}
	cout << "triangle size " << triangle.size() << endl;
	return triangle;
}

//void test_in_wild(string inputFileName1, string input_surface_path1) {
void test_in_wild() {
	string inputFileName1 = "D:\\vs\\fast_envelope_csv\\problems\\110027.stl_env.csv";
	string input_surface_path1 = "D:\\vs\\fast_envelope_csv\\problems\\110027.stl";
	///string inputFileName1 = "D:\\vs\\fast_envelope_csv\\problems\\1088280.stl_env.csv";
	///string input_surface_path1 = "D:\\vs\\fast_envelope_csv\\problems\\1088280.stl";
	///

	vector<int> outenvelope;
	std::vector<std::array<Vector3, 3>> triangles = read_CSV_triangle(inputFileName1, outenvelope);

	std::vector<Vector3> env_vertices;
	std::vector<Vector3i> env_faces;
	GEO::Mesh envmesh;

	///////////////////////////////////////////////////////
	bool ok1 = MeshIO::load_mesh(input_surface_path1, env_vertices, env_faces, envmesh);
	if (!ok1) {
		std::cout << ("Unable to load mesh") << std::endl;
		return;
	}
	std::cout << "envface size  " << env_faces.size() << "\nenv ver size " << env_vertices.size() << std::endl;



	Scalar shrink = 1;
	Scalar eps = 1e-3;

	int ft;
	// if there are over one million triangles, then test maximal one million triangles
	if (triangles.size() > 1000000) {
		ft=1000000;
	}
	else {
		ft = triangles.size();//test face number
	}
	//////////////////////////////////////////////////////////////
	const int fn = ft;//test face number


	eps = eps / shrink;
	//eps = eps * sqrt(3)*(1 - (1 / sqrt(3)));//TODO to make bbd similar size to aabb method
	igl::Timer timer, timer1, timer2;




	timer.start();
	timer1.start();
	const FastEnvelope fast_envelope(env_vertices, env_faces, eps);
	//std::cout<<"p_size "<<fast_envelope.prism_size<<endl;
	std::cout << "time in initialization, " << timer1.getElapsedTimeInSec() << endl;
	// fast_envelope.print_ini_number(); //TODO
	timer2.start();
	vector<bool> pos1, pos2;
	pos1.resize(fn);
	pos2.resize(fn);

	for (int i = 0; i < fn; i++) {

		pos1[i] = outenvelope[i];
		//fast_envelope.print_prisms(triangles[i], "D:\\vs\\fast_envelope_csv\\problems\\");
		pos2[i] = fast_envelope.is_outside(triangles[i]);
		//if (i - i / 1000*1000 == 0) cout << "ten thousand test over " << i / 1000 << endl;

	}
	std::cout << "time in checking, " << timer2.getElapsedTimeInSec() << endl;
	std::cout << "time total, " << timer.getElapsedTimeInSec() << endl;


	int rcd = 0, eq0 = 0, eq02 = 0, rmk = 0;
	for (int i = 0; i < fn; i++) {
		if (pos1[i] - pos2[i] != 0) {
			//if (pos1[i]== 0) {
			rcd = rcd + 1;
			//std::cout << "envelope test different! different face NO. " << i << " the difference: " << pos1[i] - pos2[i] << std::endl;
			//std::cout << "envelope test same! same face NO. " << i << "the in and out : " <<pos1[i] <<","<<pos2[i] << std::endl;
		}
		if (pos1[i] == 0) {
			eq0 = eq0 + 1;
		}
		if (pos2[i] == 0) {
			eq02 = eq02 + 1;
		}
		if (pos1[i] == 0 && pos2[i] == 1) {
			rmk++;
		}
	}

	std::cout << "how many different cases:  " << rcd << std::endl;
	std::cout << "aabb inside triangle number:  " << eq0 << std::endl;
	std::cout << "our  inside triangle number:  " << eq02 << std::endl;
	std::cout << "0-1 cases number " << rmk << std::endl;
	cout << endl;
	// FastEnvelope::print_number(); //TODO

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<bool> pos3;
	pos3.resize(fn);
	std::array<Vector3, 3> tri;
	
	int pieces;
	std::vector<std::array<Vector3, 3>> t;
	std::vector<int> trindex1, trindex2;
	int insiden_o = 0, insiden_s = 0;

	for (int i = 0; i < fn; i++) {
		tri = triangles[i];
		pieces = 20;
		//l1 = (tri[0] - tri[1]).norm() / 30;
		pos3[i] = fast_envelope.sample_triangle_outside(tri, pieces);
	}

	int count = 0, count1 = 0, count2 = 0, count3 = 0;
	for (int i = 0; i < fn; i++) {
		if (pos2[i] - pos3[i] != 0) {
			count++;
		}
		if (pos2[i] - pos3[i] == 1) {
			count1++;
			trindex2.push_back(i);
		}
		if (pos2[i] - pos3[i] == -1) {
			trindex1.push_back(i);
		}
		if (pos2[i] == 0) {
			insiden_o++;
		}
		if (pos3[i] == 0) {
			insiden_s++;
		}

	}

	std::cout << "how many different cases in comparison:  " << count << std::endl;
	std::cout << "1-0 cases in comparison:  " << count1 << std::endl;
	std::cout << "!!0-1 cases in comparison:  " << trindex1.size() << std::endl;
	for (int i = 0; i < trindex1.size(); i++) cout << "NO. " << trindex1[i] << endl;
	std::cout << "1-0 case size:  " << trindex2.size() << std::endl;
	std::cout << "our inside size:  " << insiden_o << std::endl;
	std::cout << "sap inside size:  " << insiden_s << std::endl;

	int nbr = 0;
	for (int i = 0; i < trindex2.size(); i++) {

		if (FastEnvelope::is_triangle_degenerated(triangles[trindex2[i]][0], triangles[trindex2[i]][1], triangles[trindex2[i]][2]) != 0) {
			nbr++;
		}

	}
	cout << "\ndegeneration counting: " << nbr << endl;

	////////////////////////////////////////////////////////////////////////////////////////

	cout << "\nstarting iteration: " << endl;

	const int irt = 12;
	std::vector<bool> pos4;
	vector<int> ti, ti1;//triangle index
	ti = trindex2;
	int lth[irt] = { 60,100,200,400,600,800,1000,2000,2500,5000,10000,20000 };
	for (int j = 0; j < irt; j++) {
		int howmany = 0;
		ti1.resize(0);
		pos4.resize(ti.size());
		for (int i = 0; i < ti.size(); i++) {
			tri = triangles[ti[i]];


			pos4[i] = fast_envelope.sample_triangle_outside(tri, lth[j]);
			if (pos4[i] == 0) {
				howmany++;
				ti1.push_back(ti[i]);
			}

		}
		if (ti1.size() != 0) {
			cout << "the ith iteration " << j << endl;
			cout << "how many differences " << howmany << endl;
			ti = ti1;
		}
		else {
			cout << "kill all the different cases"<<endl;
			break;
		};
	}


	/*for (int j = 0; j < irt; j++) {
		int howmany = 0;
		ti1.resize(0);
		pos4.resize(ti.size());
		for (int i = 0; i < ti.size(); i++) {
			tri[0] = triangles[ti[i]][2];
			tri[1] = triangles[ti[i]][1];
			tri[2] = triangles[ti[i]][0];


			pos4[i] = fast_envelope.sample_triangle_outside(tri, lth[j]);
			if (pos4[i] == 0) {
				howmany++;
				ti1.push_back(ti[i]);
			}

		}
		if (ti1.size() != 0) {
			cout << "the ith iteration " << j << endl;
			cout << "how many differences " << howmany << endl;
			ti = ti1;
		}
		else {
			cout << "kill all the different cases";
			break;
		};
	}
*/

	if (ti1.size() > 0) {
		cout << "still have 1-0 cases not finished" << endl;
		for (int i = 0; i < ti1.size(); i++) {
			cout << "NO. ith " << ti[i] << endl;
		}
	}

	/*if (trindex1.size() > 0) {
		std::ofstream fout;
		fout.open("D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\visualtriangle.txt");
		int idx = 0;
		std::cout << "output NO. " << trindex1[idx] << endl;
		for (int i = 0; i < 3; i++) {

			fout << std::setprecision(17) << triangles[trindex1[idx]][i][0] << " " << triangles[trindex1[idx]][i][1] << " " << triangles[trindex1[idx]][i][2] << endl;

		}
		fout.close();

		fast_envelope.print_prisms(triangles[trindex1[idx]], "D:\\vs\\fast_envelope_csv\\problems\\");

	}
*/
	////for aabb method
	Vector3 min, max;

	Scalar dd;
	get_bb_corners(env_vertices, min, max);
	dd = ((max - min).norm()) *eps;
	timer.start();
	AABBWrapper sf_tree(envmesh);
	for (int i = 0; i < fn; i++) {

		is_out_function(triangles[i], dd, sf_tree); ;
	}
	cout << "aabb time " << timer.getElapsedTimeInSec() << endl;

	std::cout << "TEST aabb FINISHED  " << std::endl;
	//////////////////////////////
	//////////

}



//void test_without_sampling(string inputFileName1, string input_surface_path1) {
void test_without_sampling() {
	/*string inputFileName1 = "d:\\vs\\fast_envelope_csv\\thingi10k_debug\\100639\\100639.stl_env.csv";
	string input_surface_path1 = "d:\\vs\\fast_envelope_csv\\thingi10k_debug\\100639\\helicopter_logo_x1.stl";*/
	string inputFileName1 = "d:\\vs\\fast_envelope_csv\\problems\\905406.stl_envelope_log.csv";
	string input_surface_path1 = "d:\\vs\\fast_envelope_csv\\problems\\905406.stl";
	

	vector<int> outenvelope;
	std::vector<std::array<Vector3, 3>> triangles = read_CSV_triangle(inputFileName1, outenvelope);
	if (triangles.size() == 0) return;
	std::vector<Vector3> env_vertices;
	std::vector<Vector3i> env_faces;
	GEO::Mesh envmesh;

	///////////////////////////////////////////////////////
	bool ok1 = MeshIO::load_mesh(input_surface_path1, env_vertices, env_faces, envmesh);
	if (!ok1) {
		std::cout << ("Unable to load mesh") << std::endl;
		return;
	}
	std::cout << "envface size  " << env_faces.size() << "\nenv ver size " << env_vertices.size() << std::endl;



	Scalar shrink = 1;
	Scalar eps = 1e-3;
	const int spac = 10;// space subdivision parameter
	int ft;
	// if there are over one million triangles, then test maximal one million triangles
	if (triangles.size() > 500000) {
		ft = 500000;
	}
	else {
		ft = triangles.size();//test face number
	}
	//////////////////////////////////////////////////////////////
	const int fn = ft;//test face number


	eps = eps / shrink;
	//eps = eps * sqrt(3)*(1 - (1 / sqrt(3)));//TODO to make bbd similar size to aabb method
	igl::Timer timer, timer1, timer2;


	/////////////////////////////////
	////for aabb method
	//Vector3 min, max;
	//Parameters params;
	//Scalar dd;
	//get_bb_corners(params, env_vertices, min, max);
	//dd = ((max - min).norm()) / 1000 / shrink;
	//timer.start();
	//AABBWrapper sf_tree(envmesh);
	//for (int i = 0; i < fn; i++) {

	//	is_out_function(triangles[i], dd, sf_tree); ;
	//}
	//cout << "aabb time " << timer.getElapsedTimeInSec() << endl;

	//std::cout << "TEST aabb FINISHED  " << std::endl;
	//////////////////////////////

	Scalar temptime=0;
	timer.start();
	timer1.start();
	const FastEnvelope fast_envelope(env_vertices, env_faces, eps);
	//std::cout<<"p_size "<<fast_envelope.prism_size<<endl;
	std::cout << "time in initialization, " << timer1.getElapsedTimeInSec() << endl;
	// fast_envelope.print_ini_number(); //TODO
	timer2.start();
	vector<bool> pos1, pos2;
	pos1.resize(fn);
	pos2.resize(fn);
	
	for (int i = 0; i < fn; i++) {//3294
								  //34783,89402,
		
		pos1[i] = outenvelope[i];
		timer1.start();
		pos2[i] = fast_envelope.is_outside(triangles[i]);
		//if (i % 100 == 0) cout << "ten thousand test over " << i << endl;
		if (timer1.getElapsedTimeInSec() > temptime) {
			temptime = timer1.getElapsedTimeInSec();
			cout << "time get longer " << i << ", " << temptime << std::endl;
			
		}

	}
	
	
	std::cout << "time in checking, " << timer2.getElapsedTimeInSec() << endl;
	std::cout << "time total, " << timer.getElapsedTimeInSec() << endl;
	fast_envelope.printnumber();
	fast_envelope.reset_time();

	int rcd = 0, eq0 = 0, eq02 = 0, rmk = 0;
	for (int i = 0; i < fn; i++) {
		if (pos1[i] - pos2[i] != 0) {
			//if (pos1[i]== 0) {
			rcd = rcd + 1;
			//std::cout << "envelope test different! different face NO. " << i << " the difference: " << pos1[i] - pos2[i] << std::endl;
			//std::cout << "envelope test same! same face NO. " << i << "the in and out : " <<pos1[i] <<","<<pos2[i] << std::endl;
		}
		if (pos1[i] == 0) {
			eq0 = eq0 + 1;
		}
		if (pos2[i] == 0) {
			eq02 = eq02 + 1;
		}
		if (pos1[i] == 0 && pos2[i] == 1) {
			rmk++;
		}
	}

	std::cout << "how many different cases:  " << rcd << std::endl;
	std::cout << "aabb inside triangle number:  " << eq0 << std::endl;
	std::cout << "our  inside triangle number:  " << eq02 << std::endl;
	std::cout << "0-1 cases number " << rmk << std::endl;
	cout << endl;
	// FastEnvelope::print_number(); //TODO

////for aabb method
	Vector3 min, max;

	Scalar dd;
	get_bb_corners(env_vertices, min, max);
	dd = ((max - min).norm()) *eps;
	timer.start();
	AABBWrapper sf_tree(envmesh);
	for (int i = 0; i < fn; i++) {

		is_out_function(triangles[i], dd, sf_tree);
	}
	cout << "aabb time, " << timer.getElapsedTimeInSec() << endl;

	std::cout << "TEST aabb FINISHED  " << std::endl;
	//////////////////////////////
	//////////

}


void sample_triangle_test() {
	string inputFileName = "D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\100029.stl_env.csv";
	string input_surface_path1 = "D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\elevator_and_stabiliser_-_V4.stl";
	vector<int> outenvelope;
	std::vector<std::array<Vector3, 3>> triangles = read_CSV_triangle(inputFileName, outenvelope);
	std::array<Vector3, 3> tri = triangles[10000];
	std::vector<Vector3> ps;
	Scalar l1 = max(max((tri[0] - tri[1]).norm(), (tri[2] - tri[1]).norm()), (tri[0] - tri[2]).norm()) / 10000;
	cout << "l1 " << l1 << endl;
	//FastEnvelope::triangle_sample(tri, ps, l1);
	std::cout << "ps size " << ps.size() << endl;


	std::ofstream fout;
	fout.open("D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\triangle.txt");
	for (int i = 0; i < 3; i++) {

		fout << std::setprecision(17) << tri[i][0] << " " << tri[i][1] << " " << tri[i][2] << endl;

	}
	fout.close();

	fout.open("D:\\vs\\fast_envelope_csv\\thingi10k_debug\\100029\\points.txt");
	for (int i = 0; i < ps.size(); i++) {

		fout << std::setprecision(17) << ps[i][0] << " " << ps[i][1] << " " << ps[i][2] << endl;

	}
	fout.close();

}
template<typename T>
T multyprecision(const T &num1, const T &num2) {
	//int n_digits = 40;

	//using namespace arbitrary_precision;

	//float_precision num1(1, n_digits);
	//float_precision num2(3, n_digits);

	T res = num1 / num2 * T(2) - T(2) / T(3);

	return res;
	//#include<arbitraryprecision/intervalprecision.h>

}



template<typename T>
int test1(T a, T b, const std::function<int(T)> &checker) {
	int flag = checker(a);

	T c(a*b);

	cout << "precision " << c.ref_lower()->precision() << endl;
	if (flag == 0)
		std::cout << "same!" << std::endl;
	std::cout << b << std::endl;

	return 0;
}
void try2() {
	int axxy = 0;
	assert(axxy == 1);
	cout << "here ！@！！！！！！！！！！" << endl;
}
void rational_try(const Rational &r) {
	Rational c = Rational(1) / r;
	cout << Rational(c*r) << endl;
}

void writelist() {
	/*int list[64][2];
	for (int i = 0; i < 64; i++) {
		list[i][0] = -1;
		list[i][1] = -1;
	}
	int m[18][4] = {

	{0,2,0,1},
	{0,3,1,2},
	{0,4,2,3},
	{0,5,3,4},
	{0,6,4,5},

	{0,7,0,5},
	{1,2,6,7},
	{1,3,7,8},
	{1,4,8,9},
	{1,5,9,10},

	{1,6,10,11},
	{1,7,6,11},
	{2,3,1,7},
	{2,7,0,6},
	{3,4,2,8},

	{4,5,3,9},
	{5,6,4,10},
	{6,7,5,11}
	};
	for (int i = 0; i < 18; i++) {
		list[m[i][0] * 8 + m[i][1]][0] = m[i][2];
		list[m[i][0] * 8 + m[i][1]][1] = m[i][3];
		list[m[i][1] * 8 + m[i][0]][0] = m[i][2];
		list[m[i][1] * 8 + m[i][0]][1] = m[i][3];

	}
	cout << "start" << endl;
	for (int i = 0; i < 64; i++) {
		cout <<" {"<< list[i][0] << ", " << list[i][1]<<"}," << endl;
	}
	cout << "end" << endl;*/


	int list[36][2];
	for (int i = 0; i < 36; i++) {
		list[i][0] = -1;
		list[i][1] = -1;
	}
	int m[12][4] = {

	{0,2,0,3},
	{0,3,0,1},
	{0,4,1,2},
	{0,5,2,3},
	{1,2,4,7},

	{1,3,4,5},
	{1,4,5,6},
	{1,5,6,7},
	{2,3,0,4},
	{2,5,3,7},

	{3,4,1,5},
	{4,5,2,6}
	};
	for (int i = 0; i < 12; i++) {
		list[m[i][0] * 6 + m[i][1]][0] = m[i][2];
		list[m[i][0] * 6 + m[i][1]][1] = m[i][3];
		list[m[i][1] * 6 + m[i][0]][0] = m[i][2];
		list[m[i][1] * 6 + m[i][0]][1] = m[i][3];

	}
	cout << "start" << endl;
	for (int i = 0; i < 36; i++) {
		cout << " {" << list[i][0] << ", " << list[i][1] << "}," << endl;
	}
	cout << "end" << endl;

}

#include <gmp.h>
void testM() {
	//{
	//Rational r1(1);
	//Rational r3(3);

	//Rational rr = r1 / r3 / r3;
	//Rational rrr = r3 * rr*r3;

	//std::cout << rrr << std::endl;
	//Scalar p = 1.3;
	//Rational pr(p);
	//pr = pr * pr;
	//pr = pr * pr;
	//pr = pr * pr;
	//double pr1;
	//std::cout << "rr "<<std::setprecision(17) << pr << std::endl;
	//rational_try(Rational(0.7));
	//}

	{
		Multiprecision r1(1);
		Multiprecision r3(3);

		Multiprecision rr = r1 / r3 / r3;
		Multiprecision rrr = r3 * rr * r3;

		std::cout << rrr << std::endl;
		Scalar p = 1.473;
		Multiprecision pr(p);
		pr = pr * pr;//9 6
		pr = pr * pr;//1 12
		pr = Multiprecision(p) * pr;//3 15
		//pr = pr * pr;

		//std::cout << "mp " << pr << std::endl;
		//std::cout << "mp " << std::setprecision(17) << pr << std::endl;
	}

	Rational r = double(1.473);
	r = r * r;
	r = r * r;
	r = 1.473 * r;

	//std::cout << "r  " << std::setprecision(16) << r << std::endl;



	typedef Multiprecision T;
	//typedef Rational T;
	int prec = 100;

	T a;
	a.value->_mp_prec = prec;
	a = T(141414) / T(100000);
	T b;
	b.value->_mp_prec = prec;
	b = T(76) / T(100);
	T result;
	result.value->_mp_prec = prec;
	result = (a * b);
	cout << "result " << result << endl;
	T b1;
	b1.value->_mp_prec = prec;
	b1 = T(76) / T(10);
	T result1;
	result1.value->_mp_prec = prec;
	result1 = (a * b1);
	cout << "result1 " << result1 << endl;
	T n5;
	n5.value->_mp_prec = prec;
	n5 = T(1) / T(10);
	T result2;
	result2.value->_mp_prec = prec;
	result2 = (result1 *n5);
	cout << "result2 " << result2 << endl;
	T minu;
	minu.value->_mp_prec = prec;
	minu = (result - result2);
	cout << "check " << minu.get_sign() << endl;

	if (minu == 0) {
		cout << "=0" << endl;
	}
	Multiprecision multi;
	multi.value->_mp_prec = 100;

	mpf_t x;
	mpf_init2(x, 512);

	cout << "prec " << mpf_get_prec(x) << endl;

	Multiprecision a1(3.14);
	cout << "prec " << a1.get_prec_bits() << endl;

	Multiprecision b2 = 3.14;
	cout << "prec2 " << b2.get_prec_bits() << endl;

}
void try_eigen() {
	Vector3 u;
	u[0] = 0;
	u[1] = 0;
	u[2] = 0;
	Eigen::MatrixXd V = u;
	cout << "row of v " << V.rows() << endl;
}

void tryspeed() {
	std::vector<int> a,b,c,d;
	igl::Timer timer;
	const int size = 100000000;
	std::vector<int> ini;

	for (int i = 0; i < size; i++) {
		ini.push_back(int (rand()));
	}
	timer.start();
	for (int i = 0; i < size; i++) {
		a.push_back(ini[i]);
	}
	cout << "time pure push " << timer.getElapsedTimeInSec() << endl;

	timer.start();
	b.reserve(size);
	for (int i = 0; i < size; i++) {
		b.push_back(ini[i]);
	}
	cout << "time reserve push " << timer.getElapsedTimeInSec() << endl;

	timer.start();
	c.resize(size);
	for (int i = 0; i < size; i++) {
		c[i]=(ini[i]);
	}
	cout << "time reserve push " << timer.getElapsedTimeInSec() << endl;

}



int main(int argc, char const *argv[])
{
	srand(42);

#ifndef WIN32
	setenv("GEO_NO_SIGNAL_HANDLER", "1", 1);
#endif
	GEO::initialize(0);
	GEO::CmdLine::import_arg_group("standard");
	GEO::CmdLine::import_arg_group("pre");
	GEO::CmdLine::import_arg_group("algo");
	

	//test_in_wild(argv[1],argv[2]);
	//test_in_wild();
	/*test_without_sampling();
	test_without_sampling();*/
	test_without_sampling();
	/*bool a=true;
	if (a) cout << "a true" <<a<< endl;
	if (!a) cout << "a false" << endl;*/
	//tryspeed();
	
	/*for (int i = 0; i < (argc - 1) / 2; i++) {
		test_without_sampling(argv[2*i+1], argv[2*i+2]);
		std::cout << argv[2 * i + 1] <<" done!\n" << std::endl;
	}
*/

	//fordebug();












	return 0;
}
