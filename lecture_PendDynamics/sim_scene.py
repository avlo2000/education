import numpy as np
import pygame


class SimScene:
    def __init__(self, screen: pygame.Surface):
        self.screen = screen

        self._width = screen.get_width()
        self._height = screen.get_height()
        self._scale = 100
        self._offset_x = self._width // 2
        self._offset_y = 500

    def set_state(self, state: np.ndarray) -> None:
        self.state = state

    def set_linearized_state(self, state_lin: np.ndarray) -> None:
        self.state_lin = state_lin

    def reset(self) -> None:
        self.state = self.INITIAL_STATE.copy()

    def _draw_pendulum(self, surface, theta, x_pos, cart_color, cart_border,
                        rod_color, pivot_color, bob_color, rod_w=4, bob_r=15):
        scale = self._scale
        offset_x, offset_y = self._offset_x, self._offset_y
        r = 1.0

        cx = offset_x + int(x_pos * scale)
        cy = offset_y
        bx = cx + int(r * scale * np.sin(theta))
        by = cy - int(r * scale * np.cos(theta))

        cart_w, cart_h = 80, 40
        cart_rect = pygame.Rect(cx - cart_w // 2, cy - cart_h // 2, cart_w, cart_h)
        pygame.draw.rect(surface, cart_color, cart_rect)
        pygame.draw.rect(surface, cart_border, cart_rect, 2)
        pygame.draw.line(surface, rod_color, (cx, cy), (bx, by), rod_w)
        pygame.draw.circle(surface, pivot_color, (cx, cy), 6)
        pygame.draw.circle(surface, bob_color, (bx, by), bob_r)

    def render(self, u: float, paused: bool = False) -> None:
        screen = self.screen
        theta, _, x_pos, _ = self.state
        scale = self._scale
        offset_x, offset_y = self._offset_x, self._offset_y
        width = self._width
        force_mag = 20.0

        screen.fill((20, 20, 30))
        pygame.draw.line(screen, (100, 100, 100), (0, offset_y + 20), (width, offset_y + 20), 2)

        if hasattr(self, 'state_lin') and self.state_lin is not None:
            ghost = pygame.Surface((self._width, self._height), pygame.SRCALPHA)
            t_lin, _, x_lin, _ = self.state_lin
            alpha = 90
            self._draw_pendulum(
                ghost, t_lin, x_lin,
                cart_color=(70, 130, 200, alpha),
                cart_border=(200, 200, 255, alpha),
                rod_color=(200, 200, 200, alpha),
                pivot_color=(150, 150, 150, alpha),
                bob_color=(80, 220, 120, alpha),
                rod_w=3, bob_r=12,
            )
            screen.blit(ghost, (0, 0))

        self._draw_pendulum(
            screen, theta, x_pos,
            cart_color=(70, 130, 200),
            cart_border=(200, 200, 255),
            rod_color=(200, 200, 200),
            pivot_color=(150, 150, 150),
            bob_color=(220, 80, 80),
        )

        panel = pygame.Surface((200, 100), pygame.SRCALPHA)
        panel.fill((0, 0, 0, 130))
        screen.blit(panel, (6, 6))

        bar_x, bar_y = 10, 12
        bar_w, bar_h = 180, 14

        pygame.draw.rect(screen, (60, 60, 60), (bar_x, bar_y, bar_w, bar_h))
        theta_fill = max(0, min(bar_w, int((theta / (2 * np.pi) + 0.5) * bar_w)))
        pygame.draw.rect(screen, (100, 180, 255), (bar_x, bar_y, theta_fill, bar_h))
        pygame.draw.line(screen, (255, 255, 255), (bar_x + bar_w // 2, bar_y), (bar_x + bar_w // 2, bar_y + bar_h), 1)

        bar_y2 = bar_y + bar_h + 6
        pygame.draw.rect(screen, (60, 60, 60), (bar_x, bar_y2, bar_w, bar_h))
        x_fill = max(0, min(bar_w, int((x_pos / 10.0 + 0.5) * bar_w)))
        pygame.draw.rect(screen, (80, 220, 130), (bar_x, bar_y2, x_fill, bar_h))
        pygame.draw.line(screen, (255, 255, 255), (bar_x + bar_w // 2, bar_y2), (bar_x + bar_w // 2, bar_y2 + bar_h), 1)

        bar_y3 = bar_y2 + bar_h + 6
        pygame.draw.rect(screen, (60, 60, 60), (bar_x, bar_y3, bar_w, bar_h))
        u_fill = max(0, min(bar_w, int((u / force_mag + 1.0) / 2.0 * bar_w)))
        u_color = (100, 255, 100) if abs(u) > 0 else (120, 120, 120)
        pygame.draw.rect(screen, u_color, (bar_x, bar_y3, u_fill, bar_h))
        pygame.draw.line(screen, (255, 255, 255), (bar_x + bar_w // 2, bar_y3), (bar_x + bar_w // 2, bar_y3 + bar_h), 1)

        if paused:
            pause_surf = pygame.Surface((80, 24), pygame.SRCALPHA)
            pause_surf.fill((200, 80, 80, 180))
            screen.blit(pause_surf, (width // 2 - 40, 10))
            pygame.draw.rect(screen, (255, 255, 255), (width // 2 - 10, 14, 6, 16))
            pygame.draw.rect(screen, (255, 255, 255), (width // 2 + 4, 14, 6, 16))
