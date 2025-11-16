from typing import Callable, List
import numpy as np
import matplotlib.pyplot as plt


class GridOpt:
    def __init__(self, mn: np.ndarray, mx: np.ndarray, ns: np.ndarray, depth: int):
        self._mn = mn
        self._mx = mx
        self._ns = ns
        self._depth = depth

    def nms(self, peaks: List[np.ndarray]) -> List[np.ndarray]:
        cell_size = 0.5 * (self._mx - self._mn) / (self._ns - 1)
        tol = cell_size / 2**self._depth
        selected_peaks = []
        for peak in peaks:
            is_far = True
            for sel_peak in selected_peaks:
                if np.all(np.abs(peak - sel_peak) <= tol):
                    is_far = False
                    break
            if is_far:
                selected_peaks.append(peak)
        return selected_peaks

    def _find_peaks(self, arr: np.ndarray):
        # finds peas in N-dimensional array,
        # arr[i1, i2, ..., in] is a peak if and ony if
        # arr[i1, i2, ..., in] >= arr[ + 1, i2, ..., in] and arr[i1, i2, ..., in] >= arr[i1, i2 + 1, ..., in] and ...
        peaks = []
        it = np.nditer(arr, flags=['multi_index'], order='C')
        while not it.finished:
            idx = it.multi_index
            val = it[0]
            is_peak = True
            for dim in range(arr.ndim):
                for offset in [-1, 1]:
                    neighbor_idx = list(idx)
                    neighbor_idx[dim] += offset
                    if 0 <= neighbor_idx[dim] < arr.shape[dim]:
                        if arr[tuple(neighbor_idx)] > val:
                            is_peak = False
                            break
                if not is_peak:
                    break
            if is_peak:
                peaks.append(idx)
            it.iternext()
        return peaks

    def _solve(self, 
               fn: Callable[[np.ndarray], float], 
               mn: np.ndarray, 
               mx: np.ndarray, 
               d: int,
               ax: plt.Axes = None):
        spaces = [np.linspace(mn_v, mx_v, n) for mn_v, mx_v, n in zip(mn, mx, self._ns)]
        x = np.meshgrid(*spaces, indexing='ij')
        y = fn(*x)
        cell_size = 0.5 * (mx - mn) / (self._ns - 1)
        peaks = self._find_peaks(y)
        all_peak_coords = []
        
        for peak in peaks:
            peak_coords = np.array([spaces[dim][peak[dim]] for dim in range(len(peak))])
            new_mn = np.clip(peak_coords - cell_size, mn, mx)
            new_mx = np.clip(peak_coords + cell_size, mn, mx)
            if d != 0:
                all_peak_coords.extend(self._solve(fn, new_mn, new_mx, d - 1, ax))
            else:
                all_peak_coords.append(peak_coords)

        if ax is not None:
            ax.plot_surface(*x, y)
        return all_peak_coords

        
    def solve(self, fn: Callable[[np.ndarray], float]) -> List[np.ndarray]:
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        peaks = self._solve(fn, self._mn, self._mx, self._depth, ax)
        peaks = self.nms(peaks)
        plt.show()
        return peaks



def main():
    from sympy import symbols, diff, solve

    def saddle(x, y):
        return -(x**2 - y**2)

    def polynome(x, y):
        return -(x**4 + y**4 - 4*x**2 + 4)
    
    fn = saddle
    
    x, y = symbols('x y')
    df_dx = diff(fn(x, y), x)
    df_dy = diff(fn(x, y), y)
    critical_points = solve([df_dx, df_dy], (x, y))
    print("Critical points:", critical_points)

    opt = GridOpt(
        np.array([-2.0, -2.0]),
        np.array([2.0, 2.0]),
        np.array([10, 10]),
        depth=3
    )
    peaks = opt.solve(fn)
    print("Found peaks at:")
    for peak in peaks:
        print(peak)


if __name__ == '__main__':
    main()