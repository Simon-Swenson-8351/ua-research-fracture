import sympy as sp

# This file was created to provide a backbone for the HMC sampler, which requires derivatives and thus symbolic
# manipulation. scipy.stats has really convenient sampling routines, but it does not do things symbolically.
class SymbolicDistribution:

    def __init__(self, eqn):
        self.eqn = eqn

    def sample(self, rng):
        raise Exception('calling abstract method')

# Normal Distribution as parameterized by mean and standard deviation
def norm_pdf(expr_x, expr_mean, expr_std):
    return 1 / (expr_std * sp.sqrt(2 * sp.pi)) * sp.exp(-(expr_x - expr_mean) ** 2 / (2 * expr_std ** 2))


# Both high and low are included in the distribution, not that it matters much.
def uniform_pdf(expr_x, expr_low, expr_high):
    return sp.Piecewise(
        (0, expr_x < expr_low),
        (1 / (expr_high - expr_low), expr_x <= expr_high),
        (0, True)
    )


# When sym_low and sym_high are bound, ensure: 0 <= sym_low < sym_high
def uniform_pmf(expr_n, expr_low, expr_high):
    fn = sp.Piecewise(
        (0.0, expr_n < expr_low),
        (1 / (expr_high - expr_low + 1), expr_n <= expr_high),
        (0.0, True)
    )
    return sp.sequence(fn, (expr_n, 0, sp.oo))


# When sym_loc is bound, ensure: 0 < p < 1 AND 0 <= sym_loc
def geometric_pmf(expr_n, expr_p, expr_loc, lower_bound=0, upper_bound=sp.oo):
    fn = (1 - expr_p) ** (expr_n - 1 - expr_loc) * expr_p
    return sp.sequence(fn, (expr_n, lower_bound, upper_bound))


# Attempts to sample a symbolic distribution, given the support and maximum density value.
def sample_pdf(pdf, min_x, max_x, max_y):
    raise Exception('Not implemented')


def sample_pmf(pmf, low, high):
    raise Exception('Not implemented')
