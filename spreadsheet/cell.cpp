#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

class Cell::EmptyImpl : public Impl {
public:
    Value GetValue(const Sheet&) const override {
        return 0.;
    }
    std::string GetText() const override {
        return "";
    }
};

class Cell::TextImpl : public Impl {
public:
    TextImpl(std::string&& text)
        : text_(move(text))
    {
    }
    Value GetValue(const Sheet&) const override {
        return !text_.empty() && text_[0] == ESCAPE_SIGN ? text_.substr(1) : text_;
    }
    std::string GetText() const override {
        return text_;
    }
private:
    std::string text_;
};

class Cell::FormulaImpl : public Cell::Impl {
public:
    FormulaImpl(std::string&& text)
        : formula_(ParseFormula(std::move(text)))
    {
    }
    Value GetValue(const Sheet& sheet) const override {
    auto res = formula_->Evaluate(sheet);
    if (std::holds_alternative<double>(res)) {
        return std::get<double>(res);
    }
    return std::get<FormulaError>(res);
}
    std::string GetText() const override {
        return '=' + formula_->GetExpression();
    }
    std::vector<Position> GetReferencedCells() const{
        return formula_->GetReferencedCells();
    }
private:
    std::unique_ptr<FormulaInterface> formula_;
};

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    : sheet_(sheet)
    , impl_(std::make_unique<EmptyImpl>())
{
}

void Cell::Set(std::string text) {
    if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        auto it = text.begin() + 1;
        while (it != text.end() && isupper(*it)) {
            ++it;
        }
        while (it != text.end() && it != text.begin() + 1 && isdigit(*it)) {
            ++it;
        }
        if(it != text.begin() + 1 && it != text.end() && isalpha(*it)){
            throw FormulaException("");
        }
        auto new_formula_impl = std::make_unique<FormulaImpl>(text.substr(1));
        auto new_referenced_cells = new_formula_impl->GetReferencedCells();
        for(auto cell: new_referenced_cells){
            sheet_.MakeEmptyCell(cell);
            dynamic_cast<const Cell*>(sheet_.GetCell(cell))->CheckCycle(this);
        }
        impl_ = std::move(new_formula_impl);
        down_nodes_.clear();
        down_nodes_.insert(new_referenced_cells.begin(), new_referenced_cells.end());
    }
    else {
        impl_ = std::make_unique<TextImpl>(std::move(text));
        down_nodes_.clear();
    }
    ClearCache();
}

bool Cell::IsEmpty() const{
    return dynamic_cast<EmptyImpl*>(impl_.get());
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
    ClearCache();
    down_nodes_.clear();
}

Cell::Value Cell::GetValue() const {
    if(!cache_.has_value()){
        cache_ = impl_->GetValue(sheet_);
    }
    return cache_.value();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

void Cell::ClearCache() const{
    cache_.reset();
    for(auto cell: up_nodes_){
        if(cache_.has_value()){
            dynamic_cast<const Cell*>(sheet_.GetCell(cell))->ClearCache();
        }
    }
}

void Cell::AddUpNode(const Position& pos){
    up_nodes_.insert(pos);
}

void Cell::CheckCycle(const Cell* root_cell) const{
    if(root_cell == this){
        throw CircularDependencyException("");
    }
    for(auto cell: down_nodes_){
        dynamic_cast<const Cell*>(sheet_.GetCell(cell))->CheckCycle(root_cell);
    }
}