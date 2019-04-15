import numpy as np


class Bagging(object):

    def __init__(self, get_model, sample_size=1.0, T=10):
        self.get_model = get_model
        self.sample_size = sample_size
        self.T = T

    def __fit_one__(self, X, y):
        model = self.get_model()
        indices = np.random.random_integers(0, len(X) - 1,
                                            int(self.sample_size * len(X)))
        model.fit(X[indices], y[indices])
        return model

    def fit(self, X, y):
        self.models = [self.__fit_one__(X, y) for _ in xrange(self.T)]

    def __predict__(self, x):
        results = [m.__predict__(x) for m in self.models]
        r_counts = [(r, np.sum(results == r)) for r in np.unique(results)]
        return max(r_counts, key=lambda e: e[1])[0]

    def predict(self, X):
        return np.apply_along_axis(lambda x: self.__predict__(x),
                                   axis=1, arr=X)

    def score(self, X, y):
        return sum([m.score(X, y) for m in self.models]) / self.T
