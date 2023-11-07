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

    ngridx = ngridy = 256
    xi = np.linspace(np.min(x), np.max(x), ngridx)
    yi = np.linspace(np.min(y), np.max(y), ngridy)

    triang = tri.Triangulation(x, y)
    interpolator = tri.LinearTriInterpolator(triang, z)
    Xi, Yi = np.meshgrid(xi, yi)
    zi = interpolator(Xi, Yi)

    # plt.pcolormesh(xi, yi, zi, vmin=80, vmax=150, cmap=plt.cm.jet)
    plt.contourf(xi, yi, zi, levels=np.linspace(80, 150, num=22), cmap=plt.cm.jet, extend='both')
    plt.colorbar()
    plt.show()