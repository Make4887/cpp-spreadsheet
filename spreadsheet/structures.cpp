#include "common.h"

#include <cctype>
#include <cmath>
#include <sstream>
#include <utility>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };

bool Size::operator==(Size rhs) const {
	return rows == rhs.rows && cols == rhs.cols;
}

bool Position::operator==(const Position rhs) const {
	return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
	return std::pair(row, col) < std::pair(rhs.row, rhs.col);
}

bool Position::IsValid() const {
	return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

std::string Position::ToString() const {
	if (!IsValid()) {
		return "";
	}
	std::string result;
	int c = col;
	int i = 0;
	do {
		i = 0;
		int copy_c = c;
		while (copy_c > LETTERS || (copy_c == LETTERS && i == 0)) {
			copy_c /= LETTERS;
			i++;
		}
		if (i == 0) {
			result.push_back('A' + copy_c);
		}
		else {
			result.push_back('A' + copy_c - 1);
		}
		c -= std::pow(LETTERS, i) * copy_c;
	} while (c > 0 || i > 0);
	return result + std::to_string(row + 1);
}

Position Position::FromString(std::string_view str) {
	auto it = str.begin();
	if(it == str.end() || !isupper(*it)){
		return Position::NONE;
	}
	int c = int(*it - 'A' + 1);
	++it;
	while (it != str.end() && isupper(*it)) {
		c = c * 26 + int(*it - 'A' + 1);
		++it;
	}
	--c;
	if (it == str.end() || !isdigit(*it)) {
		return Position::NONE;
	}
	int r = int(*it - '0');
	++it;
	while (it != str.end() && isdigit(*it)) {
		r = r * 10 + int(*it - '0');
		++it;
	}
	--r;
	if (it != str.end() || r >= MAX_ROWS || c >= MAX_COLS) {
		return Position::NONE;
	}
	return { r, c };
}
