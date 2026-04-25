#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include "parser.h"
#include "integrator.h"

struct Options {
    std::string expression;
    double lower = 0.0;
    double upper = 1.0;
    IntegrationMethod method = IntegrationMethod::Adaptive;
    int n = 1000;
    double tol = 1e-8;
    bool show_help = false;
    bool show_methods = false;
};

static bool str_eq(const char* s, std::string_view v) {
    return s && (v == s);
}

static IntegrationMethod parse_method(const char* s) {
    if (!s) return IntegrationMethod::Adaptive;
    if (strcmp(s, "trapezoidal") == 0) return IntegrationMethod::Trapezoidal;
    if (strcmp(s, "simpson") == 0)     return IntegrationMethod::Simpson;
    if (strcmp(s, "adaptive") == 0)    return IntegrationMethod::Adaptive;
    std::cerr << "Warning: unknown method '" << s << "', using adaptive\n";
    return IntegrationMethod::Adaptive;
}

static void print_usage(const char* prog) {
    std::cout << "定积分计算器 — Definite Integral Calculator\n"
              << "===========================================\n\n"
              << "Usage:\n"
              << "  " << prog << " <expression> <lower> <upper> [options]\n\n"
              << "Arguments:\n"
              << "  expression      Mathematical function of x (e.g. \"sin(x)\", \"x^2 + 3*x\")\n"
              << "  lower           Lower bound of integration\n"
              << "  upper           Upper bound of integration\n\n"
              << "Options:\n"
              << "  --method <m>    Integration method: trapezoidal | simpson | adaptive (default)\n"
              << "  --n <int>       Number of subdivisions for trapezoidal/simpson (default: 1000)\n"
              << "  --tol <float>   Error tolerance for adaptive method (default: 1e-8)\n"
              << "  --methods       List available functions and operators\n"
              << "  --help          Show this help message\n\n"
              << "Examples:\n"
              << "  " << prog << " \"sin(x)\" 0 3.1415926535\n"
              << "  " << prog << " --method simpson \"x^2 + 2*x + 1\" 0 1\n"
              << "  " << prog << " --tol 1e-12 \"exp(-x^2)\" -5 5\n"
              << "  " << prog << " --method trapezoidal \"sqrt(1 - x^2)\" -1 1 --n 10000\n";
}

static void print_methods() {
    std::cout << "Supported functions and operators (via exprtk):\n\n"
              << "Arithmetic:      +  -  *  /  %  ^\n"
              << "Comparisons:     <  >  <=  >=  ==  !=\n"
              << "Trigonometric:   sin(x)  cos(x)  tan(x)\n"
              << "Inverse trig:    asin(x)  acos(x)  atan(x)  atan2(y,x)\n"
              << "Hyperbolic:      sinh(x)  cosh(x)  tanh(x)\n"
              << "Exponential:     exp(x)  log(x)  log10(x)  log2(x)\n"
              << "Power/Root:      sqrt(x)  pow(x,y)  root(x,y)\n"
              << "Rounding:        ceil(x)  floor(x)  round(x)  trunc(x)\n"
              << "Misc:            abs(x)  sign(x)  min(x,y)  max(x,y)\n"
              << "Constants:        pi  e  epsilon  infinity\n\n"
              << "Built-in variables: x\n";
}

int main(int argc, char* argv[]) {
    Options opts;

    // Parse arguments — collect positional after flags
    int i = 1;
    while (i < argc) {
        if (str_eq(argv[i], "--help")) {
            opts.show_help = true;
            i++;
        } else if (str_eq(argv[i], "--methods")) {
            opts.show_methods = true;
            i++;
        } else if (str_eq(argv[i], "--method") && i + 1 < argc) {
            opts.method = parse_method(argv[++i]);
            i++;
        } else if (str_eq(argv[i], "--n") && i + 1 < argc) {
            opts.n = std::atoi(argv[++i]);
            if (opts.n < 2) opts.n = 1000;
            i++;
        } else if (str_eq(argv[i], "--tol") && i + 1 < argc) {
            opts.tol = std::atof(argv[++i]);
            if (opts.tol <= 0.0) opts.tol = 1e-8;
            i++;
        } else if (argv[i][0] == '-') {
            std::cerr << "Unknown option: " << argv[i] << "\n";
            print_usage(argv[0]);
            return 1;
        } else {
            break;
        }
    }

    if (opts.show_help) {
        print_usage(argv[0]);
        return 0;
    }

    if (opts.show_methods) {
        print_methods();
        return 0;
    }

    // Positional args: expression lower upper
    int remaining = argc - i;
    if (remaining < 3) {
        std::cerr << "Error: missing arguments.";
        if (remaining > 0) std::cerr << " Got expression but need lower and upper.";
        std::cerr << "\n\n";
        print_usage(argv[0]);
        return 1;
    }

    opts.expression = argv[i];
    opts.lower = std::atof(argv[i + 1]);
    opts.upper = std::atof(argv[i + 2]);

    if (opts.expression.empty()) {
        std::cerr << "Error: empty expression\n";
        return 1;
    }

    // Compile expression
    MathParser parser;
    if (!parser.compile(opts.expression)) {
        std::cerr << "Error: failed to parse expression \"" << opts.expression << "\"\n";
        return 1;
    }

    // Integrate
    auto f = [&](double x) { return parser.evaluate(x); };
    auto result = integrate(f, opts.lower, opts.upper, opts.method, opts.n, opts.tol);

    // Output
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "  Expression:  " << opts.expression << "\n";
    std::cout << "  Interval:    [" << opts.lower << ", " << opts.upper << "]\n";
    std::cout << "  Method:      " << result.method_name << "\n";
    std::cout << "  Result:      " << result.result << "\n";
    std::cout << "  Evaluations: " << result.evaluations << "\n";

    return 0;
}
