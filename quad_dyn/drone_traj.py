import numpy as np
from imgui_bundle import implot3d, implot, imgui
from scipy.spatial.transform import Rotation


class ValPlot:
    def __init__(self, ts: np.ndarray):
        self._label2data = {}
        self._ts = np.ascontiguousarray(ts, dtype=np.float64)

    def add_data(self, label: str, data: np.ndarray):
        self._label2data[label] = np.ascontiguousarray(data, dtype=np.float64)
        return self

    def plot(self):
        for label, data in self._label2data.items():
            implot.plot_line(label, self._ts, data)


class DroneTraj:
    def __init__(self):
        self.p = np.zeros((0, 3))
        self.q = np.zeros((0, 4))
        self.t = np.zeros((0))
        self.val_plots: dict[str, ValPlot] = {}

    def set_data(self, p: np.ndarray, q: np.ndarray, t: np.ndarray):
        self.p = np.ascontiguousarray(p, dtype=np.float64)
        self.q = np.ascontiguousarray(q, dtype=np.float64)
        self.t = np.ascontiguousarray(t, dtype=np.float64)

        self.xs = np.ascontiguousarray(self.p[:, 0])
        self.ys = np.ascontiguousarray(self.p[:, 1])
        self.zs = np.ascontiguousarray(self.p[:, 2])

    def draw(self):
        if self.p.shape[0] == 0:
            return

        t_cycle = 0.0
        idx = 0
        if self.t.shape[0] > 0:
            duration = self.t[-1] - self.t[0]
            if duration > 1e-6:
                t_now = imgui.get_time()
                t_cycle = (t_now % duration) + self.t[0]
                idx = np.searchsorted(self.t, t_cycle)
                if idx >= self.p.shape[0]:
                    idx = self.p.shape[0] - 1

        if implot3d.begin_plot("Drone Trajectory", size=imgui.ImVec2(-1, imgui.get_content_region_avail().y * 0.6)):
            pos = self.p[idx]
            implot3d.setup_axes_limits(pos[0] - 15, pos[0] + 15, pos[1] - 15,
                                       pos[1] + 15, pos[2] - 15, pos[2] + 15, implot3d.Cond_.always)

            implot3d.plot_line("Path", self.xs, self.ys, self.zs)

            quat = self.q[idx]
            rot = Rotation.from_quat(quat, scalar_first=True).as_matrix()

            p_origin = implot3d.plot_to_pixels(pos[0], pos[1], pos[2])
            p_x = implot3d.plot_to_pixels(pos[0] + 1.0, pos[1], pos[2])
            p_y = implot3d.plot_to_pixels(pos[0], pos[1] + 1.0, pos[2])
            p_z = implot3d.plot_to_pixels(pos[0], pos[1], pos[2] + 1.0)

            def dist_sq(p1, p2):
                return (p1.x - p2.x)**2 + (p1.y - p2.y)**2

            s_sq = max(dist_sq(p_origin, p_x), dist_sq(
                p_origin, p_y), dist_sq(p_origin, p_z))

            if s_sq > 1e-6:
                axis_len = 50.0 / np.sqrt(s_sq)
            else:
                axis_len = 0.5

            x_start = pos - rot[:, 0] * axis_len
            x_end = pos + rot[:, 0] * axis_len
            implot3d.set_next_line_style(col=[1, 0, 0, 1])
            implot3d.plot_line("X", np.array([x_start[0], x_end[0]]), np.array(
                [x_start[1], x_end[1]]), np.array([x_start[2], x_end[2]]))
            implot3d.plot_scatter("X",
                                  np.array([x_start[0], x_end[0]]), np.array([x_start[1], x_end[1]]), np.array([x_start[2], x_end[2]]))

            y_start = pos - rot[:, 1] * axis_len
            y_end = pos + rot[:, 1] * axis_len
            implot3d.set_next_line_style(col=[0, 1, 0, 1])
            implot3d.plot_line("Y", np.array([y_start[0], y_end[0]]), np.array(
                [y_start[1], y_end[1]]), np.array([y_start[2], y_end[2]]))
            implot3d.plot_scatter("Y",
                                  np.array([y_start[0], y_end[0]]), np.array([y_start[1], y_end[1]]), np.array([y_start[2], y_end[2]]))

            z_start = pos - rot[:, 2] * 0.3
            z_end = pos + rot[:, 2] * 0.3
            implot3d.set_next_line_style(col=[0, 0, 1, 1])
            implot3d.plot_line("Z", np.array([z_start[0], z_end[0]]), np.array(
                [z_start[1], z_end[1]]), np.array([z_start[2], z_end[2]]))

            implot3d.end_plot()

        plot_height = imgui.get_content_region_avail().y / 3

        for label, val_plot in self.val_plots.items():
            if implot.begin_plot(label, size=imgui.ImVec2(-1, plot_height)):
                val_plot.plot()
                implot.drag_line_x(-1, t_cycle,
                                   [1, 1, 1, 1], 1, implot.DragToolFlags_.no_inputs)
                implot.end_plot()

    def add_value_plot(self, label: str) -> ValPlot:
        if label in self.val_plots:
            return self.val_plots[label]
        val_plot = ValPlot(self.t)
        self.val_plots[label] = val_plot
        return val_plot


def main():
    from imgui_bundle import imgui, hello_imgui

    traj = DroneTraj()

    N = 1000
    t = np.linspace(0, 20, N)
    p = np.zeros((N, 3))
    p[:, 0] = np.cos(t) * t * 0.1
    p[:, 1] = np.sin(t) * t * 0.1
    p[:, 2] = t * 0.5

    q = np.zeros((N, 4))
    q[:, 0] = 1.0

    traj.set_data(p, q, t)
    traj.add_value_plot("X Position").add_data("X", p[:, 0])
    traj.add_value_plot("Y Position").add_data("Y", p[:, 1])
    traj.add_value_plot("Z Position").add_data("Z", p[:, 2])

    def gui():
        imgui.text("Drone Trajectory Example")
        traj.draw()

    runner_params = hello_imgui.RunnerParams()
    runner_params.callbacks.show_gui = gui
    runner_params.app_window_params.window_title = "Drone Trajectory"
    runner_params.app_window_params.window_geometry.size = (1280, 720)

    def post_init():
        implot3d.create_context()
        implot.create_context()

    def before_exit():
        implot.destroy_context()
        implot3d.destroy_context()

    runner_params.callbacks.post_init = post_init
    runner_params.callbacks.before_exit = before_exit

    hello_imgui.run(runner_params)


if __name__ == "__main__":
    main()
