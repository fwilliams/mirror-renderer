#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "utils/tuple.h"

struct VertexFixture {

	VertexFixture() {
	}

	~VertexFixture() {
	}

	static constexpr size_t roundup(size_t n, size_t r) {
		const size_t n_mod_r = n % r;
		return (n_mod_r == 0) ? n : (n + (r - n_mod_r));
	}
};

using namespace glm;
using namespace utils;
using namespace std;

BOOST_FIXTURE_TEST_SUITE(VertexTestFixture, VertexFixture)

BOOST_AUTO_TEST_CASE(test_number_of_elements) {
	typedef Tuple<int> Vertex1;
	typedef Tuple<vec3, float, vec4> Vertex3;
	typedef Tuple<vec3, float, vec4, mat4> Vertex4;
	typedef Tuple<vec3, float, vec4, mat4, vec2> Vertex5;
	typedef Tuple<vec3, float, vec4, mat4, vec2, int, bvec2, mat3, float, int> Vertex10;

	BOOST_CHECK_EQUAL(Vertex1::size(), 1);
	BOOST_CHECK_EQUAL(Vertex3::size(), 3);
	BOOST_CHECK_EQUAL(Vertex4::size(), 4);
	BOOST_CHECK_EQUAL(Vertex5::size(), 5);
	BOOST_CHECK_EQUAL(Vertex10::size(), 10);
}

BOOST_AUTO_TEST_CASE(test_list_element_type) {
	typedef Tuple<vec3, float, vec4> Vertex3;
	typedef Vertex3::ElementType<0> TestType0;
	typedef Vertex3::ElementType<1> TestType1;
	typedef Vertex3::ElementType<2> TestType2;

	bool check = std::is_same<TestType0, vec3>();
	BOOST_CHECK_EQUAL(check, true);

	check = std::is_same<TestType1, float>();
	BOOST_CHECK_EQUAL(check, true);

	check = std::is_same<TestType2, vec4>();
	BOOST_CHECK_EQUAL(check, true);
}

BOOST_AUTO_TEST_CASE(test_static_get) {
	typedef Tuple<vec3, float, vec4> Vertex3;

	Vertex3 v;

	Vertex3::get<0>(v) = vec3(1.0f);
	Vertex3::get<1>(v) = 2.71f;
	Vertex3::get<2>(v) = vec4(1.0f);

	vec3 v3 = Vertex3::get<0>(v);
	float f = Vertex3::get<1>(v);
	vec4 v4 = Vertex3::get<2>(v);
	BOOST_CHECK_EQUAL(f, 2.71f);
	BOOST_CHECK(v3 == vec3(1.0f));
	BOOST_CHECK(v4 == vec4(1.0f));
}

BOOST_AUTO_TEST_CASE(test_method_get) {
	typedef Tuple<vec3, float, vec4> Vertex3;

	Vertex3 v;

	Vertex3::get<0>(v) = vec3(1.0f);
	Vertex3::get<1>(v) = 3.14f;
	Vertex3::get<2>(v) = vec4(1.0f);

	vec3 v3 = v.get<0>();
	float f = v.get<1>();
	vec4 v4 = v.get<2>();
	BOOST_CHECK_EQUAL(f, 3.14f);
	BOOST_CHECK(v3 == vec3(1.0f));
	BOOST_CHECK(v4 == vec4(1.0f));
}

BOOST_AUTO_TEST_CASE(test_packing) {
	typedef Tuple<vec3, float, uint8_t, vec4> Vertex3;

	Vertex3 v;
	v.get<0>() = vec3(1.0f, 2.0f, 3.0f);
	v.get<1>() = 3.14f;
	v.get<2>() = 42;
	v.get<3>() = vec4(4.0f, 5.0f, 6.0f, 7.0f);

	struct S{vec3 v; float f; uint8_t t; vec4 vv;};
	size_t expected = sizeof(S);
	BOOST_CHECK_EQUAL(sizeof(Vertex3), expected);

	size_t offset0 = v.offset<0>();
	size_t offset1 = v.offset<1>();
	size_t offset2 = v.offset<2>();
	size_t offset3 = v.offset<3>();

	uint8_t* pv = reinterpret_cast<uint8_t*>(&v);
	vec3* v0 = reinterpret_cast<vec3*>(&pv[offset0]);
	float* v1 = reinterpret_cast<float*>(&pv[offset1]);
	uint8_t v2 = pv[offset2];
	vec4* v3 = reinterpret_cast<vec4*>(&pv[offset3]);

	BOOST_CHECK(*v0 == vec3(1.0f, 2.0f, 3.0f));
	BOOST_CHECK_EQUAL(*v1, 3.14f);
	BOOST_CHECK_EQUAL(v2, 42);
	BOOST_CHECK(*v3 == vec4(4.0f, 5.0f, 6.0f, 7.0f));
}

BOOST_AUTO_TEST_CASE(test_packing_tight) {
	typedef TightTuple<vec3, float, uint8_t, vec4> Vertex3;

	Vertex3 v;
	v.get<0>() = vec3(1.0f, 2.0f, 3.0f);
	v.get<1>() = 3.14f;
	v.get<2>() = 42;
	v.get<3>() = vec4(4.0f, 5.0f, 6.0f, 7.0f);

	size_t expected = sizeof(vec3) + sizeof(float) + sizeof(vec4) + sizeof(uint8_t);
	BOOST_CHECK_EQUAL(sizeof(Vertex3), expected);

	size_t offset0 = v.offset<0>();
	size_t offset1 = v.offset<1>();
	size_t offset2 = v.offset<2>();
	size_t offset3 = v.offset<3>();

	uint8_t* pv = reinterpret_cast<uint8_t*>(&v);
	vec3* v0 = reinterpret_cast<vec3*>(&pv[offset0]);
	float* v1 = reinterpret_cast<float*>(&pv[offset1]);
	uint8_t v2 = pv[offset2];
	vec4* v3 = reinterpret_cast<vec4*>(&pv[offset3]);

	BOOST_CHECK(*v0 == vec3(1.0f, 2.0f, 3.0f));
	BOOST_CHECK_EQUAL(*v1, 3.14f);
	BOOST_CHECK_EQUAL(v2, 42);
	BOOST_CHECK(*v3 == vec4(4.0f, 5.0f, 6.0f, 7.0f));
}

BOOST_AUTO_TEST_CASE(test_packing_2) {
	typedef Tuple2<vec3, float, uint8_t, vec4> Vertex3;

	Vertex3 v;
	v.get<0>() = vec3(1.0f, 2.0f, 3.0f);
	v.get<1>() = 3.14f;
	v.get<2>() = 42;
	v.get<3>() = vec4(4.0f, 5.0f, 6.0f, 7.0f);

	size_t expected = roundup(sizeof(vec3), 2) +
			roundup(sizeof(float), 2) +
			roundup(sizeof(vec4), 2) +
			roundup(sizeof(uint8_t), 2);
	BOOST_CHECK_EQUAL(sizeof(Vertex3), expected);

	size_t offset0 = v.offset<0>();
	size_t offset1 = v.offset<1>();
	size_t offset2 = v.offset<2>();
	size_t offset3 = v.offset<3>();

	uint8_t* pv = reinterpret_cast<uint8_t*>(&v);
	vec3* v0 = reinterpret_cast<vec3*>(&pv[offset0]);
	float* v1 = reinterpret_cast<float*>(&pv[offset1]);
	uint8_t v2 = pv[offset2];
	vec4* v3 = reinterpret_cast<vec4*>(&pv[offset3]);

	BOOST_CHECK(*v0 == vec3(1.0f, 2.0f, 3.0f));
	BOOST_CHECK_EQUAL(*v1, 3.14f);
	BOOST_CHECK_EQUAL(v2, 42);
	BOOST_CHECK(*v3 == vec4(4.0f, 5.0f, 6.0f, 7.0f));
}

BOOST_AUTO_TEST_CASE(test_packing_4) {
	typedef Tuple4<vec3, float, uint8_t, vec4> Vertex3;

	Vertex3 v;
	v.get<0>() = vec3(1.0f, 2.0f, 3.0f);
	v.get<1>() = 3.14f;
	v.get<2>() = 42;
	v.get<3>() = vec4(4.0f, 5.0f, 6.0f, 7.0f);

	size_t expected = roundup(sizeof(vec3), 4) +
			roundup(sizeof(float), 4) +
			roundup(sizeof(vec4), 4) +
			roundup(sizeof(uint8_t), 4);
	BOOST_CHECK_EQUAL(sizeof(Vertex3), expected);

	size_t offset0 = v.offset<0>();
	size_t offset1 = v.offset<1>();
	size_t offset2 = v.offset<2>();
	size_t offset3 = v.offset<3>();

	uint8_t* pv = reinterpret_cast<uint8_t*>(&v);
	vec3* v0 = reinterpret_cast<vec3*>(&pv[offset0]);
	float* v1 = reinterpret_cast<float*>(&pv[offset1]);
	uint8_t v2 = pv[offset2];
	vec4* v3 = reinterpret_cast<vec4*>(&pv[offset3]);

	BOOST_CHECK(*v0 == vec3(1.0f, 2.0f, 3.0f));
	BOOST_CHECK_EQUAL(*v1, 3.14f);
	BOOST_CHECK_EQUAL(v2, 42);
	BOOST_CHECK(*v3 == vec4(4.0f, 5.0f, 6.0f, 7.0f));
}

BOOST_AUTO_TEST_CASE(test_constructor) {
	typedef Tuple<vec3, float, uint8_t, vec4> Vertex3;

	Vertex3 v(vec3(1.0f, 2.0f, 3.0f), 3.14f, 42, vec4(4.0f, 5.0f, 6.0f, 7.0f));

	size_t expected = roundup(sizeof(vec3), 4) +
			roundup(sizeof(float), 4) +
			roundup(sizeof(vec4), 4) +
			roundup(sizeof(uint8_t), 4);
	BOOST_CHECK_EQUAL(sizeof(Vertex3), expected);

	size_t offset0 = v.offset<0>();
	size_t offset1 = v.offset<1>();
	size_t offset2 = v.offset<2>();
	size_t offset3 = v.offset<3>();

	uint8_t* pv = reinterpret_cast<uint8_t*>(&v);
	vec3* v0 = reinterpret_cast<vec3*>(&pv[offset0]);
	float* v1 = reinterpret_cast<float*>(&pv[offset1]);
	uint8_t v2 = pv[offset2];
	vec4* v3 = reinterpret_cast<vec4*>(&pv[offset3]);

	BOOST_CHECK(*v0 == vec3(1.0f, 2.0f, 3.0f));
	BOOST_CHECK_EQUAL(*v1, 3.14f);
	BOOST_CHECK_EQUAL(v2, 42);
	BOOST_CHECK(*v3 == vec4(4.0f, 5.0f, 6.0f, 7.0f));
}

BOOST_AUTO_TEST_CASE(test_new_delete) {
	typedef Tuple<vec3, float, uint8_t, vec4> Vertex3;
	Vertex3* v = new Vertex3();

	v->get<0>() = vec3(1.0);
	v->get<1>() = 3.14f;
	v->get<2>() = 42;
	v->get<3>() = vec4(2.0);

	BOOST_CHECK(v->get<0>() == vec3(1.0));
	BOOST_CHECK_EQUAL(v->get<1>(), 3.14f);
	BOOST_CHECK_EQUAL(v->get<2>(), 42);
	BOOST_CHECK(v->get<3>() == vec4(2.0));

	delete v;
}

BOOST_AUTO_TEST_CASE(test_new_delete_array) {
	typedef Tuple<vec3, float, uint8_t, vec4> Vertex3;
	Vertex3* v = new Vertex3[100];

	for(size_t i = 0; i < 100; i++) {
		v[i].get<0>() = vec3(1.0);
		v[i].get<1>() = 3.14f;
		v[i].get<2>() = 42;
		v[i].get<3>() = vec4(2.0);
	}

	for(size_t i = 0; i < 100; i++) {
		BOOST_CHECK(v[i].get<0>() == vec3(1.0));
		BOOST_CHECK_EQUAL(v[i].get<1>(), 3.14f);
		BOOST_CHECK_EQUAL(v[i].get<2>(), 42);
		BOOST_CHECK(v[i].get<3>() == vec4(2.0));
	}

	delete[] v;
}

BOOST_AUTO_TEST_SUITE_END()
