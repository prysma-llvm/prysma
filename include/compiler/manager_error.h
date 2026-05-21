//===-- manager_error.h -----------------------------------------*- C++ -*-===//
//
// Part of the Prysma Project, under the GNU GPL v3.0 or later.
// See LICENSE at the project root for license information.
// SPDX-License-Identifier: GPL-3.0-or-later WITH Prysma-exception-1.0
//
//===----------------------------------------------------------------------===//

#ifndef ERROR_MANAGER_H
#define ERROR_MANAGER_H

#include <string>
#include <stdexcept>
#include "compiler/macros/prysma_nodiscard.h"

struct Line {
private:
    int value;
public:
    explicit Line(int val) : value(val) {}
    PRYSMA_NODISCARD auto getValue() const -> int { return value; }
};

struct Column {
private:
    int value;
public:
    explicit Column(int val) : value(val) {}
    PRYSMA_NODISCARD auto getValue() const -> int { return value; }
};

class CompilationError : public std::runtime_error
{
private:
    Line line;
    Column column;

public:
    PRYSMA_NODISCARD auto getLine() const -> int { return line.getValue(); }
    PRYSMA_NODISCARD auto getColumn() const -> int { return column.getValue(); }

    CompilationError(const std::string& message, Line lin, Column col)
        : std::runtime_error(message), line(lin), column(col)
    {
    }

    CompilationError(const CompilationError&) = default;
    auto operator=(const CompilationError&) -> CompilationError& = default;
    CompilationError(CompilationError&&) = default;
    auto operator=(CompilationError&&) -> CompilationError& = default;

    ~CompilationError() override = default;
};

#endif /* ERROR_MANAGER_H */
