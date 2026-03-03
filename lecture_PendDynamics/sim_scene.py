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

    def reset(self) -> None:
        self.state = self.INITIAL_STATE.copy()

    def render(self, u: float, paused: bool = False) -> None:
        screen = self.screen
        theta, _, x_pos, _ = self.state
        r = 1.0
        scale = self._scale
        offset_x, offset_y = self._offset_x, self._offset_y
        width = self._width
        force_mag = 20.0

        screen.fill((20, 20, 30))

        cart_pixel_x = offset_x + int(x_pos * scale)
        cart_pixel_y = offset_y

        bob_pixel_x = cart_pixel_x + int(r * scale * np.sin(theta))
        bob_pixel_y = cart_pixel_y - int(r * scale * np.cos(theta))

        pygame.draw.line(screen, (100, 100, 100), (0, offset_y + 20), (width, offset_y + 20), 2)

        cart_w, cart_h = 80, 40
        cart_rect = pygame.Rect(cart_pixel_x - cart_w // 2, cart_pixel_y - cart_h // 2, cart_w, cart_h)
        pygame.draw.rect(screen, (70, 130, 200), cart_rect)
        pygame.draw.rect(screen, (200, 200, 255), cart_rect, 2)

        pygame.draw.line(screen, (200, 200, 200), (cart_pixel_x, cart_pixel_y), (bob_pixel_x, bob_pixel_y), 4)

        pygame.draw.circle(screen, (150, 150, 150), (cart_pixel_x, cart_pixel_y), 6)
        pygame.draw.circle(screen, (220, 80, 80), (bob_pixel_x, bob_pixel_y), 15)

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
