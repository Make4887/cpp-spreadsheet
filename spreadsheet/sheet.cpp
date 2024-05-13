#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    CheckPosition(pos);
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

const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosition(pos);
    if (pos.row >= static_cast<int>(data_.size()) ||
        pos.col >= static_cast<int>(data_[0].size()) ||
        !data_[pos.row][pos.col]) {
        return nullptr;
    }
    return data_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPosition(pos);
    if (pos.row >= static_cast<int>(data_.size()) ||
        pos.col >= static_cast<int>(data_[0].size()) ||
        !data_[pos.row][pos.col]) {
        return nullptr;
    }
    return data_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    CheckPosition(pos);
    if (pos.col >= size_.cols || pos.row >= size_.rows) {
        return;
    }
    data_[pos.row][pos.col] = nullptr;
    bool empty = true;
    for (int row = size_.rows - 1; row >= 0; --row) {
        for (int col = 0; col < size_.cols; ++col) {
            if (data_[row][col]) {
                size_.rows = row + 1;
                --row;
                empty = false;
                break;
            }
        }
    }

    if (empty) {
        size_.rows = 0;
        size_.cols = 0;
    }

    for (int col = size_.cols - 1; col >= 0; --col) {
        for (int row = 0; row < size_.rows; ++row) {
            if (data_[row][col]) {
                size_.cols = col + 1;
                --col;
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

void Sheet::CheckPosition(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position Error");
    }
}
