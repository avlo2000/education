import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

def cross_product_animation():
    """
    Animate the cross product of vector (0, 0, 1) and (x, y, 0),
    where (x, y) moves along a unit circle.
    """
    
    # Create figure and 3D axis
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    
    # Fixed vector a = (0, 0, 1)
    vec_a = np.array([0, 0, 1])
    
    # Animation parameters
    frames = 200
    angles = np.linspace(0, 2*np.pi, frames)
    
    # Set up the plot limits and labels
    ax.set_xlim([-1.5, 1.5])
    ax.set_ylim([-1.5, 1.5])
    ax.set_zlim([-1.5, 1.5])
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    ax.set_title('Cross Product Animation: (0,0,1) × (x,y,0)')
    
    # Initialize empty line objects for vectors
    line_a, = ax.plot([], [], [], 'b-', linewidth=3, label='Vector a = (0,0,1)')
    line_b, = ax.plot([], [], [], 'r-', linewidth=3, label='Vector b = (x,y,0)')
    line_cross, = ax.plot([], [], [], 'g-', linewidth=3, label='Cross product a × b')
    
    # Add origin point
    ax.scatter([0], [0], [0], color='black', s=50, label='Origin')
    
    # Add unit circle in xy-plane for reference
    circle_theta = np.linspace(0, 2*np.pi, 100)
    circle_x = np.cos(circle_theta)
    circle_y = np.sin(circle_theta)
    circle_z = np.zeros_like(circle_x)
    ax.plot(circle_x, circle_y, circle_z, 'k--', alpha=0.3, label='Unit circle')
    
    # Add text for displaying current values
    text = ax.text2D(0.02, 0.95, '', transform=ax.transAxes, fontsize=10,
                     verticalalignment='top', bbox=dict(boxstyle='round', facecolor='wheat'))
    
    def animate(frame):
        # Calculate current position on unit circle
        angle = angles[frame]
        x = np.cos(angle)
        y = np.sin(angle)
        
        # Vector b = (x, y, 0)
        vec_b = np.array([x, y, 0])
        
        # Calculate cross product: a × b
        # (0,0,1) × (x,y,0) = (-y, x, 0)
        cross_product = np.cross(vec_a, vec_b)
        
        # Update vector a (fixed)
        line_a.set_data([0, vec_a[0]], [0, vec_a[1]])
        line_a.set_3d_properties([0, vec_a[2]])
        
        # Update vector b (moving on unit circle)
        line_b.set_data([0, vec_b[0]], [0, vec_b[1]])
        line_b.set_3d_properties([0, vec_b[2]])
        
        # Update cross product vector
        line_cross.set_data([0, cross_product[0]], [0, cross_product[1]])
        line_cross.set_3d_properties([0, cross_product[2]])
        
        # Update text with current values
        text.set_text(f'Angle: {angle:.2f} rad ({np.degrees(angle):.1f}°)\n'
                     f'Vector b: ({x:.2f}, {y:.2f}, 0)\n'
                     f'Cross product: ({cross_product[0]:.2f}, {cross_product[1]:.2f}, {cross_product[2]:.2f})\n'
                     f'Magnitude: {np.linalg.norm(cross_product):.2f}')
        
        return line_a, line_b, line_cross, text
    
    # Create animation
    anim = FuncAnimation(fig, animate, frames=frames, interval=50, blit=False, repeat=True)
    
    # Add legend
    ax.legend(loc='upper right')
    
    # Add grid
    ax.grid(True, alpha=0.3)
    
    return fig, anim

def analyze_cross_product():
    """
    Analyze the mathematical properties of the cross product in this case.
    """
    print("Cross Product Analysis:")
    print("=" * 50)
    print("Vector a = (0, 0, 1)")
    print("Vector b = (cos(θ), sin(θ), 0)")
    print()
    print("Cross product a × b = (0, 0, 1) × (cos(θ), sin(θ), 0)")
    print("                    = (-sin(θ), cos(θ), 0)")
    print()
    print("Properties:")
    print("1. The cross product is always in the xy-plane (z-component = 0)")
    print("2. The magnitude is always 1 (unit vector)")
    print("3. The cross product rotates 90° counterclockwise from vector b")
    print("4. The cross product traces out a unit circle in the xy-plane")
    print("5. When b points in +x direction, cross product points in +y direction")
    print("6. When b points in +y direction, cross product points in -x direction")

if __name__ == "__main__":
    # Print analysis
    analyze_cross_product()
    
    # Create and show animation
    fig, anim = cross_product_animation()
    
    # Optional: Save animation as gif (uncomment to save)
    # anim.save('cross_product_animation.gif', writer='pillow', fps=20)
    
    plt.tight_layout()
    plt.show()