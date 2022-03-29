#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "arg_utils.h"
#include "bitio.h"
#include "hufftree.h"
#include "huffman.h"
#include <cstddef>
#include <random>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

using std::size_t;
using std::mt19937;
using std::vector;
using std::string;
using std::string_view;
using std::stringstream;
using std::exception;
using std::invalid_argument;

using arg_utils::convert_args;
using arg_utils::ex_assert;
using arg_utils::validate_args;
using arg_utils::get_mode;
using arg_utils::get_input_file;
using arg_utils::get_output_file;

using bit_io::BitInputStream;
using bit_io::BitOutputStream;
using bit_io::out_of_bits;

using huff_tree::CharCounter;
using huff_tree::HuffTree;

using huffman::HuffmanArchiver;
using huffman::HuffmanDearchiver;
using huffman::HuffFileData;
using huffman::invalid_file_format;

const size_t BITS_IN_CHAR = sizeof(char) * 8;

TEST_SUITE("test arg_utils") {
	TEST_CASE("test convert_args 1") {
		const char *argv[6]{"hw_02", "kek1", "kek2", "kek3", "kek4", "kek5"};

		vector <string_view> v = convert_args(6, argv);
		vector <string_view> v2 = {
			string_view(argv[1]),
			string_view(argv[2]),
			string_view(argv[3]),
			string_view(argv[4]),
			string_view(argv[5])
		};

		REQUIRE(v.size() == 5);
		for (size_t i = 0; i < 5; i++) {
			CHECK(v[i] == v2[i]);
		}
	}

	TEST_CASE("test convert_args 2") {
		const char *argv[3]{"hw_02", "abacaba", "abracadabra"};

		vector <string_view> v = convert_args(3, argv);
		vector <string_view> v2 = {
			string_view(argv[1]),
			string_view(argv[2])
		};

		REQUIRE(v.size() == 2);
		for (size_t i = 0; i < 2; i++) {
			CHECK(v[i] == v2[i]);
		}
	}

	TEST_CASE("test convert_args 3") {
		const char *argv[3]{"hw_02"};

		vector <string_view> v = convert_args(1, argv);

		REQUIRE(v.size() == 0);
	}

	TEST_CASE("test ex_assert 1") {
		CHECK_THROWS_WITH_AS(ex_assert(0, "test message"), "test message", invalid_argument);
	}

	TEST_CASE("test ex_assert 2") {
		ex_assert(1, "test message");
	}

	TEST_CASE("test validate_args 1") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f")
		};

		CHECK_THROWS_WITH_AS(validate_args(args), "Too few/too many args", invalid_argument);
	}

	TEST_CASE("test validate_args 2") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("-o"),
			string_view("-u")
		};

		CHECK_THROWS_WITH_AS(validate_args(args), "Missing target or too many targets (-c or -u)", invalid_argument);
	}

	TEST_CASE("test validate_args 3") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("lol"),
			string_view("kek"),
			string_view("-o"),
			string_view("kek2")
		};

		CHECK_THROWS_WITH_AS(validate_args(args), "Missing input file flag or too many input file flags (-f)", invalid_argument);
	}

	TEST_CASE("test validate_args 4") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("kek2"),
			string_view("kek3")
		};

		CHECK_THROWS_WITH_AS(validate_args(args), "Missing output file flag or too many output file flags (-o)", invalid_argument);
	}

	TEST_CASE("test validate_args 5") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		validate_args(args);
	}

	TEST_CASE("test validate_args 6") {
		vector <string_view> args = {
			string_view("-u"),
			string_view("-f"),
			string_view("arch"),
			string_view("-o"),
			string_view("kek2")
		};

		validate_args(args);
	}

	TEST_CASE("test validate_args 7") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("--file"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		validate_args(args);
	}

	TEST_CASE("test validate_args 8") {
		vector <string_view> args = {
			string_view("-u"),
			string_view("-f"),
			string_view("arch"),
			string_view("--output"),
			string_view("kek2")
		};

		validate_args(args);
	}

	TEST_CASE("test get_mode 1") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		CHECK(get_mode(args) == false);
	}

	TEST_CASE("test get_mode 2") {
		vector <string_view> args = {
			string_view("-u"),
			string_view("-f"),
			string_view("arch"),
			string_view("-o"),
			string_view("kek2")
		};

		CHECK(get_mode(args) == true);
	}

	TEST_CASE("test get_input_file 1") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		CHECK(get_input_file(args) == "kek");
	}

	TEST_CASE("test get_input_file 2") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-o"),
			string_view("kek"),
			string_view("-f"),
			string_view("arch")
		};

		CHECK(get_input_file(args) == "arch");
	}

	TEST_CASE("test get_input_file 3") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("--file"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		CHECK(get_input_file(args) == "kek");
	}

	TEST_CASE("test get_input_file 4") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-o"),
			string_view("kek"),
			string_view("lol"),
			string_view("-f")
		};

		CHECK_THROWS_WITH_AS(get_input_file(args), "Missing input file (-f)", invalid_argument);
	}

	TEST_CASE("test get_output_file 1") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("-o"),
			string_view("arch")
		};

		CHECK(get_output_file(args) == "arch");
	}

	TEST_CASE("test get_output_file 2") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-o"),
			string_view("kek"),
			string_view("-f"),
			string_view("arch")
		};

		CHECK(get_output_file(args) == "kek");
	}

	TEST_CASE("test get_output_file 3") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("--output"),
			string_view("arch")
		};

		CHECK(get_output_file(args) == "arch");
	}

	TEST_CASE("test get_output_file 4") {
		vector <string_view> args = {
			string_view("-c"),
			string_view("-f"),
			string_view("kek"),
			string_view("lol"),
			string_view("-o")
		};

		CHECK_THROWS_WITH_AS(get_output_file(args), "Missing output file (-o)", invalid_argument);
	}
}

TEST_SUITE("test bit_io") {
	TEST_CASE("test bit_io 1") {
		const size_t N = 8000;
		mt19937 mtw(0);
		bool arr[N];
		for (size_t i = 0; i < N; i++) {
			arr[i] = mtw() & 1;
		}

		stringstream str;

		{
			BitOutputStream bo(str);
			for (size_t i = 0; i < N; i++) {
				bo.write_bit(arr[i]);
			}
		}

		BitInputStream bi(str);
		for (size_t i = 0; i < N; i++) {
			CHECK(bi.read_bit() == arr[i]);
		}
	}

	TEST_CASE("test bit_io 2") {
		const size_t N = 5321;
		mt19937 mtw(0);
		bool arr[N];
		for (size_t i = 0; i < N; i++) {
			arr[i] = mtw() & 1;
		}

		stringstream str;

		{
			BitOutputStream bo(str);
			for (size_t i = 0; i < N; i++) {
				bo.write_bit(arr[i]);
			}
		}

		BitInputStream bi(str);
		for (size_t i = 0; i < N; i++) {
			CHECK(bi.read_bit() == arr[i]);
		}
	}

	TEST_CASE("test bit_io 3") {
		stringstream str;

		{
			BitOutputStream bo(str);
			bo.write_bit(0);
			bo.write_bit(1);
			bo.write_bit(0);
			bo.write_bit(1);
			bo.write_bit(0);
			bo.write_bit(1);
			bo.write_bit(0);
		}

		BitInputStream bi(str);
		for (size_t i = 0; i < 7; i++) {
			CHECK(bi.read_bit() == (i & 1));
		}
		bi.read_bit();

		CHECK_THROWS_WITH_AS(bi.read_bit(), "no bits left in input: iostream error", out_of_bits);
	}

	TEST_CASE("test bit_io 4") {
		stringstream str;

		{
			BitOutputStream bo(str);
			bo.write_bit(0);
		}

		BitInputStream bi(str);
		for (size_t i = 0; i < 8; i++) {
			bi.read_bit();
		}

		CHECK_THROWS_WITH_AS(bi.read_bit(), "no bits left in input: iostream error", out_of_bits);
	}

	TEST_CASE("test bit_io 5") {
		stringstream str;

		{
			BitOutputStream bo(str);
		}

		try {	
			BitInputStream bi(str);
		} catch (out_of_bits &e) {
			CHECK(!strcmp(e.what(), "no bits left in input: iostream error"));
			return;
		} catch (exception &e) {
			CHECK(0);
		}

		CHECK(0);
	}
}

TEST_SUITE("test CharCounter") {
	TEST_CASE("test 1") {
		CharCounter cnt;
		std::string res = "Hello, world!";

		for (char c : res) {
			cnt.add_char(c);
		}

		CHECK(cnt.get_char_cnt('H') == 1);
		CHECK(cnt.get_char_cnt('e') == 1);
		CHECK(cnt.get_char_cnt('l') == 3);
		CHECK(cnt.get_char_cnt('o') == 2);
		CHECK(cnt.get_char_cnt(',') == 1);
		CHECK(cnt.get_char_cnt(' ') == 1);
		CHECK(cnt.get_char_cnt('w') == 1);
		CHECK(cnt.get_char_cnt('r') == 1);
		CHECK(cnt.get_char_cnt('d') == 1);
		CHECK(cnt.get_char_cnt('!') == 1);
	}

	TEST_CASE("test 2") {
		CharCounter cnt;
		std::string res = "Huffman algo";

		for (char c : res) {
			cnt.add_char(c);
		}

		CHECK(cnt.get_char_cnt('H') == 1);
		CHECK(cnt.get_char_cnt('u') == 1);
		CHECK(cnt.get_char_cnt('f') == 2);
		CHECK(cnt.get_char_cnt('m') == 1);
		CHECK(cnt.get_char_cnt('a') == 2);
		CHECK(cnt.get_char_cnt('n') == 1);
		CHECK(cnt.get_char_cnt(' ') == 1);
		CHECK(cnt.get_char_cnt('l') == 1);
		CHECK(cnt.get_char_cnt('g') == 1);
		CHECK(cnt.get_char_cnt('o') == 1);
	}

	TEST_CASE("test 3") {
		CharCounter cnt;
		std::string res = "!@#$%^&*";

		for (char c : res) {
			cnt.add_char(c);
		}

		CHECK(cnt.get_char_cnt('!') == 1);
		CHECK(cnt.get_char_cnt('@') == 1);
		CHECK(cnt.get_char_cnt('#') == 1);
		CHECK(cnt.get_char_cnt('$') == 1);
		CHECK(cnt.get_char_cnt('%') == 1);
		CHECK(cnt.get_char_cnt('^') == 1);
		CHECK(cnt.get_char_cnt('&') == 1);
		CHECK(cnt.get_char_cnt('*') == 1);
	}

	TEST_CASE("test 4") {
		CharCounter cnt;
		size_t act[256]{};
		const size_t N = 10000;
		mt19937 mtw(0);

		for (size_t i = 0; i < N; i++) {
			char c = mtw();
			act[(unsigned char)c]++; cnt.add_char(c);
		}

		for (size_t i = 0; i < (1 << BITS_IN_CHAR); i++) {
			CHECK(act[i] == cnt.get_char_cnt(i));
		}
	}

	TEST_CASE("test 5") {
		CharCounter cnt;
		for (size_t i = 0; i < (1 << BITS_IN_CHAR); i++) {
			CHECK(cnt.get_char_cnt(i) == 0);
		}
	}
}

TEST_SUITE("test HuffTree") {
	const size_t TREE_SZ = ((1 << BITS_IN_CHAR) * 2 - 1);

	void load_chars(CharCounter &cnt, mt19937 &mtw, size_t mod = 256, size_t N = 10000) {
		for (size_t i = 0; i < N; i++) {
			char c = mtw() % mod;
			cnt.add_char(c);
		}
	}

	size_t get_full_length(const CharCounter &cnt, const HuffTree &t) {
		size_t sz = 0;
		for (size_t i = 0; i < (1 << BITS_IN_CHAR); i++) {
			sz += cnt.get_char_cnt(i) * t.get_char_code(i).size();
		}
		return sz;
	}

	vector <bool> get_bit_char_seq(vector <unsigned char> &s) {
		vector <bool> result;
		for (auto c : s) {
			for (size_t i = 0; i < BITS_IN_CHAR; i++) {
				result.push_back(c & (1ll << i));
			}
		}
		return result;
	}

	vector <unsigned char> gen_char_permutation(mt19937 &mtw) {
		vector <unsigned char> p(1 << BITS_IN_CHAR);
		std::iota(p.begin(), p.end(), 0); std::shuffle(p.begin(), p.end(), mtw);
		return p;
	}

	vector <bool> gen_random_tree(mt19937 &mtw) {
		vector <int> gr[TREE_SZ], cur_v(1 << BITS_IN_CHAR);
		std::iota(cur_v.begin(), cur_v.end(), 0);
		size_t p = 1 << BITS_IN_CHAR;

		while (cur_v.size() > 1) {
			int a = mtw() % (cur_v.size() - 1) + 1;
			int b = mtw() % a;
			
			gr[p] = {cur_v[a], cur_v[b]};
			cur_v.erase(cur_v.begin() + a);
			cur_v.erase(cur_v.begin() + b);
			cur_v.push_back(p++);
		}

		vector <bool> result;
		std::function<void(int)> dfs = [&](int v) -> void {
			for (int u : gr[v]) {
				result.push_back(0);
				dfs(u);
				result.push_back(1);
			}
		};
		dfs(cur_v[0]);

		return result;
	}

	TEST_CASE("test rebuild 1") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(1);

		load_chars(cnt, mtw);
		t.rebuild(cnt);

		CHECK(get_full_length(cnt, t) == 79974);
	}

	TEST_CASE("test rebuild 2") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(2);

		load_chars(cnt, mtw);
		t.rebuild(cnt);

		CHECK(get_full_length(cnt, t) == 79991);
	}

	TEST_CASE("test rebuild 3") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(3);
		
		load_chars(cnt, mtw, 2);
		t.rebuild(cnt);

		CHECK(get_full_length(cnt, t) == 14968);
	}

	TEST_CASE("test rebuild 4") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(4);
		
		load_chars(cnt, mtw, 5);
		t.rebuild(cnt);

		CHECK(get_full_length(cnt, t) == 25952);
	}

	TEST_CASE("test rebuild 5") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(5);
		
		load_chars(cnt, mtw, 1);
		t.rebuild(cnt);

		CHECK(get_full_length(cnt, t) == 10000);
	}

	TEST_CASE("test multiple rebuilds") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(24);

		load_chars(cnt, mtw);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);

		t.rebuild(cnt);
		CHECK(get_full_length(cnt, t) == 79988);
	}

	TEST_CASE("test tree length") {
		HuffTree t;
		CharCounter cnt;
		mt19937 mtw(22);

		load_chars(cnt, mtw);

		t.rebuild(cnt);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(cnt);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(cnt);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(cnt);
		CHECK(t.get_compressed_tree().size() == 3068);
	}

	TEST_CASE("test rebuild from file 1") {
		HuffTree t;
		mt19937 mtw(28);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);
	}

	TEST_CASE("test rebuild from file 2") {
		HuffTree t;
		mt19937 mtw(11);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);
	}

	TEST_CASE("test rebuild from file 3") {
		HuffTree t;
		mt19937 mtw(19);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);

		t.rebuild(p, tree);
		CHECK(t.get_compressed_tree().size() == 3068);
	}

	TEST_CASE("test rebuild from file with get_compressed_tree 1") {
		HuffTree t;
		mt19937 mtw(24);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);

		vector <bool> compressed = get_bit_char_seq(p);
		std::copy(tree.begin(), tree.end(), std::back_inserter(compressed));

		CHECK(compressed == t.get_compressed_tree());
	}

	TEST_CASE("test rebuild from file with get_compressed_tree 2") {
		HuffTree t;
		mt19937 mtw(22);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);

		vector <bool> compressed = get_bit_char_seq(p);
		std::copy(tree.begin(), tree.end(), std::back_inserter(compressed));

		CHECK(compressed == t.get_compressed_tree());
	}

	TEST_CASE("test rebuild from file with get_compressed_tree 3") {
		HuffTree t;
		mt19937 mtw(28);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);

		vector <bool> compressed = get_bit_char_seq(p);
		std::copy(tree.begin(), tree.end(), std::back_inserter(compressed));

		CHECK(compressed == t.get_compressed_tree());
	}

	TEST_CASE("test rebuild from file with get_compressed_tree 4") {
		HuffTree t;
		mt19937 mtw(11);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);

		vector <bool> compressed = get_bit_char_seq(p);
		std::copy(tree.begin(), tree.end(), std::back_inserter(compressed));

		CHECK(compressed == t.get_compressed_tree());
	}

	TEST_CASE("test rebuild from file with get_compressed_tree 5") {
		HuffTree t;
		mt19937 mtw(19);

		vector <unsigned char> p = gen_char_permutation(mtw);
		vector <bool> tree = gen_random_tree(mtw);

		t.rebuild(p, tree);

		vector <bool> compressed = get_bit_char_seq(p);
		std::copy(tree.begin(), tree.end(), std::back_inserter(compressed));

		CHECK(compressed == t.get_compressed_tree());
	}
}

TEST_SUITE("test HuffmanArchiver and HuffmanDearchiver") {
	const size_t TREE_SZ = ((1 << BITS_IN_CHAR) * 2 - 1);

	vector <unsigned char> gen_char_permutation(mt19937 &mtw) {
		vector <unsigned char> p(1 << BITS_IN_CHAR);
		std::iota(p.begin(), p.end(), 0); std::shuffle(p.begin(), p.end(), mtw);
		return p;
	}

	vector <bool> gen_random_tree(mt19937 &mtw) {
		vector <int> gr[TREE_SZ], cur_v(1 << BITS_IN_CHAR);
		std::iota(cur_v.begin(), cur_v.end(), 0);
		size_t p = 1 << BITS_IN_CHAR;

		while (cur_v.size() > 1) {
			int a = mtw() % (cur_v.size() - 1) + 1;
			int b = mtw() % a;
			
			gr[p] = {cur_v[a], cur_v[b]};
			cur_v.erase(cur_v.begin() + a);
			cur_v.erase(cur_v.begin() + b);
			cur_v.push_back(p++);
		}

		vector <bool> result;
		std::function<void(int)> dfs = [&](int v) -> void {
			for (int u : gr[v]) {
				result.push_back(0);
				dfs(u);
				result.push_back(1);
			}
		};
		dfs(cur_v[0]);

		return result;
	}

	TEST_CASE("test statistics 1") {
		stringstream src, arch, res;
		src << "Hello, World!";

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 2") {
		stringstream src, arch, res;
		src << "ahahahahahahahhahahahahahahahahahahahaha";

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 3") {
		const int N = 1024;
		mt19937 mtw(24);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 256);
		}
		src << s;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 4") {
		const int N = 1011;
		mt19937 mtw(22);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 256);
		}
		src << s;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 5") {
		const int N = 1011;
		mt19937 mtw(19);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 3);
		}
		src << s;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 6") {
		const int N = 11111;
		mt19937 mtw(11);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 11);
		}
		src << s;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test empty file statistics") {
		stringstream src, arch, res;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test 1-byte file statistics 1") {
		stringstream src, arch, res;
		src << "a";

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test 1-byte file statistics 2") {
		stringstream src, arch, res;
		src << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test 1-byte file statistics 3") {
		const size_t N = 5555;
		stringstream src, arch, res;
		
		char c = 24;
		for (size_t i = 0; i < N; i++) {
			src << c;
		}

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test 1-byte file statistics 4") {
		stringstream src, arch, res;
		src << (char)3;

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test archive/dearchive 1") {
		stringstream src, arch, res;
		src << "Hello, World!";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 2") {
		stringstream src, arch, res;
		src << "ahahahahahahahhahahahahahahahahahahahaha";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());	
	}

	TEST_CASE("test archive/dearchive 3") {
		const int N = 1024;
		mt19937 mtw(24);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 256);
		}
		src << s;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 4") {
		const int N = 1011;
		mt19937 mtw(22);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 256);
		}
		src << s;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 5") {
		const int N = 1011;
		mt19937 mtw(19);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 3);
		}
		src << s;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 6") {
		const int N = 11111;
		mt19937 mtw(11);
		stringstream src, arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 11);
		}
		src << s;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test empty file") {
		stringstream src, arch, res;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file 1") {
		stringstream src, arch, res;
		src << "a";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file 2") {
		stringstream src, arch, res;
		src << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file 3") {
		const size_t N = 5555;
		stringstream src, arch, res;
		
		char c = 24;
		for (size_t i = 0; i < N; i++) {
			src << c;
		}

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file 4") {
		stringstream src, arch, res;
		src << (char)3;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test bad archive") {
		const int N = 1011;
		mt19937 mtw(19);
		stringstream arch, res;
		string s;

		for (size_t i = 0; i < N; i++) {
			s.push_back(mtw() % 256);
		}
		arch << s;

		HuffmanDearchiver d;
		CHECK_THROWS_AS(d.dearchive(arch, res), invalid_file_format);
	}

	TEST_CASE("test 100 bad archives") {
		const int N = 5017;
		mt19937 mtw(17);

		for (size_t it = 0; it < 100; it++) {
			stringstream arch, res;
			string s;

			for (size_t i = 0; i < N; i++) {
				s.push_back(mtw() % 256);
			}
			arch << s;

			HuffmanDearchiver d;
			CHECK_THROWS_AS(d.dearchive(arch, res), invalid_file_format);
		}
	}

	TEST_CASE("test archive undersize") {
		mt19937 mtw(21);

		for (size_t it = 0; it < 100; it++) {
			stringstream arch, res;

			vector <unsigned char> ch_perm = gen_char_permutation(mtw);
			for (char c : ch_perm) {
				arch << c;
			}

			vector <bool> tree = gen_random_tree(mtw);
			{
				BitOutputStream bo(arch);
				for (bool b : tree) {
					bo.write_bit(b);
				}

				size_t sz = 1ll << 22;
				for (size_t i = 0; i < sizeof(sz) * 8; i++) {
					bo.write_bit(sz & ((size_t)1 << i));
				}

				for (size_t i = 0; i < 9; i++) {
					bo.write_bit(0);
				}
			}

			HuffmanDearchiver d;
			CHECK_THROWS_WITH_AS(d.dearchive(arch, res), "too few bits in input file", invalid_file_format);
		}
	}
}