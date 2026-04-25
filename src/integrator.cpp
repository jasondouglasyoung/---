#include "integrator.h"
#include <cmath>
#include <algorithm>

double trapezoidal(const std::function<double(double)>& f, double a, double b, int n, int& evals) {
    double h = (b - a) / n;
    double sum = 0.5 * (f(a) + f(b));
    evals += 2;

    for (int i = 1; i < n; ++i) {
        sum += f(a + i * h);
        evals += 1;
    }

    return sum * h;
}

double simpson(const std::function<double(double)>& f, double a, double b, int n, int& evals) {
    if (n % 2 != 0) n++;

    double h = (b - a) / n;
    double sum = f(a) + f(b);
    evals += 2;

    for (int i = 1; i < n; ++i) {
        double x = a + i * h;
        sum += (i % 2 == 0) ? 2.0 * f(x) : 4.0 * f(x);
        evals += 1;
    }

    return sum * h / 3.0;
}

static double adaptive_simpson_recursive(
    const std::function<double(double)>& f,
    double a, double b,
    double fa, double fb, double fm,
    double S,
    double tol,
    int& evals)
{
    double m = (a + b) / 2.0;
    double lm = (a + m) / 2.0;
    double rm = (m + b) / 2.0;

    double fl = f(lm);
    double fr = f(rm);
    evals += 2;

    double S_left  = (m - a) / 6.0 * (fa + 4.0 * fl + fm);
    double S_right = (b - m) / 6.0 * (fm + 4.0 * fr + fb);

    if (std::abs(S_left + S_right - S) < 15.0 * tol) {
        return S_left + S_right + (S_left + S_right - S) / 15.0;
    }

    return adaptive_simpson_recursive(f, a, m, fa, fm, fl, S_left,  tol / 2.0, evals)
         + adaptive_simpson_recursive(f, m, b, fm, fb, fr, S_right, tol / 2.0, evals);
}

static double adaptive_simpson(const std::function<double(double)>& f, double a, double b, double tol, int& evals) {
    double m = (a + b) / 2.0;
    double fa = f(a), fb = f(b), fm = f(m);
    evals += 3;

    double S = (b - a) / 6.0 * (fa + 4.0 * fm + fb);
    return adaptive_simpson_recursive(f, a, b, fa, fb, fm, S, tol, evals);
}

IntegrationResult integrate(
    const std::function<double(double)>& f,
    double a, double b,
    IntegrationMethod method,
    int n,
    double tol)
{
    IntegrationResult result;
    int evals = 0;

    if (a == b) {
        result.result = 0.0;
        result.evaluations = 0;
        result.method_name = "zero-interval";
        return result;
    }

    if (a > b) {
        std::swap(a, b);
        auto r = integrate(f, a, b, method, n, tol);
        result.result = -r.result;
        result.evaluations = r.evaluations;
        result.method_name = r.method_name;
        return result;
    }

    switch (method) {
        case IntegrationMethod::Trapezoidal:
            result.result = trapezoidal(f, a, b, n, evals);
            result.method_name = "Trapezoidal";
            break;
        case IntegrationMethod::Simpson:
            result.result = simpson(f, a, b, n, evals);
            result.method_name = "Simpson";
            break;
        case IntegrationMethod::Adaptive:
            result.result = adaptive_simpson(f, a, b, tol, evals);
            result.method_name = "Adaptive Simpson";
            break;
    }

    result.evaluations = evals;
    return result;
}
