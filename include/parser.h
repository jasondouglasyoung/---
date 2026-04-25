#pragma once
#include <string>
#include <string_view>
#include "exprtk.hpp"

class MathParser {
public:
    MathParser() {
        symbol_table_.add_variable("x", x_);
        symbol_table_.add_constants();
        expression_.register_symbol_table(symbol_table_);
    }

    bool compile(std::string_view expr) {
        return parser_.compile(std::string(expr), expression_);
    }

    double evaluate(double x_val) {
        x_ = x_val;
        return expression_.value();
    }

private:
    double x_ = 0.0;
    exprtk::symbol_table<double> symbol_table_;
    exprtk::expression<double> expression_;
    exprtk::parser<double> parser_;
};
