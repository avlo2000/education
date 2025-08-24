import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
from scipy import signal

class ChirpModulationDemo:
    def __init__(self):
        self.B = 125e3
        self.SF = 7 

        self.regenerate_signal()

        self.current_snr_db = 10
        self.noise = np.zeros_like(self.modulated_chirp)
        self.noisy_signal = self.modulated_chirp.copy()

        self.setup_figure()
    
    def regenerate_signal(self):
        self.T = 1 / self.B
        self.T_s = (2**self.SF) * self.T
        self.N = 2**self.SF

        self.symbol = np.random.randint(0, self.N)

        self.t = np.linspace(0, self.T_s, self.N)
        print(f"B={self.B/1000:.1f}kHz, SF={self.SF}, Symbol: {self.symbol}")
        print(f"Samples per symbol: {len(self.t) // self.N}")
        print(f"Shift samples: {(self.symbol * (len(self.t) // self.N)) % len(self.t)}")

        f0 = 0
        f1 = self.B

        self.base_chirp = signal.chirp(self.t, f0, self.T_s, f1, method='linear')

        samples_per_symbol = len(self.t) // self.N
        shift_samples = (self.symbol * samples_per_symbol) % len(self.t)

        self.modulated_chirp = np.roll(self.base_chirp, shift_samples)

        self.signal_power = np.mean(self.modulated_chirp**2)
        
    def setup_figure(self):
        self.fig = plt.figure(figsize=(16, 12))
        gs = self.fig.add_gridspec(3, 3, height_ratios=[3, 3, 1], width_ratios=[1, 1, 1])
        
        self.ax1 = self.fig.add_subplot(gs[0, 0])
        self.ax2 = self.fig.add_subplot(gs[0, 1])
        self.ax3 = self.fig.add_subplot(gs[0, 2])

        self.ax_controls = self.fig.add_subplot(gs[2, :])
        self.ax_controls.set_xlim(0, 1)
        self.ax_controls.set_ylim(0, 1)
        self.ax_controls.axis('off')

        snr_slider_ax = plt.axes([0.1, 0.12, 0.35, 0.03])
        self.snr_slider = Slider(snr_slider_ax, 'SNR (dB)', -30, 30, valinit=10, 
                                valfmt='%0.1f dB')
        self.snr_slider.on_changed(self.update_noise)

        bw_slider_ax = plt.axes([0.55, 0.12, 0.35, 0.03])
        self.bw_slider = Slider(bw_slider_ax, 'Bandwidth (kHz)', 10, 500, 
                               valinit=self.B/1000, valfmt='%0.1f kHz')
        self.bw_slider.on_changed(self.update_bandwidth)

        sf_slider_ax = plt.axes([0.1, 0.08, 0.35, 0.03])
        self.sf_slider = Slider(sf_slider_ax, 'Spreading Factor', 5, 12, 
                               valinit=self.SF, valfmt='%d', valstep=1)
        self.sf_slider.on_changed(self.update_sf)

        noise_button_ax = plt.axes([0.55, 0.08, 0.12, 0.03])
        self.noise_button = Button(noise_button_ax, 'New Noise')
        self.noise_button.on_clicked(self.regenerate_noise)
        
        symbol_button_ax = plt.axes([0.68, 0.08, 0.12, 0.03])
        self.symbol_button = Button(symbol_button_ax, 'New Symbol')
        self.symbol_button.on_clicked(self.change_symbol)

        regen_button_ax = plt.axes([0.81, 0.08, 0.12, 0.03])
        self.regen_button = Button(regen_button_ax, 'Regenerate')
        self.regen_button.on_clicked(self.regenerate_all)

        self.update_plots()
        
    def update_bandwidth(self, val):
        self.B = self.bw_slider.val * 1000
        self.regenerate_signal()
        self.update_noise(self.current_snr_db)
        
    def update_sf(self, val):
        self.SF = int(self.sf_slider.val)
        self.regenerate_signal()
        self.update_noise(self.current_snr_db)
        
    def regenerate_all(self, event):
        self.regenerate_signal()
        self.regenerate_noise(event)
        
    def update_noise(self, val):
        self.current_snr_db = self.snr_slider.val

        SNR_linear = 10**(self.current_snr_db/10)
        noise_power = self.signal_power / SNR_linear
        noise_std = np.sqrt(noise_power)

        self.base_noise = np.random.normal(0, 1, len(self.modulated_chirp))
        self.noise = self.base_noise * noise_std
        
        self.noisy_signal = self.modulated_chirp + self.noise
        self.update_plots()
        
    def regenerate_noise(self, event):
        SNR_linear = 10**(self.current_snr_db/10)
        noise_power = self.signal_power / SNR_linear
        noise_std = np.sqrt(noise_power)

        self.base_noise = np.random.normal(0, 1, len(self.modulated_chirp))
        self.noise = self.base_noise * noise_std
        self.noisy_signal = self.modulated_chirp + self.noise
        self.update_plots()
        
    def change_symbol(self, event):
        self.symbol = np.random.randint(0, self.N)
        print(f"New symbol to transmit: {self.symbol}")
        print(f"New shift samples: {(self.symbol * (len(self.t) // self.N)) % len(self.t)}")
        samples_per_symbol = len(self.t) // self.N
        shift_samples = (self.symbol * samples_per_symbol) % len(self.t)
        self.modulated_chirp = np.roll(self.base_chirp, shift_samples)

        self.signal_power = np.mean(self.modulated_chirp**2)

        self.update_noise(self.current_snr_db)
        
    def update_plots(self):
        for ax in [self.ax1, self.ax2, self.ax3]:
            ax.clear()

        actual_signal_power = np.mean(self.modulated_chirp**2)
        actual_noise_power = np.mean(self.noise**2) if np.std(self.noise) > 0 else 1e-10
        actual_SNR_dB = 10 * np.log10(actual_signal_power / actual_noise_power)

        correlation = np.correlate(self.noisy_signal, self.base_chirp, mode='full')
        correlation_abs = np.abs(correlation)
        peak_idx = np.argmax(correlation_abs)

        shift_detected = peak_idx - (len(self.base_chirp) - 1)
        samples_per_symbol = len(self.t) // self.N
        detected_symbol = (shift_detected // samples_per_symbol) % self.N

        self.ax1.plot(self.t*1000, self.modulated_chirp, 'b-', label='Pure Signal', linewidth=2)
        self.ax1.plot(self.t*1000, self.noisy_signal, 'r-', alpha=0.7, label='Noisy Signal', linewidth=1)
        self.ax1.set_xlabel('Time (ms)')
        self.ax1.set_ylabel('Amplitude')

        success = '✓' if abs(detected_symbol - self.symbol) <= 2 else '✗'
        title = f'Chirp Signal (SNR: {actual_SNR_dB:.1f} dB, Demod: {success})'
        self.ax1.set_title(title)
        self.ax1.legend()
        self.ax1.grid(True, alpha=0.3)

        info_text = f"""B: {self.B/1000:.1f} kHz
SF: {self.SF} (N={self.N})
T_s: {self.T_s*1000:.2f} ms

Original: {self.symbol}
Detected: {detected_symbol}
Error: {abs(detected_symbol - self.symbol)}

Sig Pwr: {actual_signal_power:.4f}
Noise Pwr: {actual_noise_power:.4f}"""
        
        self.ax1.text(0.02, 0.98, info_text, transform=self.ax1.transAxes, 
                     fontsize=9, verticalalignment='top', fontfamily='monospace',
                     bbox=dict(boxstyle='round', facecolor='white', alpha=0.8))

        correlation_axis = np.arange(len(correlation)) - len(self.base_chirp) + 1
        self.ax2.plot(correlation_axis, correlation_abs, 'g-', linewidth=1)
        self.ax2.axvline(x=peak_idx - len(self.base_chirp) + 1, color='r', linestyle='--', 
                        linewidth=2, label=f'Peak at symbol {detected_symbol}')
        self.ax2.set_xlabel('Lag')
        self.ax2.set_ylabel('|Correlation|')
        self.ax2.set_title('Cross-correlation for Demodulation')
        self.ax2.legend()
        self.ax2.grid(True, alpha=0.3)

        symbols = np.arange(self.N)
        symbol_powers = np.zeros(self.N)

        samples_per_symbol = len(self.t) // self.N
        for i, sym in enumerate(symbols):
            shift_samples = (sym * samples_per_symbol) % len(self.t)
            test_chirp = np.roll(self.base_chirp, shift_samples)

            corr = np.correlate(self.noisy_signal, test_chirp, mode='valid')
            symbol_powers[i] = np.max(np.abs(corr))
        
        bars = self.ax3.bar(symbols, symbol_powers, alpha=0.7, color='lightblue', edgecolor='black')

        if self.symbol < len(bars):
            bars[self.symbol].set_color('blue')
            bars[self.symbol].set_alpha(0.9)
        if detected_symbol < len(bars) and detected_symbol != self.symbol:
            bars[detected_symbol].set_color('red')
            bars[detected_symbol].set_alpha(0.9)
        
        self.ax3.axvline(x=self.symbol, color='blue', linestyle='-', linewidth=3, alpha=0.7,
                        label=f'True: {self.symbol}')
        if detected_symbol != self.symbol:
            self.ax3.axvline(x=detected_symbol, color='red', linestyle='--', linewidth=3, alpha=0.7,
                            label=f'Detected: {detected_symbol}')
        
        self.ax3.set_xlabel('Symbol')
        self.ax3.set_ylabel('Correlation Power')
        self.ax3.set_title('Symbol Detection')
        self.ax3.legend()
        self.ax3.grid(True, alpha=0.3)

        self.ax3.set_xlim(-2, self.N + 2)
        
        plt.tight_layout()
        self.fig.canvas.draw()


if __name__ == "__main__":
    print("Interactive Chirp Modulation/Demodulation Demo")
    print("=" * 60)
    print("Controls:")
    print("- SNR Slider: Adjust signal-to-noise ratio (-30 to 30 dB)")
    print("- Bandwidth Slider: Change bandwidth (10 to 500 kHz)")
    print("- Spreading Factor Slider: Change SF (5 to 12)")
    print("- New Noise: Generate new random noise")
    print("- New Symbol: Generate new random symbol to transmit")
    print("- Regenerate: Regenerate all with current parameters")
    print("=" * 60)
    print("LoRa Parameters:")
    print("- Higher SF = More symbols, longer transmission, better sensitivity")
    print("- Higher BW = Faster data rate, worse sensitivity")
    print("=" * 60)
    
    demo = ChirpModulationDemo()
    plt.show()

