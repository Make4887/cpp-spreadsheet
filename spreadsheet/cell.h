#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <set>
#include <functional>
#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override{
        std::set<Position> res(down_nodes_.begin(), down_nodes_.end());
        return {res.begin(), res.end()};
    }

    bool IsEmpty() const;
    void AddUpNode(const Position& pos);

private:
    Sheet& sheet_;

    class Impl {
    public:
        virtual ~Impl() = default;
        virtual Value GetValue(const Sheet&) const = 0;
        virtual std::string GetText() const = 0;
    };
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    struct PositionHasher {
	size_t operator()(const Position& pos) const {
		static const size_t N = 37;
		return pos_hasher_(pos.row) + N * pos_hasher_(pos.col);
	}
    private:
        std::hash<int> pos_hasher_;
    };

    std::unique_ptr<Impl> impl_;
    
    mutable std::optional<Value> cache_;
    std::unordered_set<Position, PositionHasher> up_nodes_;
    std::unordered_set<Position, PositionHasher> down_nodes_;

    void CheckCycle(const Cell* root_cell) const;
    void ClearCache() const;
};
