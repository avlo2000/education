import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider


class PID:
    def __init__(self, P=0.0, I=0.0, D=0.0):
        self.Kp = P
        self.Ki = I
        self.Kd = D
        self.setpoint = 1.0
        self.clear()

    def clear(self):
        self.p_term = 0.0
        self.i_term = 0.0
        self.d_term = 0.0
        self.last_error = 0.0
        self.int_error = 0.0
        self.windup_guard = 20.0
        self.output = 0.0

    def update(self, feedback_value):
        error = self.setpoint - feedback_value
        delta_error = error - self.last_error

        self.p_term = self.Kp * error
        self.i_term += error

        if self.i_term < -self.windup_guard:
            self.i_term = -self.windup_guard
        elif self.i_term > self.windup_guard:
            self.i_term = self.windup_guard

        self.d_term = delta_error
        self.last_error = error
        self.output = self.p_term + (self.Ki * self.i_term) + (self.Kd * self.d_term)


time = np.linspace(0, 5, 100)


def update(val):
    pid.Kp = s_p.val
    pid.Ki = s_i.val
    pid.Kd = s_d.val
    pid.clear()
    feedback = 0
    feedback_list = []
    for t in time:
        feedback_list.append(feedback)
        pid.update(feedback)
        feedback += pid.output
    l.set_ydata(feedback_list)
    ax.set_ylim([min(feedback_list) - 0.1, max(feedback_list) + 1])
    fig.canvas.draw_idle()


pid = PID(1.0, 0.1, 0.01)

fig, ax = plt.subplots()
plt.subplots_adjust(left=0.1, bottom=0.35)
feedback = 0
feedback_list = []
for t in time:
    pid.update(feedback)
    feedback += pid.output
    feedback_list.append(feedback)
l, = plt.plot(time, feedback_list, lw=2)

ax_p = plt.axes([0.1, 0.25, 0.65, 0.03], facecolor='lightgoldenrodyellow')
ax_i = plt.axes([0.1, 0.2, 0.65, 0.03], facecolor='lightgoldenrodyellow')
ax_d = plt.axes([0.1, 0.15, 0.65, 0.03], facecolor='lightgoldenrodyellow')

s_p = Slider(ax_p, 'P', 0.0, 10.0, valinit=1.0)
s_i = Slider(ax_i, 'I', 0.0, 1.0, valinit=0.1)
s_d = Slider(ax_d, 'D', 0.0, 1.0, valinit=0.01)

s_p.on_changed(update)
s_i.on_changed(update)
s_d.on_changed(update)

plt.show()
