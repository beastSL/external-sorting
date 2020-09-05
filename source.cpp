#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <set>
#include <climits>
#include <iterator>
#include <cstdio>
#include <memory>
#include <stack>

using namespace std;

const int MDIVB = 400;
const int BL_SIZE = 1e7;
int n;

stack<string> filenames;

void write(ofstream& filename, const vector<int>& vec, int size) {
	filename.write(reinterpret_cast<const char*>(vec.data()), sizeof(int) * size);
}

void read(ifstream& filename, vector<int>& vec, int size) {
	vec.resize(size, 0);
	filename.read(reinterpret_cast<char*>(vec.data()), sizeof(int) * size);
}

void delete_file() {
	string filename = filenames.top();
	filenames.pop();
	remove(filename.c_str());
}

void generate_input_file(int n) {
	ofstream fout("input.bin", ofstream::binary);
	mt19937 rnd(random_device{}());
	vector<int> to_write;
	for (int i = 0; i < n; i++) {
		//cout << i << ' ';
		to_write.push_back(rnd());
		//cout << to_write.back() << '\n';
		if (to_write.size() == BL_SIZE) {
			write(fout, to_write, BL_SIZE);
			to_write.clear();
		}
	}
	write(fout, to_write, to_write.size());
	fout.close();
}

int split_into_sorted_blocks(int n) {
	ifstream fin("input.bin", ifstream::binary);
	int bl_enum = 0;
	int i = 0;
	while (i < n) {
		vector<int> cur_block;
		if (i + BL_SIZE > n) {
			read(fin, cur_block, n - i);
			while (cur_block.size() < BL_SIZE) {
				cur_block.push_back(INT_MAX);
			}
			ofstream block(to_string(bl_enum++) + ".bin", ofstream::binary);
			sort(cur_block.begin(), cur_block.end());
			while (cur_block.size() < BL_SIZE) {
				cur_block.push_back(INT_MAX);
			}
			write(block, cur_block, BL_SIZE);
			block.close();
		}
		else {
			read(fin, cur_block, BL_SIZE);
			ofstream block(to_string(bl_enum++) + ".bin", ofstream::binary);
			sort(cur_block.begin(), cur_block.end());
			write(block, cur_block, BL_SIZE);
			block.close();
		}
		i += BL_SIZE;
	}
	fin.close();
	return bl_enum;
}

void output_bin(ifstream& file, int n);

ifstream sort_k_blocks(vector<ifstream>& blocks, int l, int r) {
	ofstream cur(to_string(l) + '$' + to_string(r) + ".bin", ofstream::binary);
	vector<int> to_write;
	set<pair<int, int>> heap;
	vector<vector<int>> rd(blocks.size(), vector<int>(0));
	vector<int> indexes(blocks.size(), 1);
	for (int i = 0; i < blocks.size(); i++) {
		read(blocks[i], rd[i], BL_SIZE);
		heap.insert({ rd[i][0], i });
	}
	while (!heap.empty()) {
		auto p = *heap.begin();
		heap.erase(heap.begin());
		to_write.push_back(p.first);
		if (to_write.size() == BL_SIZE) {
			write(cur, to_write, BL_SIZE);
			to_write.clear();
		}
		if (indexes[p.second] != BL_SIZE) {
			p.first = rd[p.second][indexes[p.second]++];
			heap.insert(p);
		}
		else {
			rd[p.second].clear();
			read(blocks[p.second], rd[p.second], BL_SIZE);
			if (!blocks[p.second].eof()) {
				heap.insert({ rd[p.second][0], p.second });
			}
			else {
				blocks[p.second].close();
			}
		}
	}
	ifstream ans = ifstream{ to_string(l) + '$' + to_string(r) + ".bin", ifstream::binary };
	return ans;
}

void move_data(ifstream& from, ofstream& to) {
	int i = 0;
	from.seekg(0);
	while (i < n) {
		vector<int> cur_block;
		if (i + BL_SIZE > n) {
			read(from, cur_block, n - i);
			write(to, cur_block, n - i);
		}
		else {
			read(from, cur_block, BL_SIZE);
			write(to, cur_block, BL_SIZE);
		}
		i += BL_SIZE;
	}
	from.close();
	delete_file();
	return;
}

ifstream merge_sort(int l, int r) {
	if (r == l) {
		ifstream res(to_string(l) + ".bin", ifstream::binary);
		filenames.push(to_string(l) + ".bin");
		return res;
	}
	vector<ifstream> blocks;
	int delta = (r - l + 399) / 400;
	for (int i = l; i <= r; i += delta) {
		blocks.push_back(merge_sort(i, min(r, i + delta - 1)));
	}
	ifstream res = sort_k_blocks(blocks, l, r);
	for (int i = l; i <= r; i += delta) {
		delete_file();
	}
	filenames.push(to_string(l) + '$' + to_string(r) + ".bin");

	return res;
}

void output_bin(ifstream& file, int n) {
	for (int i = 0; i < n / BL_SIZE; i++) {
		vector<int> vec;
		read(file, vec, BL_SIZE);
		for (auto el : vec) {
			cout << el << ' ';
		}
	}
	if (n % BL_SIZE) {
		vector<int> vec;
		read(file, vec, n % BL_SIZE);
		for (auto el : vec) {
			cout << el << ' ';
		}
	}
	cout << '\n';
}

bool check_correct(ifstream& file) {
	int last = -INT_MAX;
	for (int i = 0; i < n / BL_SIZE; i++) {
		vector<int> vec;
		read(file, vec, BL_SIZE);
		for (auto el : vec) {
			if (el < last) {
				return false;
			} else {
				last = el;
			}
		}
	}
	if (n % BL_SIZE) {
		vector<int> vec;
		read(file, vec, n % BL_SIZE);
		for (auto el : vec) {
			if (el < last) {
				return false;
			} else {
				last = el;
			}
		}
	}
	return true;
}

int main() {
	cin >> n;
	generate_input_file(n);
	int num = split_into_sorted_blocks(n) - 1;

	ifstream res = merge_sort(0, num);
	
	ofstream output("output.bin", ofstream::binary);
	move_data(res, output);
	output.close();
	{
		ifstream file("output.bin", ifstream::binary);
		cout << check_correct(file);
		file.close();
	}
}