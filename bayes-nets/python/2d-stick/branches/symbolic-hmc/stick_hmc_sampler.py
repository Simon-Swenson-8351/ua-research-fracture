import numpy as np

# Sample from the stick distribution, given an observation, using the Hamiltonian (particle physics) method.
class StickHMCSampler:

    def __init__(self, posterior):
        self.covar = None
        self.posterior = posterior

        self.posterior_derivatives = None

    def run_all(self, rng):
        pass

    def step(self, rng):
        pass

    def __hamiltonian_update(self, initial_state, epsilon, L):
        he = epsilon / 2
        p_cur = initial_state['momentum']
        p_halfcur = None
        q_cur = initial_state['position']
        for t in np.arange(0.0, L, epsilon):
            # leapfrog t times.
            p_halfcur = p_cur
            for i in range(len(p_cur)):
                cur_post_der = self.posterior_derivatives[i]

                for j in range(len(p_cur)):

                p_halfcur[i] -= he * self.posterior_derivatives[i](q_cur)
                q_cur[i] += epsilon * p_halfcur[i] / self.covar[i, i]
            p_cur = p_halfcur
            for i in range(len(p_cur)):
                p_cur[i] -= he * self.posterior_derivatives[i](q_cur[i])
        return {'momentum': p_cur, 'position': q_cur}

    def __momentum_resample(self):
        result = [None] *