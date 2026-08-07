import numpy


def A_state_nl(theta, thetadot, xdot, u, m, M, r, g, b, c):
    """Nonlinear state-space A matrix (Jacobian of f w.r.t. state)."""
    return numpy.array([
        [0, 1, 0, 0],
        [0, 
            -b/(M + m*numpy.sin(theta)**2), 
            (-m*(-2*b*r*xdot + 2*c*thetadot*numpy.cos(theta) - g*m*r*numpy.sin(2*theta) + 2*m*r**2*thetadot**2*numpy.sin(theta) + 2*r*u)*numpy.sin(theta)*numpy.cos(theta) - (M + m*numpy.sin(theta)**2)*(c*thetadot*numpy.sin(theta) - 2*g*m*r*numpy.sin(theta)**2 + g*m*r - m*r**2*thetadot**2*numpy.cos(theta)))/(r*(M + m*numpy.sin(theta)**2)**2), 
            (c*numpy.cos(theta) + 2*m*r**2*thetadot*numpy.sin(theta))/(r*(M + m*numpy.sin(theta)**2))
        ],
        [0, 0, 0, 1],
        [0, 
         b*numpy.cos(theta)/(r*(M + m*numpy.sin(theta)**2)), 
         (r*(M + m*numpy.sin(theta)**2)*(M*g*numpy.cos(theta) - b*xdot*numpy.sin(theta) + g*m*numpy.cos(theta) - 2*m*r*thetadot**2*numpy.cos(theta)**2 + m*r*thetadot**2 + u*numpy.sin(theta)) + (2*M*c*thetadot - 2*M*g*m*r*numpy.sin(theta) - 2*b*m*r*xdot*numpy.cos(theta) + 2*c*m*thetadot - 2*g*m**2*r*numpy.sin(theta) + m**2*r**2*thetadot**2*numpy.sin(2*theta) + 2*m*r*u*numpy.cos(theta))*numpy.sin(theta)*numpy.cos(theta))/(r**2*(M + m*numpy.sin(theta)**2)**2), 
         -(M*c + c*m + m**2*r**2*thetadot*numpy.sin(2*theta))/(m*r**2*(M + m*numpy.sin(theta)**2))
        ],
    ])


def B_state_nl(theta, thetadot, xdot, u, m, M, r, g, b, c):
    """Nonlinear state-space B matrix (Jacobian of f w.r.t. input)."""
    return numpy.array([
        [0],
        [(M + m*numpy.sin(theta)**2)**(-1.0)],
        [0],
        [-numpy.cos(theta)/(r*(M + m*numpy.sin(theta)**2))],
    ])
