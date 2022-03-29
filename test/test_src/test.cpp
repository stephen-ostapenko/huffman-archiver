#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "arg_utils.h"
#include "bitio.h"
#include "hufftree.h"
#include "huffman.h"
#include <cstddef>
#include <cstring>
#include <random>
#include <vector>
#include <sstream>
#include <functional>

using std::size_t;
using std::mt19937;
using std::vector;
using std::string;
using std::stringstream;
using std::istream;
using std::exception;
using std::invalid_argument;

using arg_utils::Arguments;
using arg_utils::process_args;

using bit_io::BitInputStream;
using bit_io::BitOutputStream;

using huff_tree::CHARS_CNT;
using huff_tree::CharCounter;
using huff_tree::HuffTree;

using huffman::HuffmanArchiver;
using huffman::HuffmanDearchiver;
using huffman::HuffFileData;
using huffman::invalid_file_format;

TEST_SUITE("test arg_utils") {
	TEST_CASE("test missing target") {
		const size_t N = 5;
		const char *argv[N]{"hw_02", "-f", "a", "-o", "b"};

		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple targets 1") {
		const size_t N = 7;
		const char *argv[N]{"hw_02", "-c", "-u", "-f", "a", "-o", "b"};
	
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple targets 2") {
		const size_t N = 7;
		const char *argv[N]{"hw_02", "-c", "-c", "-f", "a", "-o", "b"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test missing input file") {
		const size_t N = 4;
		const char *argv[N]{"hw_02", "-c", "-o", "b"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple input files 1") {
		const size_t N = 8;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "-f", "b", "-o", "c"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple input files 2") {
		const size_t N = 8;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "--file", "b", "-o", "c"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test missing output file") {
		const size_t N = 4;
		const char *argv[N]{"hw_02", "-c", "-f", "a"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple output files 1") {
		const size_t N = 8;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "-o", "b", "-o", "c"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test multiple output files 2") {
		const size_t N = 8;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "-o", "b", "--file", "c"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test input file is the same as output file") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-u", "-f", "a", "-o", "a"};
		
		CHECK_THROWS_AS(process_args(N, argv), invalid_argument);
	}

	TEST_CASE("test correct input 1") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "-o", "b"};

		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-c");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 2") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-c", "--file", "a", "-o", "b"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-c");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 3") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-c", "-f", "a", "--output", "b"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-c");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 4") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-c", "--file", "a", "--output", "b"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-c");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 5") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-u", "-f", "a", "-o", "b"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-u");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 6") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-u", "-o", "b", "--file", "a"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-u");
		CHECK(args.get_input_file() == "a");
		CHECK(args.get_output_file() == "b");
	}

	TEST_CASE("test correct input 7") {
		const size_t N = 6;
		const char *argv[N]{"hw_02", "-o", "a", "-f", "b", "-u"};
		
		Arguments args = process_args(N, argv);
		CHECK(args.get_target() == "-u");
		CHECK(args.get_input_file() == "b");
		CHECK(args.get_output_file() == "a");
	}
}

TEST_SUITE("test bit_io") {
	TEST_CASE("test bit_io 1") {
		const size_t N = 24;
		bool arr[N]{1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0};

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
		const size_t N = 26;
		bool arr[N]{0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1};

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

		CHECK_THROWS_WITH_AS(bi.read_bit(), "no bits left in input: iostream error", istream::failure);
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

		CHECK_THROWS_WITH_AS(bi.read_bit(), "no bits left in input: iostream error", istream::failure);
	}

	TEST_CASE("test bit_io 5") {
		stringstream str;

		{
			BitOutputStream bo(str);
		}

		try {	
			BitInputStream bi(str);
		} catch (std::istream::failure &e) {
			CHECK(!std::strcmp(e.what(), "no bits left in input: iostream error"));
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
		string res = "Hello, world!";

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
		string res = "Huffman algo";

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
		string res = "!@#$%^&*";

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
		const size_t N = CHARS_CNT;

		for (size_t i = 0; i < N; i++) {
			for (size_t j = 0; j <= i; j++) {
				cnt.add_char((char)i);
			}
		}

		for (size_t i = 0; i < N; i++) {
			CHECK(i + 1 == cnt.get_char_cnt(i));
		}
	}

	TEST_CASE("test 5") {
		CharCounter cnt;
		for (size_t i = 0; i < CHARS_CNT; i++) {
			CHECK(cnt.get_char_cnt(i) == 0);
		}
	}
}

TEST_SUITE("test HuffTree") {
	const size_t TREE_SZ = CHARS_CNT * 2 - 1;

	void load_chars(CharCounter &cnt, mt19937 &mtw, size_t mod = 256, size_t N = 10000) {
		for (size_t i = 0; i < N; i++) {
			char c = mtw() % mod;
			cnt.add_char(c);
		}
	}

	size_t get_full_length(const CharCounter &cnt, const HuffTree &t) {
		size_t sz = 0;
		for (size_t i = 0; i < CHARS_CNT; i++) {
			sz += cnt.get_char_cnt(i) * t.get_char_code(i).size();
		}
		return sz;
	}

	vector <bool> get_bit_char_seq(vector <unsigned char> &s) {
		vector <bool> result;
		for (auto c : s) {
			for (size_t i = 0; i < CHAR_BIT; i++) {
				result.push_back(c & (1ll << i));
			}
		}
		return result;
	}

	vector <unsigned char> gen_char_permutation(mt19937 &mtw) {
		vector <unsigned char> p(CHARS_CNT);
		std::iota(p.begin(), p.end(), 0); std::shuffle(p.begin(), p.end(), mtw);
		return p;
	}

	vector <bool> gen_random_tree(mt19937 &mtw) {
		vector <int> gr[TREE_SZ], cur_v(CHARS_CNT);
		std::iota(cur_v.begin(), cur_v.end(), 0);
		size_t p = CHARS_CNT;

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
	const size_t TREE_SZ = CHARS_CNT * 2 - 1;

	vector <unsigned char> gen_char_permutation(mt19937 &mtw) {
		vector <unsigned char> p(CHARS_CNT);
		std::iota(p.begin(), p.end(), 0); std::shuffle(p.begin(), p.end(), mtw);
		return p;
	}

	vector <bool> gen_random_tree(mt19937 &mtw) {
		vector <int> gr[TREE_SZ], cur_v(CHARS_CNT);
		std::iota(cur_v.begin(), cur_v.end(), 0);
		size_t p = CHARS_CNT;

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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 3") {
		stringstream src, arch, res;
		src << "kek";

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 4") {
		stringstream src, arch, res;
		for (size_t i = 0; i < 37; i++) {
			src << (char)i;
		}

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 5") {
		stringstream src, arch, res;
		for (size_t i = 256; i >= 228; i--) {
			src << (char)i;
		}

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
		CHECK(x.input_sz == y.output_sz);
		CHECK(x.output_sz == y.input_sz);
		CHECK(x.additional_sz == y.additional_sz);
	}

	TEST_CASE("test statistics 6") {
		stringstream src, arch, res;
		for (size_t i = 0; i < 256; i += 2) {
			src << (char)i;
		}

		HuffmanArchiver a;
		HuffFileData x = a.archive(src, arch);

		HuffmanDearchiver d;
		HuffFileData y = d.dearchive(arch, res);

		CHECK(src.str().size() == x.input_sz);
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		CHECK(arch.str().size() == x.output_sz + x.additional_sz);
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
		stringstream src, arch, res;
		src << "kek";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 4") {
		stringstream src, arch, res;
		for (size_t i = 0; i < 37; i++) {
			src << (char)i;
		}

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 5") {
		stringstream src, arch, res;
		for (size_t i = 256; i >= 228; i--) {
			src << (char)i;
		}

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test archive/dearchive 6") {
		stringstream src, arch, res;
		for (size_t i = 0; i < 256; i += 2) {
			src << (char)i;
		}

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test empty file archive/dearchive") {
		stringstream src, arch, res;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file archive/dearchive 1") {
		stringstream src, arch, res;
		src << "a";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file archive/dearchive 2") {
		stringstream src, arch, res;
		src << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
	}

	TEST_CASE("test 1-byte file archive/dearchive 3") {
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

	TEST_CASE("test 1-byte file archive/dearchive 4") {
		stringstream src, arch, res;
		src << (char)3;

		HuffmanArchiver a;
		a.archive(src, arch);

		HuffmanDearchiver d;
		d.dearchive(arch, res);

		CHECK(src.str() == res.str());
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
				size_t ceiled_tree_size_in_bytes = (tree.size() + 7) / 8;
				size_t tree_tail_bits = ceiled_tree_size_in_bytes * 8 - tree.size();
				for (size_t i = 0; i < tree_tail_bits; i++) {
					bo.write_bit(0);
				}

				size_t sz = 2121;
				for (size_t i = 0; i < sizeof(size_t) * 8; i++) {
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