#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    MakeEmptyCell(pos);
    if(data_[pos.row][pos.col]->GetText() == text && !text.empty()){
        return;
    }
    data_[pos.row][pos.col]->Set(std::move(text));
    for(auto referenced_cell: data_[pos.row][pos.col]->GetReferencedCells()){
        dynamic_cast<Cell*>(GetCell(referenced_cell))->AddUpNode(pos);
    }
    if (size_.cols <= pos.col) {
        size_.cols = pos.col + 1;
    }
    if (size_.rows <= pos.row) {
        size_.rows = pos.row + 1;
    }
}

void Sheet::MakeEmptyCell(Position pos){
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    int row_size = 0;
    if (!data_.empty()) {
        row_size = static_cast<int>(data_[0].size());
    }
    for (int i = data_.size(); i <= pos.row; ++i) {
        data_.push_back(std::vector<std::unique_ptr<Cell>>(row_size));
    }
    if (row_size <= pos.col) {
        for (int i = 0; i <= pos.row; ++i) {
            for (int j = row_size; j <= pos.col; ++j) {
                data_[i].emplace_back(nullptr);
            }
        }
    }
    if (!data_[pos.row][pos.col]) {
        data_[pos.row][pos.col] = std::make_unique<Cell>(*this);

    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= static_cast<int>(data_.size()) || pos.col >= static_cast<int>(data_[0].size()) || !data_[pos.row][pos.col]) {
        return nullptr;
    }
    return data_[pos.row][pos.col].get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= static_cast<int>(data_.size()) || pos.col >= static_cast<int>(data_[0].size()) || !data_[pos.row][pos.col]) {
        return nullptr;
    }
    return data_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.col >= size_.cols || pos.row >= size_.rows) {
        return;
    }
    data_[pos.row][pos.col] = nullptr;
    bool empty = true;
    for (int i = size_.rows - 1; i >= 0; --i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (data_[i][j]) {
                size_.rows = i + 1;
                i = -1;
                empty = false;
                break;
            }
        }
    }

    if (empty) {
        size_.rows = 0;
        size_.cols = 0;
    }

    for (int j = size_.cols - 1; j >= 0; --j) {
        for (int i = 0; i < size_.rows; ++i) {
            if (data_[i][j]) {
                size_.cols = j + 1;
                j = -1;
                break;
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (data_[i][j]) {
                auto value = data_[i][j]->GetValue();
                if (std::holds_alternative<double>(value)) {
                    output << std::get<double>(value);
                }
                else if (std::holds_alternative<std::string>(value)) {
                    output << std::get<std::string>(value);
                }
                else if (std::holds_alternative<FormulaError>(value)) {
                    output << std::get<FormulaError>(value);
                }
            }
            if (j == size_.cols - 1) {
                output << '\n';
            }
            else {
                output << '\t';
            }
        }
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_.rows; ++i) {
        for (int j = 0; j < size_.cols; ++j) {
            if (data_[i][j]) {
                output << data_[i][j]->GetText();
            }
            if (j == size_.cols - 1) {
                output << '\n';
            }
            else {
                output << '\t';
            }
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
