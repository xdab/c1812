import struct
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.tri as tri

if __name__ == '__main__':
    x0: float
    y0: float
    radius: float
    angles_a: np.ndarray
    losses_a: np.ndarray

    with open('results.rf', 'rb') as f:
        f.seek(0, 2)
        file_size = f.tell()
        print(round(file_size / 1024, 1), 'KiB')
        f.seek(0, 0)

        x0 = struct.unpack('d', f.read(8))[0]
        y0 = struct.unpack('d', f.read(8))[0]
        radius = struct.unpack('d', f.read(8))[0]
        ares = struct.unpack('d', f.read(8))[0]
        n = struct.unpack('i', f.read(4))[0]

        losses = []
        angles = []
        angle = 0.0
        while f.tell() < file_size:
            ray = np.zeros(n)
            for i in range(0, n):
                ray[i] = struct.unpack('d', f.read(8))[0]
            losses.append(ray)
            angles.append(angle)
            angle += ares

        angles_a = np.array(angles)
        losses_a = np.array(losses)

    distances_a = np.linspace(0, radius, num=n)

    x = []
    y = []
    z = []
    for i, angle in enumerate(angles_a):
        xs = x0 + distances_a * np.cos(np.radians(angle))
        x.append(xs)
        ys = y0 + distances_a * np.sin(np.radians(angle))
        y.append(ys)
        z.append(losses_a[i])
    x = np.concatenate(x)
    y = np.concatenate(y)
    z = np.concatenate(z)

    ngridx = ngridy = 400
    xi = np.linspace(np.min(x), np.max(x), ngridx)
    yi = np.linspace(np.min(y), np.max(y), ngridy)

    triang = tri.Triangulation(x, y)
    interpolator = tri.LinearTriInterpolator(triang, z)
    Xi, Yi = np.meshgrid(xi, yi)
    zi = interpolator(Xi, Yi)

    if True:
        TX_POWER_W = 45
        TX_POWER_DBM = 10 * np.log10(TX_POWER_W) + 30
        TX_GAIN = 3
        RX_GAIN = 0
        S1_DBM = -121
        S9_DBM = -73
        S_DBM = 6

        zi = TX_POWER_DBM - zi + TX_GAIN + RX_GAIN
        zi = (zi - S1_DBM) / S_DBM

        STEPS_PER_S_UNIT = 1
        levels = np.linspace(1, 9, num=8*STEPS_PER_S_UNIT+1)
        plt.contourf(xi, yi, zi, levels=levels, vmin=1,
                    vmax=9, cmap='gnuplot', extend='both')

        plt.savefig('results.png', dpi=300)
    else:
        plt.pcolormesh(xi, yi, zi, vmin=0, vmax=10, cmap='gnuplot')
    
    plt.axis('equal')
    plt.tight_layout()
    plt.colorbar()
    plt.show()
        