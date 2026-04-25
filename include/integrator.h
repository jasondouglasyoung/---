#pragma once
#include <functional>
#include <string>

enum class IntegrationMethod {
    Trapezoidal,
    Simpson,
    Adaptive,
};

struct IntegrationResult {
    double result = 0.0;
    int evaluations = 0;
    std::string method_name;
};

IntegrationResult integrate(
    const std::function<double(double)>& f,
    double a, double b,
    IntegrationMethod method = IntegrationMethod::Adaptive,
    int n = 1000,
    double tol = 1e-8
);
